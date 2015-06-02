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
Search::quiescence ( int alpha, int beta, const char promotionPiece, int N_PIECE, int depth ) {
  if ( !running )
    return 0;
  ASSERT ( chessboard[KING_BLACK + side] );
  int score = -_INFINITE;
  if ( !( numMovesq++ & 1023 ) )	//TODO eliminare?
    running = checkTime (  );

  score = getScore ( side, alpha, beta );
  if ( score >= beta ) {
    return beta;
  }
///************* hash ****************

  _Thash *phashe_always = nullptr;
  _Thash *phashe;
  bool hash_greater = false;
  bool hash_always = false;
  char hashf = hashfALPHA;
  _Thash *phashe_greater = phashe = &( hash_array_greater[side][zobristKey % HASH_SIZE] );
  for ( int i = 0; i < 2; i++ ) {
    if ( phashe->key == zobristKey ) {
      if ( phashe->from != phashe->to && phashe->flags & 0b11 )	// hashfEXACT or hashfBETA
	!i ? hash_greater = true : hash_always = true;
      if ( phashe->depth >= depth ) {
	INC ( probeHash );
	if ( !currentPly ) {
	  if ( phashe->flags == hashfBETA ) {
	    incKillerHeuristic ( phashe->from, phashe->to, 1 );
	  }
	}
	else {
	  if ( phashe->flags == hashfALPHA ) {
	    if ( phashe->score <= alpha ) {
	      INC ( n_cut_hashA );
	      return alpha;
	    }
	  }
	  else {
	    ASSERT ( phashe->flags == hashfEXACT || phashe->flags == hashfBETA );
	    if ( phashe->score >= beta ) {
	      INC ( n_cut_hashB );
	      return beta;
	    }
	  }
	}
	INC ( cutFailed );
      }
    }
    phashe_always = phashe = &( hash_array_always[side][zobristKey % HASH_SIZE] );
  }

///********** end hash ***************
    /**************Delta Pruning ****************/
  char fprune = 0;
  int fscore;
  if ( ( fscore = score + ( promotionPiece == NO_PROMOTION ? VALUEQUEEN : 2 * VALUEQUEEN ) ) < alpha ) {
    fprune = 1;
  }
    /************ end Delta Pruning *************/
  if ( score > alpha )
    alpha = score;
  incListId (  );

  u64 friends = getBitBoard < side > (  );
  u64 enemies = getBitBoard < side ^ 1 > (  );
  if ( generateCaptures < side > ( enemies, friends ) ) {
    decListId (  );
    return _INFINITE - ( mainDepth + depth );
  }

  if ( !getListSize (  ) ) {
    --listId;
    return score;
  }
  _Tmove *move;
  _Tmove *best = nullptr;
  u64 oldKey = zobristKey;
  if ( hash_greater )
    sortHashMoves ( listId, phashe_greater );
  else if ( hash_always )
    sortHashMoves ( listId, phashe_always );
  while ( ( move = getNextMove ( &gen_list[listId] ) ) ) {
    if ( !makemove ( move, false, true ) ) {
      takeback ( move, oldKey, false );
      continue;
    }
    if ( checkInsufficientMaterial ( N_PIECE - 1 ) ) {	//TODO
      takeback ( move, oldKey, false );
      INC ( nCutInsufficientMaterial );
      continue;
    }

	/**************Delta Pruning ****************/
    if ( fprune && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&fscore + PIECES_VALUE[move->capturedPiece] <= alpha /*&& !in_check (side) */  ) {
      INC ( nCutFp );
      takeback ( move, oldKey, false );
      continue;
    }
	/************ end Delta Pruning *************/
    int val = -quiescence < side ^ 1 > ( -beta, -alpha, move->promotionPiece, N_PIECE - 1, depth - 1 );
    score = max ( score, val );
    takeback ( move, oldKey, false );
    if ( score > alpha ) {
      if ( score >= beta ) {
	decListId (  );
	recordHash ( running, phashe_greater, phashe_always, depth, hashfBETA, zobristKey, score, move );
	return beta;
      }
      best = move;
      alpha = score;
      hashf = hashfEXACT;
    }
  }
  recordHash ( running, phashe_greater, phashe_always, depth, hashf, zobristKey, score, best );
  decListId (  );
  return score;
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

bool
Search::checkInsufficientMaterial ( int N_PIECE ) {
  return false;			//TODO commentare
  if ( N_PIECE > 4 )
    return false;
  //insufficient material to mate
  if ( !chessboard[PAWN_BLACK] && !chessboard[PAWN_WHITE] && !chessboard[ROOK_BLACK] && !chessboard[ROOK_WHITE] && !chessboard[QUEEN_WHITE]
       && !chessboard[QUEEN_BLACK] ) {
    u64 allBishop = chessboard[BISHOP_BLACK] | chessboard[BISHOP_WHITE];

    //kings with one or more bishops, and all bishops on the same color
    if ( allBishop ) {
      if ( ( allBishop & BLACK_SQUARES ) == allBishop || ( allBishop & WHITE_SQUARES ) == allBishop ) {
	return true;
      }
    }
    else {
      if ( _bits::bitCount ( chessboard[KNIGHT_WHITE] | chessboard[KNIGHT_BLACK] ) < 2 ) {	//no bishop and 0 or 1 knight
	return true;
      }
    }
    // king + knight versus king or king versus king
    if ( !( chessboard[KNIGHT_WHITE] | chessboard[KNIGHT_BLACK] ) ) {	//no knight
      if ( _bits::bitCount ( allBishop ) < 2 ) {	// no knight and 0 or 1 bishop
	return true;
      }
    }
  }
  return false;
}

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

  return getSide (  )? search < WHITE > ( depth, alpha, beta, pline, _bits::bitCount ( getBitBoard < WHITE > (  ) | getBitBoard < BLACK > (  ) ) )
    : search < BLACK > ( depth, alpha, beta, pline, _bits::bitCount ( getBitBoard < WHITE > (  ) | getBitBoard < BLACK > (  ) ) );
}

