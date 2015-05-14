#include "Search.h"
#include "Bits.h"

Search::Search ( char *fen ):
GenMoves ( fen ) {
  eval = new Eval ( NULL, this );
  null_sem = false;
#ifdef DEBUG_MODE
  LazyEvalCuts = 0;
#endif
}

void
Search::startClock (  ) {
  ftime ( &start_time );
}

void
Search::setRandomParam (  ) {
  eval->setRandomParam (  );
}

void
Search::setMainPly ( int m ) {
  mainDepth = m;
}

void
Search::writeParam ( char *param_file, int cd_param, bool append ) {
  eval->writeParam ( param_file, cd_param, append );
}

int
Search::checkTime (  ) {	//return 1;
  if ( running == 2 )
    return 2;
  struct timeb t_current;
  ftime ( &t_current );
  return ( ( int ) ( 1000 * ( t_current.time - start_time.time ) ) ) >= maxTimeMillsec ? 0 : 1;
}

#ifdef TEST_MODE
int
Search::getScore ( const int side
#ifdef FP_MODE
		   , const int alpha, const int beta
#endif
		   , int *a1, int *a2, int *a3, int *a4, int *a5, int *a6, int *a7, int *a8, int *a9, int *a10, int *a11 ) {
  return eval->score ( side
#ifdef FP_MODE
		       , alpha, beta
#endif
		       , a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11 );
}
#endif
int
Search::getScore ( int side
#ifdef FP_MODE
		   , int alpha, int beta
#endif
   ) {
  return eval->score ( side
#ifdef FP_MODE
		       , alpha, beta
#endif
     );
}
Search::~Search (  ) {
  delete eval;
}

