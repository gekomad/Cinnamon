#include "Search.h"

Search::Search (  ) {
#ifdef DEBUG_MODE
  LazyEvalCuts = cumulativeMovesCount = totGen = 0;
#endif
  ponder = nullSearch = false;
}

void
Search::setNullMove ( bool b ) {
  nullSearch = !b;
}

void
Search::startClock (  ) {
  ftime ( &startTime );
}

void
Search::setMainPly ( int m ) {
  mainDepth = m;
}

int
Search::checkTime (  ) {
  if ( running == 2 )
    return 2;
  if ( ponder )
    return 1;
  struct timeb t_current;
  ftime ( &t_current );
  return _time::diffTime ( t_current, startTime ) >= maxTimeMillsec ? 0 : 1;
}

Search::~Search (  ) {
}

template < int side > int
Search::quiescence ( u64 key, int alpha, int beta, const char promotionPiece, int dep ) {
  if ( !running )
    return 0;
  ASSERT ( dep + mainDepth < MAX_PLY );
  ASSERT ( chessboard[KING_BLACK + side] );

  int score = -_INFINITE;
  if ( !( numMovesq++ & 1023 ) )	//TODO eliminare?
    running = checkTime (  );

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
  if ( score > alpha )
    alpha = score;
  incListId (  );

  u64 friends = getBitBoard < side > (  );
  u64 enemies = getBitBoard < side ^ 1 > (  );
  if ( generateCaptures < side > ( enemies, friends, &key ) ) {
    gen_list[listId--].size = 0;
    return _INFINITE - ( mainDepth - dep );
  }
  int listcount = gen_list[listId].size;
  if ( !listcount ) {
    --listId;
    return score;
  }
  _Tmove *move;
  u64 oldKey = key;
  while ( ( move = getNextMove ( &gen_list[listId] ) ) ) {
    makemove ( move, &key, false );
#ifndef NO_FP_MODE
	/**************Delta Pruning ****************/
    if ( fprune && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&fscore + PIECES_VALUE[move->capturedPiece] <= alpha /*&& !in_check (side) */  ) {
      INC ( nCutFp );
      takeback ( move, &key, oldKey, false );
      continue;
    }
	/************ end Delta Pruning *************/
#endif
    int val = -quiescence < side ^ 1 > ( key, -beta, -alpha, move->promotionPiece, dep + 1 );
    score = max ( score, val );
    takeback ( move, &key, oldKey, false );
    if ( score >= beta ) {
      gen_list[listId--].size = 0;
      return beta;
    }
    if ( score > alpha ) {
      alpha = score;
    }
  }
  gen_list[listId--].size = 0;
  return alpha;
}

void
Search::setPonder ( bool r ) {
  ponder = r;
}