template < int side > int
Search::search ( int depth, int alpha, int beta, _TpvLine * pline, int N_PIECE ) {	//TODO eliminare N_PIECE
  if ( !running )
    return 0;

  if ( mainDepth != depth && checkDraw ( zobristKey ) ) {
    return -lazyEval < side > (  ) * 2;
  }
  u64 oldKey = zobristKey;
  INC ( cumulativeMovesCount );

#ifdef DEBUG_MODE
  double betaEfficiencyCount = 0.0;
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
    return quiescence < side > ( alpha, beta, -1, N_PIECE, 0 );
  }

//************* hash ****************

  bool hash_greater = false;
  // bool updatePvFromHash=false;
  _Thash *phashe_greater = &( hash_array_greater[side][zobristKey % HASH_SIZE] );
  if ( phashe_greater->key == zobristKey ) {
    if ( phashe_greater->from != phashe_greater->to && phashe_greater->flags & 0b11 )	// hashfEXACT or hashfBETA
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
	  /* if (phashe_greater->from != phashe_greater->to) {
	     updatePvFromHash=true;
	     updatePv<side>(pline, &line, phashe_greater->from, phashe_greater->to);
	     } */
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
  bool hash_always = false;
  _Thash *phashe_always = &( hash_array_always[side][zobristKey % HASH_SIZE] );
  if ( phashe_always->key == zobristKey ) {
    if ( phashe_always->from != phashe_always->to && phashe_always->flags & 0b11 )	// hashfEXACT or hashfBETA
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
	  /*  if (!updatePvFromHash && phashe_always->from != phashe_always->to) {
	     updatePv<side>(pline, &line, phashe_always->from, phashe_always->to);
	     } */
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

//********** end hash ***************
  if ( !( numMoves & 1023 ) )
    running = checkTime (  );
  int score = -_INFINITE;
  ++numMoves;
//********* null move ***********
  int n_pieces_side;
  if ( !is_incheck_side && !nullSearch && depth >= 3 && ( n_pieces_side = getNpiecesNoPawnNoKing < side > (  ) ) >= 3 ) {
    nullSearch = true;
    int nullScore = -search < side ^ 1 > ( depth - ( 2 + ( depth > ( 3 + ( n_pieces_side < 2 ? 2 : 0 ) ) ) ) - 1, -beta, -beta + 1, &line, N_PIECE );
    nullSearch = false;
    if ( nullScore >= beta ) {
      INC ( nNullMoveCut );
      return nullScore;
    }
  }
///******* null move end ********
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

  incListId (  );

  ASSERT ( KING_BLACK + side >= 0 && KING_BLACK + side < 12 );
  ASSERT ( KING_BLACK + ( side ^ 1 ) >= 0 && KING_BLACK + ( side ^ 1 ) < 12 );
  friendKing[side] = BITScanForward ( chessboard[KING_BLACK + side] );
  friendKing[side ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( side ^ 1 )] );
  u64 friends = getBitBoard < side > (  );
  u64 enemies = getBitBoard < side ^ 1 > (  );
  if ( generateCaptures < side > ( enemies, friends ) ) {
    decListId (  );
    score = _INFINITE - ( mainDepth - depth + 1 );
    return score;
  }
  generateMoves < side > ( friends | enemies );
  int listcount = getListSize (  );
  if ( !listcount ) {
    --listId;
    if ( is_incheck_side )
      return -_INFINITE + ( mainDepth - depth + 1 );
    else
      return -lazyEval < side > (  ) * 2;
  }


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

  _Tmove *best = nullptr;
  bool check_alpha = false;
  if ( hash_greater )
    sortHashMoves ( listId, phashe_greater );
  else if ( hash_always )
    sortHashMoves ( listId, phashe_always );
  INC ( totGen );
  _Tmove *move;
  bool checkInCheck = false;
  int countMove = 0;
  char hashf = hashfALPHA;
  while ( ( move = getNextMove ( &gen_list[listId] ) ) ) {
    countMove++;
    INC ( betaEfficiencyCount );
    if ( !makemove ( move, true, checkInCheck ) ) {
      takeback ( move, oldKey, true );
      continue;
    }
    checkInCheck = true;
    if ( futilPrune && check_alpha && ( ( move->type & 0x3 ) != PROMOTION_MOVE_MASK )	/*&& move->capturedPiece == 12 */
	 &&futilScore + PIECES_VALUE[move->capturedPiece] <= alpha && !inCheck < side > (  ) ) {
      INC ( nCutFp );
      takeback ( move, oldKey, true );
      continue;
    }
    if ( move->capturedPiece != SQUARE_FREE && checkInsufficientMaterial ( N_PIECE - 1 ) ) {	//TODO
      takeback ( move, oldKey, true );
      INC ( nCutInsufficientMaterial );
      continue;
    }

    //Late Move Reduction
    int val = INT_MAX;
    if ( countMove > 4 && !is_incheck_side && depth > 4 && move->capturedPiece == SQUARE_FREE && move->promotionPiece == NO_PROMOTION ) {
      currentPly++;
      val = -search < side ^ 1 > ( depth - 2, -( alpha + 1 ), -alpha, &line, N_PIECE );
      ASSERT ( val != INT_MAX );
      currentPly--;
    }

    if ( val > alpha ) {
      int doMws = ( score > -_INFINITE + 100 );
      int lwb = max ( alpha, score );
      int upb = ( doMws ? ( lwb + 1 ) : beta );
      currentPly++;
      val = -search < side ^ 1 > ( depth - 1, -upb, -lwb, &line, move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1 );
      ASSERT ( val != INT_MAX );
      currentPly--;
      if ( doMws && ( lwb < val ) && ( val < beta ) ) {
	currentPly++;
	val = -search < side ^ 1 > ( depth - 1, -beta, -val + 1, &line, move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1 );
	currentPly--;
      }
    }
    score = max ( score, val );
    takeback ( move, oldKey, true );
    move->score = score;
    if ( score > alpha ) {
      if ( score >= beta ) {
	decListId (  );
	ASSERT ( move->score == score );
	INC ( nCutAB );
#ifdef DEBUG_MODE
	betaEfficiency += betaEfficiencyCount / ( double ) listcount *100.0;
#endif
	recordHash ( running, phashe_greater, phashe_always, depth - extension, hashfBETA, zobristKey, score, move );
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
  recordHash ( running, phashe_greater, phashe_always, depth - extension, hashf, zobristKey, score, best );
  decListId (  );
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

/*
template <int side>
void Search::updatePv(_TpvLine * pline, const _TpvLine * line, const int from, const int to) {
    ASSERT(line->cmove < MAX_PLY - 1);
    _Tmove* mos=&(pline->argmove[0]);
    int type = STANDARD_MOVE_MASK;
    mos->side = (char) side;
    mos->capturedPiece = getPieceAt<side^1>(POW2[to]);
    mos->from = from;
    mos->to = to;

    if ((chessboard[PAWN_WHITE] |chessboard[PAWN_BLACK]) & POW2[from]) {
///promotion
        if (POW2[to] & 0xff000000000000ffULL) {//TODO verificare
            //  cout <<"promotion"<<endl;
            //   display();
            type = PROMOTION_MOVE_MASK;
            mos->promotionPiece = QUEEN_BLACK + side;
        } else {
            mos->promotionPiece = NO_PROMOTION;
///enpassant
            //if ((chessboard[PAWN_WHITE] |chessboard[PAWN_BLACK]) & POW2[from])
            //TODO verificare
            if (mos->capturedPiece == SQUARE_FREE && FILE_AT[from] != FILE_AT[to]) {
                //  cout <<"enpassant"<<endl;
                //  display();
                type = ENPASSANT_MOVE_MASK;
            }
        }
    }
///
    mos->type = rightCastle | type;
    memcpy(pline->argmove + 1, line->argmove, line->cmove * sizeof(_Tmove));
    ASSERT(line->cmove >= 0);
    pline->cmove = line->cmove + 1;
}*/