int
Search::quiescence ( int alpha, const int side, int beta, const char promotion_piece, int dep ) {
  if ( !running )
    return 0;
#ifdef DEBUG_MODE
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( ( !side && !chessboard[KING_BLACK] ) || ( side && !chessboard[KING_WHITE] ) ) {
    ASSERT ( 0 );
  }
#endif
  int score = -_INFINITE;
  if ( !( num_movesq++ & 1023 ) )
    running = checkTime (  );
  int xside, listcount;
  Tmove *mossa;
  score = eval->score ( side
#ifdef FP_MODE
			, alpha, beta
#endif
     );
  if ( score >= beta ) {
    return beta;
  }
#ifdef FP_MODE
	/**************Delta Pruning ****************/
  char fprune = 0;
  int fscore;
  if ( ( fscore = score + ( promotion_piece == -1 ? VALUEQUEEN : 2 * VALUEQUEEN ) ) < alpha ) {
    fprune = 1;
  }
	/************ end Delta Pruning *************/
#endif
  xside = side ^ 1;
  if ( score > alpha )
    alpha = score;
  list_id++;
  ASSERT ( list_id < MAX_PLY );
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
  for ( int i = 0; i < MAX_MOVE; i++ )
    gen_list[list_id][i].nextBoard = 0;
#endif
#endif
  if ( generateCap ( STANDARD_MOVE_MASK, side ) ) {
    gen_list[list_id--][0].score = 0;
    return _INFINITE - ( mainDepth - dep );
  }
  listcount = gen_list[list_id][0].score;
  if ( !listcount ) {
    --list_id;
    return score;
  }
  int i;
  while ( ( i = getNextMove ( gen_list[list_id] ) ) != -1 ) {
    mossa = &gen_list[list_id][i];
    makemove ( mossa );
#ifdef FP_MODE
		/**************Delta Pruning ****************/
    if ( fprune && ( ( mossa->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& mossa->capture == 12 */
	 &&fscore + PIECES_VALUE[mossa->capture] <= alpha /*&& !in_check (side) */  ) {
      INC ( n_cut_fp );
      takeback ( mossa );
      continue;
    }
		/************ end Delta Pruning *************/
#endif
    int val = -quiescence ( -beta, xside, -alpha, mossa->promotion_piece, dep + 1 );
    score = _max ( score, val );
    takeback ( mossa );
    if ( score >= beta ) {
      gen_list[list_id--][0].score = 0;
      return beta;
    }
    if ( score > alpha ) {
      alpha = score;
    }
  }
  gen_list[list_id--][0].score = 0;
  return alpha;
}

void
Search::setRunning ( int r ) {
  running = r;
}

int
Search::getRunning (  ) {
  return running;
}

void
Search::setMaxTimeMillsec ( int n ) {
  maxTimeMillsec = n;
}

int
Search::getMaxTimeMillsec (  ) {
  return maxTimeMillsec;
}

int
Search::search ( const int side, int depth, int alpha, int beta, LINE * pline
#ifdef DEBUG_MODE
		 , Tmove * Pmossa
#endif
   ) {
  if ( !running )
    return 0;
#ifdef HASH_MODE
  char hashf = hashfALPHA;
#endif
#ifdef DEBUG_MODE
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( ( !side && !chessboard[KING_BLACK] ) || ( side && !chessboard[KING_WHITE] ) )
    ASSERT ( 0 );
#endif
  LINE line;
  line.cmove = 0;
  //************* hash ****************
#ifdef HASH_MODE
  Thash *phashe = &hash_array[key % HASH_SIZE];
  if ( phashe->key == key && phashe->depth >= depth ) {
    ASSERT ( phashe->depth > 0 && phashe->depth <= 32 );
    ASSERT ( phashe->flags > 0 && phashe->flags < 4 );
    ASSERT ( phashe->key != 0 );
    if ( phashe->flags == hashfEXACT ) {
      INC ( n_cut_hash );
      if ( phashe->best.from != 0 && phashe->best.to != 0 )
	updatePv ( pline, &line, &phashe->best, depth );
      return phashe->score;
    }
    else if ( phashe->flags == hashfBETA ) {
      if ( phashe->score >= beta ) {
	INC ( n_cut_hash );
	if ( phashe->best.from != 0 && phashe->best.to != 0 )
	  updatePv ( pline, &line, &phashe->best, depth );
	return beta;
      }
    }
    else if ( phashe->flags == hashfALPHA ) {
      if ( phashe->score <= alpha ) {
	INC ( n_cut_hash );
	if ( phashe->best.from != 0 && phashe->best.to != 0 )
	  updatePv ( pline, &line, &phashe->best, depth );
	return alpha;
      }
    }
  }
#endif
  //********** end hash ***************
  if ( !( num_moves & 1023 ) )
    running = checkTime (  );
  int score = -_INFINITE;
  int n_pieces_x_side = n_pieces ( side ^ 1 );
  int n_pieces_side = n_pieces ( side );
  int is_incheck_side = in_check ( side );
  if ( ( is_incheck_side ) /*|| make_extension() */  )
    depth++;
  if ( depth == 0 ) {
    score = quiescence ( alpha, side, beta, -1, 0 );
    pline->cmove = 0;
    return score;
  };
  ++num_moves;
#ifdef TEST_MODE
  num_moves_test++;
#endif

#ifdef FP_MODE
  int fprune, mat_balance;
  int fscore, Fmax = 0;
#endif
  int val;
  //********* null move ***********
#ifdef NULL_MODE
  if ( !is_incheck_side && null_ok ( depth, side, n_pieces_side ) ) {
    null_sem = true;
    int null_score = -search ( side ^ 1, depth - R_adpt ( side, depth ) - 1, -beta, -beta + 1, &line
#ifdef DEBUG_MODE
			       , Pmossa
#endif
       );
    null_sem = false;
    if ( null_score >= beta ) {
      INC ( null_move_cut );
      return null_score;
    }
  }

#endif
  ///******* null move end ********
#ifdef FP_MODE
	/**************Futility Pruning****************/
	/**************Futility Pruning razor at pre-pre-frontier*****/
  fprune = 0;

  ASSERT ( score == -_INFINITE );
  if ( depth <= 3 && !is_incheck_side ) {
    mat_balance = eval->lazyEval ( side );
    if ( depth == 3 && n_pieces_x_side > 4 && ( mat_balance + RAZOR_MARGIN ) <= alpha ) {
      INC ( n_cut_razor );
      depth--;
    }
    else
      //**************Futility Pruning at pre-frontier*****
    if ( depth == 2 && ( fscore = mat_balance + EXT_FUTILY_MARGIN ) <= alpha ) {
      fprune = 1;
      score = Fmax = fscore;
    }
    else
      //**************Futility Pruning at frontier*****
    if ( depth == 1 && ( fscore = mat_balance + FUTIL_MARGIN ) <= alpha ) {
      fprune = 1;
      score = Fmax = fscore;
    }
  }
	/************ end Futility Pruning*************/
#endif

  int listcount;
  Tmove *mossa;
  list_id++;
  ASSERT ( list_id < MAX_PLY && list_id >= 0 );
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
  for ( int i = 0; i < MAX_MOVE; i++ )
    gen_list[list_id][i].nextBoard = 0;
#endif
#endif
  ASSERT ( list_id < MAX_PLY );
  ASSERT ( KING_BLACK + side >= 0 && KING_BLACK + side < 12 );
  ASSERT ( KING_BLACK + ( side ^ 1 ) >= 0 && KING_BLACK + ( side ^ 1 ) < 12 );
  Friend_king[side] = BITScanForward ( chessboard[KING_BLACK + side] );
  Friend_king[side ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( side ^ 1 )] );
  if ( generateCap ( STANDARD_MOVE_MASK, side ) ) {
    gen_list[list_id--][0].score = 0;
    return _INFINITE - ( mainDepth - depth + 1 );
  }
  generateMoves ( STANDARD_MOVE_MASK, side );
  listcount = gen_list[list_id][0].score;
  if ( !listcount ) {
    --list_id;
    if ( is_incheck_side )
      return -_INFINITE + ( mainDepth - depth + 1 );
    return 0;
  }
  int check_alpha = 0;
#ifdef HASH_MODE
  Tmove best;
  memset ( &best, 0, sizeof ( Tmove ) );
#endif
  int ii;
  while ( ( ii = getNextMove ( gen_list[list_id] ) ) != -1 ) {
    mossa = &gen_list[list_id][ii];
#ifdef DEBUG_MODE
#ifndef PERFT_MODE
    memcpy ( mossa->stack_move, Pmossa->stack_move, sizeof ( Tchessboard ) * Pmossa->nextBoard );
    mossa->nextBoard = Pmossa->nextBoard + 1;
    memcpy ( mossa->stack_move[mossa->nextBoard - 1], chessboard, sizeof ( Tchessboard ) );
#endif
    if ( !( mossa->type & ( KING_SIDE_CASTLE_MOVE_MASK | QUEEN_SIDE_CASTLE_MOVE_MASK ) ) ) {
      assert ( mossa->from >= 0 && mossa->from < 64 );
      if ( ( int ) get_piece_at ( side, TABLOG[mossa->from] ) == 12 ) {
	print (  );
#ifndef PERFT_MODE
	cout << "\n========%d=========" << mossa->nextBoard;
	for ( int i = 0; i < mossa->nextBoard; i++ )
	  print ( &mossa->stack_move[i] );
#endif
	cout << "\n------------------";
	cout << "\nget_piece_at(side, tablog(mossa->from)):" << get_piece_at ( side, TABLOG[mossa->from] );
	cout << "\nget_piece_at(side^1, tablog(mossa->from)):" << get_piece_at ( side ^ 1, TABLOG[mossa->from] );
	cout << "\ndecodeBoardinv(mossa->from, mossa->side):" << decodeBoardinv ( mossa->type, mossa->from, mossa->side );
	cout << "\ndecodeBoardinv(mossa->to, mossa->side):" << decodeBoardinv ( mossa->type, mossa->to, mossa->side );
	cout << "\nmossa->side:" << mossa->side;
	cout << "\nMossa->type:" << mossa->type;
	cout << flush;
	assert ( 0 );
      }
    }
#endif
    makemove ( mossa );
#ifdef FP_MODE
    if ( fprune && check_alpha && ( ( mossa->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& mossa->capture == 12 */
	 &&Fmax + PIECES_VALUE[mossa->capture] <= alpha && !in_check ( side ) ) {
      INC ( n_cut_fp );
      takeback ( mossa );
      continue;
    }
#endif
#ifdef HASH_MODE
    ASSERT ( key == makeZobristKey ( side ) );
#endif
    int do_mws = ( score > -_INFINITE + 100 );
    int lwb = _max ( alpha, score );
    int upb = ( do_mws ? ( lwb + 1 ) : beta );
    val = -search ( side ^ 1, depth - 1, -upb, -lwb, &line
#ifdef DEBUG_MODE
		    , mossa
#endif
       );
    if ( do_mws && ( lwb < val ) && ( val < beta ) ) {
      val = -search ( side ^ 1, depth - 1, -beta, -val + 1, &line
#ifdef DEBUG_MODE
		      , mossa
#endif
	 );
    }
    score = _max ( score, val );
    takeback ( mossa );
    mossa->score = score;
    if ( score >= beta ) {
      gen_list[list_id--][0].score = 0;
#ifdef DEBUG_MODE
      ASSERT ( mossa->score == score );
      n_cut++;
      beta_efficency += 1 / ii;
#endif
#ifdef HASH_MODE
      RecordHash ( depth, hashfBETA, key, beta, &best );
#endif
      if ( mossa->from >= 0 && mossa->to >= 0 && mossa->from < 64 && mossa->to < 64 ) {
	KillerHeuristic[mossa->from][mossa->to] = 0x400;
      }
      return score;
    }
    if ( score > alpha ) {
      alpha = score;
      check_alpha = 1;
#ifdef HASH_MODE
      hashf = hashfEXACT;
      memcpy ( &best, mossa, sizeof ( Tmove ) );
#endif
      updatePv ( pline, &line, mossa );
    }
  }
  gen_list[list_id--][0].score = 0;
  return score;
}

void
Search::updatePv ( LINE * pline, const LINE * line, const Tmove * mossa ) {
#ifdef DEBUG_MODE
  ASSERT ( line->cmove < MAX_PLY - 1 );
#endif
  memcpy ( &( pline->argmove[0] ), mossa, sizeof ( Tmove ) );
  memcpy ( pline->argmove + 1, line->argmove, line->cmove * sizeof ( Tmove ) );
#ifdef DEBUG_MODE
  assert ( line->cmove >= 0 );
#endif
  pline->cmove = line->cmove + 1;
}