void
Search::setRunning ( int r ) {
  running = r;
  if ( !r )
    maxTimeMillsec = 0;
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
Search::sortHashMoves ( int listId1, _Thash * phashe ) {
  for ( int r = 0; r < gen_list[listId1].size; r++ ) {
    _Tmove *mos = &gen_list[listId1].moveList[r];
    if ( phashe && phashe->from == mos->from && phashe->to == mos->to ) {
      mos->score = _INFINITE / 2;
      return;
    }
  }
}

/*TODO
bool Search::checkInsufficientMaterial(int N_PIECE) {

    if(N_PIECE > 4 )return false;
    int s = 0;
    //insufficient material to mate
    if (!chessboard[PAWN_BLACK] && !chessboard[PAWN_WHITE] && !chessboard[ROOK_BLACK] && !chessboard[ROOK_WHITE] && !chessboard[QUEEN_WHITE]
            && !chessboard[QUEEN_BLACK]) {
        u64 allBishop = chessboard[BISHOP_BLACK] | chessboard[BISHOP_WHITE];
        u64 allKnight = chessboard[KNIGHT_WHITE] | chessboard[KNIGHT_BLACK];
        // king + knight versus king or king versus king
        if (!allBishop && _bits::bitCount(allKnight) < 2)//no bishop and 0 or 1 knight
            return true;
        s = _bits::bitCount(allBishop);
        if (!allKnight) {//no knight
            if(s < 2)// 0 or 1 bishop
                return true;
            //kings with one or more bishops, and all bishops on the same color
            if ((allBishop & BLACK_SQUARES)==allBishop||(allBishop & WHITE_SQUARES)==allBishop)
                return true;
        }
    }
    return false;
}
*/
bool
Search::checkDraw ( u64 key ) {
  int o = 0;
  int count = 0;
  for ( int i = repetitionMapCount - 1; i >= 0; i-- ) {
    if ( repetitionMap[i] == 0 )
      return false;
    if ( ++count >= 99 ) {
      return true;
    }
    if ( repetitionMap[i] == key && ++o > 2 ) {
      return true;
    }
  }
  return false;
}

int
Search::search ( int depth, int alpha, int beta, _TpvLine * pline ) {
#ifndef NO_HASH_MODE
  ASSERT ( zobristKey );
#endif
  return getSide (  )? search < WHITE > ( zobristKey, depth, alpha, beta, pline /*,_bits::bitCount(getBitBoard(WHITE)|getBitBoard(BLACK)) */  )
    : search < BLACK > ( zobristKey, depth, alpha, beta, pline /*,_bits::bitCount(getBitBoard(WHITE)|getBitBoard(BLACK)) */  );
}

template < int side > int
Search::search ( u64 key, int depth, int alpha, int beta, _TpvLine * pline /*,int N_PIECE */  ) {
  if ( !running )
    return 0;

  if ( mainDepth != depth && checkDraw ( key ) ) {
    return -lazyEval < side > (  ) * 2;
  }
  /* if( mainDepth!=depth &&checkInsufficientMaterial(N_PIECE)) {
     return 0;
     } */
  u64 oldKey = key;
  INC ( cumulativeMovesCount );

#ifdef DEBUG_MODE
  double betaEfficencyCount = 0.0;
#endif
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  ASSERT ( chessboard[KING_BLACK + side] );

  _TpvLine line;
  line.cmove = 0;
  int is_incheck_side = inCheck < side > (  );
  int extension = 0;
  if ( is_incheck_side /*|| needExtension() */  )
    extension++;
  depth += extension;
  if ( depth == 0 ) {
    return quiescence < side > ( key, alpha, beta, -1, 0 );
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
	    updatePv < side > ( pline, &line, phashe_greater->from, phashe_greater->to );
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
	  }
	  //alpha = max(alpha, (int) phashe_greater->score);//TODO ??
	  break;
	case hashfALPHA:

	  if ( phashe_greater->score <= alpha ) {
	    INC ( n_cut_hashA );
	    return alpha;
	  }
	  //beta = min(beta, (int) phashe_greater->score);//TODO ??
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
	    updatePv < side > ( pline, &line, phashe_always->from, phashe_always->to );
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

	  }
	  //alpha = max(alpha, (int) phashe_always->score);//TODO ??
	  break;
	case hashfALPHA:
	  if ( phashe_always->score <= alpha ) {
	    INC ( n_cut_hashA );
	    return alpha;

	  }
	  //beta = min(beta, (int) phashe_always->score);//TODO ??
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
  ++numMoves;
//********* null move ***********
  int n_pieces_side;
  if ( !is_incheck_side && !nullSearch && depth >= 3 && ( n_pieces_side = getNpiecesNoPawnNoKing < side > (  ) ) >= 3 ) {
    nullSearch = true;
    int nullScore = -search < side ^ 1 > ( key, depth - ( 2 + ( depth > ( 3 + ( n_pieces_side < 2 ? 2 : 0 ) ) ) ) - 1, -beta, -beta + 1, &line /*,N_PIECE */  );
    nullSearch = false;
    if ( nullScore >= beta ) {
      INC ( nNullMoveCut );
      return nullScore;
    }
  }
///******* null move end ********
#ifndef NO_FP_MODE
    /**************Futility Pruning****************/
    /**************Futility Pruning razor at pre-pre-frontier*****/

  ASSERT ( score == -_INFINITE );
  bool futilPrune = false;
  int futilScore = 0;
  if ( depth <= 3 && !is_incheck_side ) {
    int matBalance = lazyEval < side > (  );
    if ( ( futilScore = matBalance + FUTIL_MARGIN ) <= alpha ) {
      if ( depth == 3 && ( matBalance + RAZOR_MARGIN ) <= alpha && getNpiecesNoPawnNoKing < side ^ 1 > (  ) > 3 ) {
	INC ( nCutRazor );
	depth--;
      }
      else
	//**************Futility Pruning at pre-frontier*****
      if ( depth == 2 && ( futilScore = matBalance + EXT_FUTILY_MARGIN ) <= alpha ) {
	futilPrune = true;
	score = futilScore;
      }
      else
	//**************Futility Pruning at frontier*****
      if ( depth == 1 /*&& (futilScore = matBalance + FUTIL_MARGIN) <= alpha */  ) {
	futilPrune = true;
	score = futilScore;
      }
    }
  }
    /************ end Futility Pruning*************/
#endif

  incListId (  );

  ASSERT ( KING_BLACK + side >= 0 && KING_BLACK + side < 12 );
  ASSERT ( KING_BLACK + ( side ^ 1 ) >= 0 && KING_BLACK + ( side ^ 1 ) < 12 );
  friendKing[side] = BITScanForward ( chessboard[KING_BLACK + side] );
  friendKing[side ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( side ^ 1 )] );
  u64 friends = getBitBoard < side > (  );
  u64 enemies = getBitBoard < side ^ 1 > (  );
  if ( generateCaptures < side > ( enemies, friends, &key ) ) {
    gen_list[listId--].size = 0;
    score = _INFINITE - ( mainDepth - depth + 1 );
    return score;
  }
  generateMoves < side > ( friends | enemies );
  int listcount = gen_list[listId].size;
  if ( !listcount ) {
    --listId;
    if ( is_incheck_side )
      return -_INFINITE + ( mainDepth - depth + 1 );
    else
      return -lazyEval < side > (  ) * 2;
  }

