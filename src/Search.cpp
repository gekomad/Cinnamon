#include "Search.h"
#include "utils.h"

Search::Search (  ) {
#ifdef DEBUG_MODE
  LazyEvalCuts = totmosse = totGen = 0;
#endif
//    resetRepetitionCount();
  nullSem = false;
//    pushStackMove(makeZobristKey(),false);

}



void
Search::setNullMove ( bool b ) {
  nullSem = !b;
}

void
Search::startClock (  ) {
  ftime ( &start_time );
}

void
Search::setMainPly ( int m ) {
  mainDepth = m;
}

int
Search::checkTime (  ) {
  if ( running == 2 )
    return 2;
  struct timeb t_current;
  ftime ( &t_current );
  return ( ( int ) ( 1000 * ( t_current.time - start_time.time ) ) ) >= maxTimeMillsec ? 0 : 1;
}

Search::~Search (  ) {

}

int
Search::quiescence ( u64 key, int alpha, const int side, int beta, const char promotionPiece, int dep ) {
#ifdef TUNE_CRAFTY_MODE
  return 0;
#else
  if ( !running )
    return 0;

#ifdef DEBUG_MODE
  ASSERT ( dep + mainDepth < MAX_PLY );
  if ( ( !side && !chessboard[KING_BLACK] ) || ( side && !chessboard[KING_WHITE] ) ) {
    ASSERT ( 0 );
  }
#endif
  int score = -_INFINITE;
  if ( !( numMovesq++ & 1023 ) )
    running = checkTime (  );
  int xside, listcount;
  _Tmove *move;
  score = getScore ( side, alpha, beta );
  if ( score >= beta ) {
    return beta;
  }
#ifndef NO_FP_MODE
    /**************Delta Pruning ****************/
  char fprune = 0;
  int fscore;
  if ( ( fscore = score + ( promotionPiece == -1 ? VALUEQUEEN : 2 * VALUEQUEEN ) ) < alpha ) {
    fprune = 1;
  }
    /************ end Delta Pruning *************/
#endif
  xside = side ^ 1;
  if ( score > alpha )
    alpha = score;
  list_id++;
  ASSERT ( list_id < MAX_PLY );
  u64 friends = getBitBoard ( side );
  u64 enemies = getBitBoard ( side ^ 1 );
  if ( generateCap ( side, enemies, friends, &key ) ) {
    gen_list[list_id--][0].score = 0;
    return _INFINITE - ( mainDepth - dep );
  }
  listcount = gen_list[list_id][0].score;
  if ( !listcount ) {
    --list_id;
    return score;
  }
  int i;
  u64 oldKey = key;
  while ( ( i = getNextMove ( gen_list[list_id] ) ) != -1 ) {
    move = &gen_list[list_id][i];
    makemove ( move, &key );
#ifndef NO_FP_MODE
	/**************Delta Pruning ****************/
    if ( fprune && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&fscore + PIECES_VALUE[move->capturedPiece] <= alpha /*&& !in_check (side) */  ) {
      INC ( n_cut_fp );
      takeback ( move, &key, oldKey );
      continue;
    }
	/************ end Delta Pruning *************/
#endif
    int val = -quiescence ( key, -beta, xside, -alpha, move->promotionPiece, dep + 1 );
    score = max ( score, val );
    takeback ( move, &key, oldKey );
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
#endif
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

void
Search::sortHashMoves ( int list_id1, _Thash * phashe ) {
  for ( int r = 1; r <= gen_list[list_id1][0].score; r++ ) {
    _Tmove *mos = &gen_list[list_id1][r];
    if ( phashe && phashe->from == mos->from && phashe->to == mos->to ) {
      mos->score = _INFINITE / 2;
      return;
    }
  }
}				/*
				   bool Search::checkInsufficientMaterial() {
				   int s = 0;
				   //insufficient material to mate
				   if (!chessboard[PAWN_BLACK] && !chessboard[PAWN_WHITE] && !chessboard[ROOK_BLACK] && !chessboard[ROOK_WHITE] && !chessboard[QUEEN_WHITE]
				   && !chessboard[QUEEN_BLACK]) {
				   u64 allBishop = (chessboard[BISHOP_BLACK] | chessboard[BISHOP_WHITE]);
				   u64 allKnight = (chessboard[KNIGHT_WHITE] | chessboard[KNIGHT_BLACK]);
				   // king + knight versus king or king versus king
				   if (!allBishop && bitCount(allKnight) < 2)//no bishop and 0 or 1 knight
				   return true;
				   s = bitCount(allBishop);
				   if (!allKnight) {//no knight
				   if( s < 2)// 0 or 1 bishop
				   return true;
				   //kings with one or more bishops, and all bishops on the same color
				   if ((allBishop & BLACK_SQUARES)==allBishop||(allBishop & WHITE_SQUARES)==allBishop)
				   return true;
				   }
				   }
				   return false;
				   }
				 */
/*
bool Search::checkDraw(u64 key) {
    int s = 0;
    ASSERT(key);

//if (repetitionMapCount>4 ){cout << repetitionMap[repetitionMapCount-1]<< " " <<repetitionMap[repetitionMapCount-3] <<" "<< repetitionMap[repetitionMapCount-5]<<" "<<key<<endl;}

    for(int i=repetitionMapCount-3; i>=0; i-=2) {
        //for(int i=repetitionMapCount-2; i>=0; i-=1) {
        ASSERT(i>=0);
        // cout <<repetitionMap[i].key<<" "<<key<<endl;
        if(repetitionMap[i].key==key && (++s)==2) {
            //cout <<"xxrepetition\n";
            return true;
        }

        if(repetitionMap[i].reset)return false;
        if(i&&repetitionMap[i-1].reset)return false;
    }

    return false;
}*/
int
Search::search ( u64 key, const int side, int depth, int alpha, int beta, _TpvLine * pline ) {
  if ( !running )
    return 0;
//    if(checkInsufficientMaterial())return 0;
  u64 oldKey = key;
  INC ( totmosse );

#ifdef DEBUG_MODE
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( ( !side && !chessboard[KING_BLACK] ) || ( side && !chessboard[KING_WHITE] ) )
    ASSERT ( 0 );
#endif
  _TpvLine line;
  line.cmove = 0;
  int is_incheck_side = in_check ( side );
  int extension = 0;
  if ( is_incheck_side /*|| needExtension() */  )
    extension++;
  depth += extension;
  if ( depth == 0 ) {
    // return score(side,alpha,beta);
    return quiescence ( key, alpha, side, beta, -1, 0 );
  }
  _Thash *phashe_greater = NULL;
  _Thash *phashe_always = NULL;
  bool hash_greater = false;
  bool hash_always = false;
//************* hash ****************
  char hashf = hashfALPHA;
#ifndef NO_HASH_MODE
  ASSERT ( key );
  bool updatePvFromHash = false;
  phashe_greater = &( hash_array_greater[side][key % HASH_SIZE] );
  if ( phashe_greater->key == key ) {
    if ( phashe_greater->from != phashe_greater->to && ( phashe_greater->flags == hashfEXACT || phashe_greater->flags == hashfBETA ) )
      hash_greater = true;
    if ( phashe_greater->depth >= depth ) {
      INC ( probeHash );
      if ( !currentPly ) {
	if ( phashe_greater->flags == hashfBETA )
	  incKillerHeuristic ( phashe_greater->from, phashe_greater->to, 1 );
      }
      else {
	switch ( phashe_greater->flags ) {
	case hashfEXACT:
	  INC ( n_cut_hashE );

	  if ( phashe_greater->from != phashe_greater->to ) {
	    updatePvFromHash = true;
	    updatePv ( pline, &line, phashe_greater->from, phashe_greater->to, side );
	  }
	  if ( phashe_greater->score >= beta ) {
	    return beta;
	  }

	  break;

	case hashfBETA:
	  incKillerHeuristic ( phashe_greater->from, phashe_greater->to, 1 );
	  if ( phashe_greater->score >= beta ) {
	    INC ( n_cut_hashB );
	    return beta;

	    alpha = max ( alpha, ( int ) phashe_greater->score );
	  }
	  break;
	case hashfALPHA:

	  if ( phashe_greater->score <= alpha ) {
	    INC ( n_cut_hashA );
	    return alpha;

	    beta = min ( beta, ( int ) phashe_greater->score );
	  }
	  break;
	default:
	  break;
	}
	INC ( cutFailed );
      }
    }
  }
  phashe_always = &( hash_array_always[side][key % HASH_SIZE] );
  if ( phashe_always->key == key ) {
    if ( phashe_always->from != phashe_always->to && ( phashe_always->flags == hashfEXACT || phashe_always->flags == hashfBETA ) )
      hash_always = true;
    if ( phashe_always->depth >= depth ) {
      INC ( probeHash );
      if ( !currentPly ) {
	if ( phashe_always->flags == hashfBETA )
	  incKillerHeuristic ( phashe_always->from, phashe_always->to, 1 );
      }
      else {
	switch ( phashe_always->flags ) {
	case hashfEXACT:
	  INC ( n_cut_hashE );

	  if ( !updatePvFromHash && phashe_always->from != phashe_always->to ) {
	    updatePv ( pline, &line, phashe_always->from, phashe_always->to, side );
	  }
	  if ( phashe_always->score >= beta ) {
	    return beta;
	  }

	  break;

	case hashfBETA:
	  incKillerHeuristic ( phashe_always->from, phashe_always->to, 1 );
	  if ( phashe_always->score >= beta ) {
	    INC ( n_cut_hashB );
	    return beta;
	    alpha = max ( alpha, ( int ) phashe_always->score );
	  }
	  break;
	case hashfALPHA:
	  if ( phashe_always->score <= alpha ) {
	    INC ( n_cut_hashA );
	    return alpha;

	    beta = min ( beta, ( int ) phashe_always->score );
	  }
	  break;
	default:
	  break;
	}
	INC ( cutFailed );
      }
    }
  }
#endif
//********** end hash ***************
  if ( !( numMoves & 1023 ) )
    running = checkTime (  );
  int score = -_INFINITE;
  int n_pieces_side = getNpieces ( side );
  int n_pieces_x_side = getNpieces ( side ^ 1 );
  ++numMoves;
  int fprune, mat_balance;
  int fscore, Fmax = 0;

  int val;
//********* null move ***********
  if ( !is_incheck_side && null_ok ( depth, side, n_pieces_side ) ) {
    nullSem = true;
    int nullScore = -search ( key, side ^ 1, depth - R_adpt ( side, depth ) - 1, -beta, -beta + 1, &line );
    nullSem = false;
    if ( nullScore >= beta ) {
      INC ( null_move_cut );
      return nullScore;
    }
  }
///******* null move end ********
#ifndef NO_FP_MODE
    /**************Futility Pruning****************/
    /**************Futility Pruning razor at pre-pre-frontier*****/
  fprune = 0;

  ASSERT ( score == -_INFINITE );
  if ( depth <= 3 && !is_incheck_side ) {
    mat_balance = lazyEval ( side );
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
  _Tmove *move;
  list_id++;
  ASSERT ( list_id < MAX_PLY && list_id >= 0 );
  ASSERT ( KING_BLACK + side >= 0 && KING_BLACK + side < 12 );
  ASSERT ( KING_BLACK + ( side ^ 1 ) >= 0 && KING_BLACK + ( side ^ 1 ) < 12 );
  friendKing[side] = BITScanForward ( chessboard[KING_BLACK + side] );
  friendKing[side ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( side ^ 1 )] );
  u64 friends = getBitBoard ( side );
  u64 enemies = getBitBoard ( side ^ 1 );
  if ( generateCap ( side, enemies, friends, &key ) ) {
    gen_list[list_id--][0].score = 0;
    score = _INFINITE - ( mainDepth - depth + 1 );
    return score;
  }
  generateMoves ( side, friends | enemies );
  listcount = gen_list[list_id][0].score;
  if ( !listcount ) {
    --list_id;
    if ( is_incheck_side )
      return -_INFINITE + ( mainDepth - depth + 1 );
    else
      return 0;
  }

#ifndef NO_HASH_MODE
//Enhanced Transposition Cutoff
  /*  for (int i = 1; i <= listcount; i++) {
     move = &gen_list[list_id][i];
     if (move->score == _INFINITE)
     break;
     makemove(move, &key);
     _Thash * phashe = getHash(side,key);
     if (phashe->key == key) {
     move->score = _INFINITE - 1;
     }
     takeback(move, &key, oldKey);
     } */

  /*for (int i = 1; i <= listcount; i++) {
     int from = gen_list[list_id][i].from;
     int to = gen_list[list_id][i].to;
     if (killerHeuristic[currentPly][from][to]) {
     gen_list[list_id][i].score = killerHeuristic[currentPly][from][to];
     continue;
     }
     } */

#endif
  _Tmove *best = NULL;
  int ii;
  bool check_alpha = false;
  if ( hash_greater )
    sortHashMoves ( list_id, phashe_greater );
  else if ( hash_always )
    sortHashMoves ( list_id, phashe_always );
#ifdef DEBUG_MODE
  double N = 0.0;
#endif
  INC ( totGen );
  while ( ( ii = getNextMove ( gen_list[list_id] ) ) != -1 ) {
    INC ( N );
    move = &gen_list[list_id][ii];
    makemove ( move, &key );

    /* if(move->pieceFrom==WHITE||move->pieceFrom==BLACK||move->capturedPiece!=SQUARE_FREE)
       pushStackMove(key,true);
       else {
       pushStackMove(key,false);
       if (checkDraw( key)) {
       popStackMove();
       takeback(move, &key, oldKey);
       continue;
       }
       } */
#ifndef NO_FP_MODE
    if ( fprune && check_alpha && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&Fmax + PIECES_VALUE[move->capturedPiece] <= alpha && !in_check ( side ) ) {
      INC ( n_cut_fp );
//            popStackMove();
      takeback ( move, &key, oldKey );
      continue;
    }
#endif

    int doMws = ( score > -_INFINITE + 100 );
    int lwb = max ( alpha, score );
    int upb = ( doMws ? ( lwb + 1 ) : beta );
    currentPly++;
    val = -search ( key, side ^ 1, depth - 1, -upb, -lwb, &line );
    currentPly--;
    if ( doMws && ( lwb < val ) && ( val < beta ) ) {
      currentPly++;
      val = -search ( key, side ^ 1, depth - 1, -beta, -val + 1, &line );
      currentPly--;
    }
    score = max ( score, val );
    //popStackMove();
    takeback ( move, &key, oldKey );
    if ( !running ) {
      gen_list[list_id--][0].score = 0;
      return 0;
    }
    move->score = score;
    if ( score > alpha ) {
      if ( score >= beta ) {
	gen_list[list_id--][0].score = 0;
#ifdef DEBUG_MODE
	ASSERT ( move->score == score );
	INC ( n_cut );
	beta_efficency += N / ( double ) listcount *100.0;
#endif
	recordHash ( running, phashe_greater, phashe_always, depth - extension, hashfBETA, key, score, move );
	setKillerHeuristic ( move->from, move->to, 0x400 );
	return score;
      }
      alpha = score;
      check_alpha = true;
      hashf = hashfEXACT;
      best = move;
      move->score = score;	//si usa in it
      updatePv ( pline, &line, move );
    }
  }
  recordHash ( running, phashe_greater, phashe_always, depth - extension, hashf, key, score, best );
  gen_list[list_id--][0].score = 0;
  return score;
}

void
Search::updatePv ( _TpvLine * pline, const _TpvLine * line, const _Tmove * move ) {
  ASSERT ( line->cmove < MAX_PLY - 1 );
  memcpy ( &( pline->argmove[0] ), move, sizeof ( _Tmove ) );
  memcpy ( pline->argmove + 1, line->argmove, line->cmove * sizeof ( _Tmove ) );
  ASSERT ( line->cmove >= 0 );
  pline->cmove = line->cmove + 1;
}

void
Search::updatePv ( _TpvLine * pline, const _TpvLine * line, const int from, const int to, const int side ) {
  ASSERT ( line->cmove < MAX_PLY - 1 );
  _Tmove mos;
  int type = STANDARD_MOVE_MASK;
  mos.side = ( char ) side;
  mos.capturedPiece = getPieceAt ( ( side ^ 1 ), TABLOG[to] );
  int pieceFrom = getPieceAt ( ( side ), TABLOG[from] );
  mos.from = from;
  mos.to = to;
  mos.promotionPiece = -1;
  if ( ( pieceFrom == PAWN_WHITE && from > 55 ) || ( pieceFrom == PAWN_BLACK && from < 8 ) ) {
    type = PROMOTION_MOVE_MASK;
    mos.promotionPiece = QUEEN_BLACK + side;
  }
//TODO enpassant
  mos.type = RIGHT_CASTLE | type;
  memcpy ( &( pline->argmove[0] ), &mos, sizeof ( _Tmove ) );
  memcpy ( pline->argmove + 1, line->argmove, line->cmove * sizeof ( _Tmove ) );
  ASSERT ( line->cmove >= 0 );
  pline->cmove = line->cmove + 1;
}