#ifndef NO_HASH_MODE
//Enhanced Transposition Cutoff
  /*  for (int i = 0; i < listcount; i++) {
     move = &gen_list[listId][i];
     if (move->score == _INFINITE)
     break;
     makemove(move, &key);
     _Thash * phashe = getHash(side,key);
     if (phashe->key == key) {
     move->score = _INFINITE - 1;
     }
     takeback(move, &key, oldKey);
     } */

  /*for (int i = 0; i < listcount; i++) {
     int from = gen_list[listId][i].from;
     int to = gen_list[listId][i].to;
     if (killerHeuristic[currentPly][from][to]) {
     gen_list[listId][i].score = killerHeuristic[currentPly][from][to];
     continue;
     }
     } */

#endif
  _Tmove *best = NULL;
  bool check_alpha = false;
  if ( hash_greater )
    sortHashMoves ( listId, phashe_greater );
  else if ( hash_always )
    sortHashMoves ( listId, phashe_always );
  INC ( totGen );
  _Tmove *move;
  while ( ( move = getNextMove ( &gen_list[listId] ) ) ) {
    INC ( betaEfficencyCount );
    makemove ( move, &key, true );
#ifndef NO_FP_MODE
    if ( futilPrune && check_alpha && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&futilScore + PIECES_VALUE[move->capturedPiece] <= alpha && !inCheck < side > (  ) ) {
      INC ( nCutFp );
      takeback ( move, &key, oldKey, true );
      continue;
    }
#endif

    int doMws = ( score > -_INFINITE + 100 );
    int lwb = max ( alpha, score );
    int upb = ( doMws ? ( lwb + 1 ) : beta );
    currentPly++;
    int val = -search < side ^ 1 > ( key, depth - 1, -upb, -lwb, &line /*,move->capturedPiece==SQUARE_FREE?N_PIECE:N_PIECE-1 */  );
    currentPly--;
    if ( doMws && ( lwb < val ) && ( val < beta ) ) {
      currentPly++;
      val = -search < side ^ 1 > ( key, depth - 1, -beta, -val + 1, &line /*,move->capturedPiece==SQUARE_FREE?N_PIECE:N_PIECE-1 */  );
      currentPly--;
    }
    score = max ( score, val );
    takeback ( move, &key, oldKey, true );
    move->score = score;
    if ( score > alpha ) {
      if ( score >= beta ) {
	gen_list[listId--].size = 0;
	ASSERT ( move->score == score );
	INC ( nCutAB );
#ifdef DEBUG_MODE
	betaEfficency += betaEfficencyCount / ( double ) listcount *100.0;
#endif
	recordHash ( running, phashe_greater, phashe_always, depth - extension, hashfBETA, key, score, move );
	setKillerHeuristic ( move->from, move->to, 0x400 );
	return score;
      }
      alpha = score;
      check_alpha = true;
      hashf = hashfEXACT;
      best = move;
      move->score = score;	//used in it
      updatePv ( pline, &line, move );
    }
  }
  recordHash ( running, phashe_greater, phashe_always, depth - extension, hashf, key, score, best );
  gen_list[listId--].size = 0;
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
Search::pushMovesPath ( char move ) {
  movesPath += move;
}

void
Search::clearMovesPath (  ) {
  movesPath.clear (  );
}

string
Search::getMovesPath (  ) {
  return movesPath;
}

template < int side > void
Search::updatePv ( _TpvLine * pline, const _TpvLine * line, const int from, const int to ) {
  ASSERT ( line->cmove < MAX_PLY - 1 );
  _Tmove mos;
  int type = STANDARD_MOVE_MASK;
  mos.side = ( char ) side;
  mos.capturedPiece = getPieceAt < side ^ 1 > ( POW2[to] );
  int pieceFrom = getPieceAt < side > ( POW2[from] );
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
