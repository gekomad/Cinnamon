#include "GenMoves.h"

GenMoves::GenMoves (  ) {
  perftMode = false;
  currentPly = 0;
//    evaluateMobilityMode = false;
  gen_list = ( _TmoveP * ) calloc ( MAX_PLY, sizeof ( _TmoveP ) );
  assert ( gen_list );
  for ( int i = 0; i < MAX_PLY; i++ ) {
    gen_list[i].moveList = ( _Tmove * ) calloc ( MAX_MOVE, sizeof ( _Tmove ) );
    assert ( gen_list[i].moveList );
  }
  repetitionMap = ( u64 * ) malloc ( sizeof ( u64 ) * MAX_REP_COUNT );
  assert ( repetitionMap );
  listId = -1;
  repetitionMapCount = 0;
}

void
GenMoves::generateMoves ( const int side, u64 allpieces ) {
  side ? generateMoves < WHITE > ( allpieces ) : generateMoves < BLACK > ( allpieces );
}

template < int side > void
GenMoves::generateMoves ( u64 allpieces ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  tryAllCastle ( side, allpieces );
  performBishopShift ( BISHOP_BLACK + side, side, allpieces );
  performRookQueenShift ( ROOK_BLACK + side, side, allpieces );
  performRookQueenShift ( QUEEN_BLACK + side, side, allpieces );
  performBishopShift ( QUEEN_BLACK + side, side, allpieces );
  performPawnShift < side > ( ~allpieces );
  performKnightShiftCapture ( KNIGHT_BLACK + side, ~allpieces, side );
  performKingShiftCapture ( side, ~allpieces );
}

bool
GenMoves::generateCaptures ( const int side, u64 enemies, u64 friends, u64 * key ) {
  return side ? generateCaptures < WHITE > ( enemies, friends, key ) : generateCaptures < BLACK > ( enemies, friends, key );
}

template < int side > bool
GenMoves::generateCaptures ( u64 enemies, u64 friends, u64 * key ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  u64 allpieces = enemies | friends;

  if ( performPawnCapture < side > ( enemies, key ) )
    return true;
  if ( performKingShiftCapture ( side, enemies ) )
    return true;

  if ( performKnightShiftCapture ( KNIGHT_BLACK + side, enemies, side ) )
    return true;

  if ( performBishopCapture ( BISHOP_BLACK + side, enemies, side, allpieces ) )
    return true;

  if ( performRookQueenCapture ( ROOK_BLACK + side, enemies, side, allpieces ) )
    return true;

  if ( performRookQueenCapture ( QUEEN_BLACK + side, enemies, side, allpieces ) )
    return true;
  if ( performBishopCapture ( QUEEN_BLACK + side, enemies, side, allpieces ) )
    return true;

  return false;
}

void
GenMoves::setKillerHeuristic ( int from, int to, int value ) {
  ASSERT ( from >= 0 && from < 64 && to >= 0 && to < 64 );
  killerHeuristic[from][to] = value;
}

void
GenMoves::incKillerHeuristic ( int from, int to, int value ) {
  ASSERT ( from >= 0 && from < 64 && to >= 0 && to < 64 );
  ASSERT ( killerHeuristic[from][to] <= killerHeuristic[from][to] + value );
  killerHeuristic[from][to] += value;
}

void
GenMoves::setPerft ( bool b ) {
  perftMode = b;
}

void
GenMoves::clearKillerHeuristic (  ) {
  memset ( killerHeuristic, 0, sizeof ( killerHeuristic ) );
}

_Tmove *
GenMoves::getNextMove ( _TmoveP * list ) {
  _Tmove *gen_list1 = list->moveList;
  int listcount = list->size;
  int bestId = -1;
  int j, bestScore;
  for ( j = 0; j < listcount; j++ ) {
    if ( !gen_list1[j].used ) {
      bestId = j;
      bestScore = gen_list1[bestId].score;
      break;
    }
  }
  if ( bestId == -1 )
    return NULL;
  for ( int i = j + 1; i < listcount; i++ ) {
    if ( !gen_list1[i].used && gen_list1[i].score > bestScore )
      bestId = i;
    bestScore = gen_list1[bestId].score;
  }
  gen_list1[bestId].used = true;
  return &gen_list1[bestId];
}

/*
int GenMoves::getNextMove(_TmoveP* gen_list1) {TODO bench su 64 bit
    u128 bits;
    memcpy(&bits,&(gen_list1->used),sizeof(u128));
    //cout <<bits.h<<" "<<bits.l<<endl<<flush;
    if(isZero(&bits))return -1;
    int bestScoreId=BITScanForward(&bits);
    int bestScore=gen_list1->moveList[bestScoreId+1].score;
    unsetBit(&bits,bestScoreId);

    int i;
    while (!isZero(&bits)) {
        i=BITScanForward(&bits);
        if (gen_list1->moveList[i+1].score > bestScore) {
            bestScoreId = i;
            bestScore=gen_list1->moveList[bestScoreId+1].score;
        }
        unsetBit(&bits,i);
    }

    unsetBit(&gen_list1[0].used,bestScoreId);
    return bestScoreId+1;
}*/

GenMoves::~GenMoves (  ) {
  for ( int i = 0; i < MAX_PLY; i++ )
    free ( gen_list[i].moveList );
  free ( gen_list );
  free ( repetitionMap );
}

bool
GenMoves::isPinned ( const int side, const uchar position, const uchar piece ) {
  u64 king = chessboard[KING_BLACK + side];
  int posKing = BITScanForward ( king );
  u64 pow2position = POW2[position];
  if ( !( LEFT_RIGHT_RANK_FILE[posKing] & pow2position ) )
    return false;
  int xside = side ^ 1;
  chessboard[piece] &= NOTPOW2[position];
  u64 allpieces = getBitBoard < WHITE > (  ) | getBitBoard < BLACK > (  );
  u64 qr = chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside];
  u64 qb = chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside];

  if ( king & RANK[position] && RANK[position] & qr ) {
    //rank
    for ( int n = position + 1; n <= ORIZ_LEFT[position]; n++ ) {
      if ( qr & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;
    }
    for ( int n = position - 1; n >= ORIZ_RIGHT[position]; n-- ) {
      if ( qr & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;

    }
  }
  else if ( king & FILE_[position] && FILE_[position] & qr ) {
    for ( int n = posKing + 8; n <= VERT_UPPER[posKing]; n += 8 ) {
      if ( qr & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( POW2[n] & allpieces )
	break;
    }
    for ( int n = posKing - 8; n >= VERT_LOWER[posKing]; n -= 8 ) {
      if ( qr & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( POW2[n] & allpieces )
	break;
    }
  }
  else if ( king & LEFT_DIAG[position] && LEFT_DIAG[position] & qb ) {
    for ( int n = position + 7; n <= LEFT_UPPER[position]; n += 7 ) {
      if ( qb & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;
    }
    for ( int n = position - 7; n >= LEFT_LOWER[position]; n -= 7 ) {
      if ( qb & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;
    }
  }
  else if ( king & RIGHT_DIAG[position] && RIGHT_DIAG[position] & qb ) {
    for ( int n = position + 9; n <= RIGHT_UPPER[position]; n += 9 ) {
      if ( qb & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;
    }
    for ( int n = position - 9; n >= RIGHT_LOWER[position]; n -= 9 ) {
      if ( qb & POW2[n] ) {
	chessboard[piece] |= pow2position;
	return true;
      }
      if ( allpieces & POW2[n] )
	break;
    }
  }
  chessboard[piece] |= pow2position;
  return false;
}


void
GenMoves::performCastle ( const int side, const uchar type, u64 * key ) {
  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( getPieceAt ( side, POW2_3 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, POW2_1 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_2 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_0 ) == ROOK_WHITE );

      updateZobristKey ( key, KING_WHITE, 3 );
      updateZobristKey ( key, KING_WHITE, 1 );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | POW2_1 ) & NOTPOW2_3;

      updateZobristKey ( key, ROOK_WHITE, 2 );
      updateZobristKey ( key, ROOK_WHITE, 0 );
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | POW2_2 ) & NOTPOW2_0;

    }
    else {
      ASSERT ( type & QUEEN_SIDE_CASTLE_MOVE_MASK );
      ASSERT ( getPieceAt ( side, POW2_3 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, POW2_4 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_5 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_6 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_7 ) == ROOK_WHITE );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | POW2_5 ) & NOTPOW2_3;

      updateZobristKey ( key, KING_WHITE, 5 );
      updateZobristKey ( key, KING_WHITE, 3 );
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | POW2_4 ) & NOTPOW2_7;

      updateZobristKey ( key, ROOK_WHITE, 4 );
      updateZobristKey ( key, ROOK_WHITE, 7 );
    }
  }
  else {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {

      ASSERT ( getPieceAt ( side, POW2_59 ) == KING_BLACK );
      ASSERT ( getPieceAt ( side, POW2_58 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_57 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_56 ) == ROOK_BLACK );

      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | POW2_57 ) & NOTPOW2_59;

      updateZobristKey ( key, KING_BLACK, 57 );
      updateZobristKey ( key, KING_BLACK, 59 );
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | POW2_58 ) & NOTPOW2_56;

      updateZobristKey ( key, ROOK_BLACK, 58 );
      updateZobristKey ( key, ROOK_BLACK, 56 );
    }
    else {
      ASSERT ( type & QUEEN_SIDE_CASTLE_MOVE_MASK );
      ASSERT ( getPieceAt ( side, POW2_59 ) == KING_BLACK );
      ASSERT ( getPieceAt ( side, POW2_60 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_61 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_62 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, POW2_63 ) == ROOK_BLACK );

      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | POW2_61 ) & NOTPOW2_59;

      updateZobristKey ( key, KING_BLACK, 61 );
      updateZobristKey ( key, KING_BLACK, 59 );
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | POW2_60 ) & NOTPOW2_63;

      updateZobristKey ( key, ROOK_BLACK, 60 );
      updateZobristKey ( key, ROOK_BLACK, 63 );
    }
  }
}

void
GenMoves::performRookQueenShift ( const int piece, const int side, const u64 allpieces ) {
  ASSERT ( piece >= 0 && piece < 12 );
  int n, position;
  u64 x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    for ( n = position + 1; n <= ORIZ_LEFT[position]; n++ ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    for ( n = position - 1; n >= ORIZ_RIGHT[position]; n-- ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    for ( n = position + 8; n <= VERT_UPPER[position]; n += 8 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    for ( n = position - 8; n >= VERT_LOWER[position]; n -= 8 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x2 &= NOTPOW2[position];
  };
}

void
GenMoves::performBishopShift ( const int piece, const int side, const u64 allpieces ) {
  ASSERT ( piece >= 0 && piece < 12 );
  int position;
  int n;
  u64 x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    for ( n = position + 7; n <= LEFT_UPPER[position]; n += 7 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }

    for ( n = position - 7; n >= LEFT_LOWER[position]; n -= 7 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }

    for ( n = position + 9; n <= RIGHT_UPPER[position]; n += 9 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }

    for ( n = position - 9; n >= RIGHT_LOWER[position]; n -= 9 ) {
      if ( ( allpieces & POW2[n] ) == 0 )
	pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x2 &= NOTPOW2[position];
  };
}

void
GenMoves::tryAllCastle ( const int side, const u64 allpieces ) {
  if ( side == WHITE ) {
    if ( POW2_3 & chessboard[KING_WHITE] && !( allpieces & 0x6ULL ) && RIGHT_CASTLE & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_0 && !attackSquare < WHITE > ( 1 ) && !attackSquare < WHITE > ( 2 ) && !attackSquare < WHITE > ( 3 ) ) {
      pushmove < KING_SIDE_CASTLE_MOVE_MASK > ( -1, -1, WHITE, NO_PROMOTION, -1 );
    }
    if ( POW2_3 & chessboard[KING_WHITE] && !( allpieces & 0x70ULL ) && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_7 && !attackSquare < WHITE > ( 3 ) && !attackSquare < WHITE > ( 4 ) && !attackSquare < WHITE > ( 5 ) ) {
      pushmove < QUEEN_SIDE_CASTLE_MOVE_MASK > ( -1, -1, WHITE, NO_PROMOTION, -1 );
    }
  }
  else {
    if ( POW2_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_KING_CASTLE_BLACK_MASK && !( allpieces & 0x600000000000000ULL )
	 && chessboard[ROOK_BLACK] & POW2_56 && !attackSquare < BLACK > ( 57 ) && !attackSquare < BLACK > ( 58 ) && !attackSquare < BLACK > ( 59 ) ) {
      pushmove < KING_SIDE_CASTLE_MOVE_MASK > ( -1, -1, BLACK, NO_PROMOTION, -1 );
    }
    if ( POW2_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_BLACK_MASK && !( allpieces & 0x7000000000000000ULL )
	 && chessboard[ROOK_BLACK] & POW2_63 && !attackSquare < BLACK > ( 59 ) && !attackSquare < BLACK > ( 60 ) && !attackSquare < BLACK > ( 61 ) ) {
      pushmove < QUEEN_SIDE_CASTLE_MOVE_MASK > ( -1, -1, BLACK, NO_PROMOTION, -1 );
    }
  }
}



bool
GenMoves::performRookQueenCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces ) {
  ASSERT ( piece >= 0 && piece < 12 );
  int n, position;
  u64 x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    for ( n = position + 1; n <= ORIZ_LEFT[position]; n++ ) {
      if ( allpieces & POW2[n] ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    for ( n = position - 1; n >= ORIZ_RIGHT[position]; n-- ) {
      if ( allpieces & POW2[n] ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position + 8; n <= VERT_UPPER[position]; n += 8 ) {
      if ( allpieces & POW2[n] ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    for ( n = position - 8; n >= VERT_LOWER[position]; n -= 8 ) {
      if ( allpieces & POW2[n] ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x2 &= NOTPOW2[position];
  };
  return false;
}

bool
GenMoves::performBishopCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces ) {
  ASSERT ( piece >= 0 && piece < 12 );
  int position, n;
  u64 x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );

    for ( n = position + 7; n <= LEFT_UPPER[position]; n += 7 ) {
      if ( allpieces & POW2[n] ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position - 7; n >= LEFT_LOWER[position]; n -= 7 ) {
      if ( ( allpieces & POW2[n] ) ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position + 9; n <= RIGHT_UPPER[position]; n += 9 ) {
      if ( ( allpieces & POW2[n] ) ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position - 9; n >= RIGHT_LOWER[position]; n -= 9 ) {
      if ( ( allpieces & POW2[n] ) ) {
	if ( enemies & POW2[n] )
	  if ( pushmove < STANDARD_MOVE_MASK > ( position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x2 &= NOTPOW2[position];
  };
  return false;
}

bool
GenMoves::performKnightShiftCapture ( const int piece, const u64 enemies, const int side ) {
  u64 x1;
  int o, pos;
  u64 x = chessboard[piece];
  while ( x ) {
    pos = BITScanForward ( x );
    x1 = enemies & KNIGHT_MASK[pos];
    while ( x1 ) {
      o = BITScanForward ( x1 );
      if ( pushmove < STANDARD_MOVE_MASK > ( pos, o, side, NO_PROMOTION, piece ) )
	return true;
      x1 &= NOTPOW2[o];
    };
    x &= NOTPOW2[pos];
  }
  return false;
}

bool
GenMoves::performKingShiftCapture ( int side, const u64 enemies ) {
  int o;
  int pos = BITScanForward ( chessboard[KING_BLACK + side] );
  ASSERT ( pos != -1 );
  u64 x1 = enemies & NEAR_MASK1[pos];
  while ( x1 ) {
    o = BITScanForward ( x1 );
    if ( pushmove < STANDARD_MOVE_MASK > ( pos, o, side, NO_PROMOTION, KING_BLACK + side ) )
      return true;
    x1 &= NOTPOW2[o];
  };
  return false;
}

template < int side > void
GenMoves::checkJumpPawn ( u64 x, const u64 xallpieces ) {
  x &= TABJUMPPAWN;
  if ( side ) {
    x = ( ( ( x << 8 ) & xallpieces ) << 8 ) & xallpieces;
  }
  else {
    x = ( ( ( x >> 8 ) & xallpieces ) >> 8 ) & xallpieces;
  };
  int o;
  while ( x ) {
    o = BITScanForward ( x );
    pushmove < STANDARD_MOVE_MASK > ( o + ( side ? -16 : 16 ), o, side, NO_PROMOTION, side );
    x &= NOTPOW2[o];
  };
}

template < int side > void
GenMoves::performPawnShift ( const u64 xallpieces ) {
  int o, tt;
  u64 x = chessboard[side];
  if ( x & PAWNS_7_2[side] )
    checkJumpPawn < side > ( x, xallpieces );
  if ( side ) {
    x <<= 8;
    tt = -8;
  }
  else {
    tt = 8;
    x >>= 8;
  };
  x &= xallpieces;
  while ( x ) {
    o = BITScanForward ( x );
    ASSERT ( getPieceAt ( side, POW2[o + tt] ) != SQUARE_FREE );
    ASSERT ( getBitBoard ( side ) & POW2[o + tt] );
    if ( o > 55 || o < 8 ) {
      pushmove < PROMOTION_MOVE_MASK > ( o + tt, o, side, QUEEN_BLACK + side, side );	//queen
      if ( perftMode ) {
	pushmove < PROMOTION_MOVE_MASK > ( o + tt, o, side, KNIGHT_BLACK + side, side );	//knight
	pushmove < PROMOTION_MOVE_MASK > ( o + tt, o, side, BISHOP_BLACK + side, side );	//bishop
	pushmove < PROMOTION_MOVE_MASK > ( o + tt, o, side, ROOK_BLACK + side, side );	//rock
      }
    }
    else
      pushmove < STANDARD_MOVE_MASK > ( o + tt, o, side, NO_PROMOTION, side );
    x &= NOTPOW2[o];
  };
}

template < int side > bool
GenMoves::performPawnCapture ( const u64 enemies, u64 * key ) {
  if ( !chessboard[side] ) {
    if ( enpassantPosition != -1 )
      updateZobristKey ( key, 13, enpassantPosition );
    enpassantPosition = -1;
    return false;
  }
  int GG;
  u64 x;
  if ( side ) {
    x = ( chessboard[side] << 7 ) & TABCAPTUREPAWN_LEFT & enemies;
    GG = -7;
  }
  else {
    x = ( chessboard[side] >> 7 ) & TABCAPTUREPAWN_RIGHT & enemies;
    GG = 7;
  };
  int o;
  while ( x ) {
    o = BITScanForward ( x );
    if ( ( side && o > 55 ) || ( !side && o < 8 ) ) {	//PROMOTION
      if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, QUEEN_BLACK + side, side ) )
	return true;		//queen
      if ( perftMode ) {
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, KNIGHT_BLACK + side, side ) )
	  return true;		//knight
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, ROOK_BLACK + side, side ) )
	  return true;		//rock
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, BISHOP_BLACK + side, side ) )
	  return true;		//bishop
      }
    }
    else if ( pushmove < STANDARD_MOVE_MASK > ( o + GG, o, side, NO_PROMOTION, side ) )
      return true;
    x &= NOTPOW2[o];
  };
  if ( side ) {
    GG = -9;
    x = ( chessboard[side] << 9 ) & TABCAPTUREPAWN_RIGHT & enemies;
  }
  else {
    GG = 9;
    x = ( chessboard[side] >> 9 ) & TABCAPTUREPAWN_LEFT & enemies;
  };
  while ( x ) {
    o = BITScanForward ( x );
    if ( ( side && o > 55 ) || ( !side && o < 8 ) ) {	//PROMOTION
      if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, QUEEN_BLACK + side, side ) )
	return true;		//queen
      if ( perftMode ) {
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, KNIGHT_BLACK + side, side ) )

	  return true;		//knight
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, BISHOP_BLACK + side, side ) )

	  return true;		//bishop
	if ( pushmove < PROMOTION_MOVE_MASK > ( o + GG, o, side, ROOK_BLACK + side, side ) )

	  return true;		//rock
      }
    }
    else if ( pushmove < STANDARD_MOVE_MASK > ( o + GG, o, side, NO_PROMOTION, side ) )
      return true;
    x &= NOTPOW2[o];
  };

  //ENPASSANT
  if ( enpassantPosition != -1 ) {
    x = ENPASSANT_MASK[side ^ 1][enpassantPosition] & chessboard[side];
    while ( x ) {
      o = BITScanForward ( x );
      pushmove < ENPASSANT_MOVE_MASK > ( o, ( side ? enpassantPosition + 8 : enpassantPosition - 8 ), side, NO_PROMOTION, side );
      x &= NOTPOW2[o];
    }
    updateZobristKey ( key, 13, enpassantPosition );
    enpassantPosition = -1;
  }

  return false;
}


u64
GenMoves::getKingAttackers ( const int side ) {
  int kingPosition = BITScanForward ( chessboard[KING_BLACK + side] );
  int xside = side ^ 1;
  u64 attackers = 0;
  attackers |= KNIGHT_MASK[kingPosition] & chessboard[KNIGHT_BLACK + xside];
  attackers |= NEAR_MASK1[kingPosition] & chessboard[KING_BLACK + xside];
  attackers |= PAWN_CAPTURE_MASK[side][kingPosition] & chessboard[PAWN_BLACK + xside];
  if ( !( ( RANK_FILE[kingPosition] & ( chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside] ) )
	  | ( LEFT_RIGHT_DIAG[kingPosition] & ( chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside] ) ) )
     ) {
    return attackers;
  }
  u64 allpieces = getBitBoard < WHITE > (  ) | getBitBoard < BLACK > (  ) | POW2[kingPosition];

  //right
  u64 qb = chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside];
  for ( int n = kingPosition + 9; n <= RIGHT_UPPER[kingPosition]; n += 9 ) {
    if ( qb & POW2[n] )
      attackers |= POW2[n];
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = kingPosition - 9; n >= RIGHT_LOWER[kingPosition]; n -= 9 ) {
    if ( qb & POW2[n] )
      attackers |= POW2[n];
    if ( allpieces & POW2[n] )
      break;
  }
  // left
  for ( int n = kingPosition + 7; n <= LEFT_UPPER[kingPosition]; n += 7 ) {
    if ( qb & POW2[n] )
      attackers |= POW2[n];
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = kingPosition - 7; n >= LEFT_LOWER[kingPosition]; n -= 7 ) {
    if ( qb & POW2[n] )
      attackers |= POW2[n];
    if ( allpieces & POW2[n] )
      break;
  }

  //file
  u64 qr = chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside];
  for ( int n = kingPosition + 8; n <= VERT_UPPER[kingPosition]; n += 8 ) {
    if ( qr & POW2[n] ) {
      attackers |= POW2[n];
    }
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = kingPosition - 8; n >= VERT_LOWER[kingPosition]; n -= 8 ) {
    if ( qr & POW2[n] ) {
      attackers |= POW2[n];
    }
    if ( allpieces & POW2[n] )
      break;

  }
  //rank
  for ( int n = kingPosition + 1; n <= ORIZ_LEFT[kingPosition]; n++ ) {
    if ( qr & POW2[n] ) {
      attackers |= POW2[n];
    }
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = kingPosition - 1; n >= ORIZ_RIGHT[kingPosition]; n-- ) {
    if ( qr & POW2[n] ) {
      attackers |= POW2[n];
    }
    if ( allpieces & POW2[n] )
      break;

  }
  return attackers;
}

template < int side > bool
GenMoves::attackSquare ( const uchar Position ) {
  if ( KNIGHT_MASK[Position] & chessboard[KNIGHT_BLACK + ( side ^ 1 )] ) {
    return true;
  }
  if ( NEAR_MASK1[Position] & chessboard[KING_BLACK + ( side ^ 1 )] ) {
    return true;
  }
  //enpassant
  if ( PAWN_CAPTURE_MASK[side][Position] & chessboard[PAWN_BLACK + ( side ^ 1 )] ) {
    return true;
  }
  ASSERT ( Position < 64 );
  if ( !( ( RANK_FILE[Position] & ( chessboard[ROOK_BLACK + ( side ^ 1 )] | chessboard[QUEEN_BLACK + ( side ^ 1 )] ) )
	  | ( LEFT_RIGHT_DIAG[Position] & ( chessboard[QUEEN_BLACK + ( side ^ 1 )] | chessboard[BISHOP_BLACK + ( side ^ 1 )] ) ) )
     ) {
    return false;
  }
  u64 allpieces = getBitBoard < WHITE > (  ) | getBitBoard < BLACK > (  ) | POW2[Position];

  //right
  u64 qb = chessboard[QUEEN_BLACK + ( side ^ 1 )] | chessboard[BISHOP_BLACK + ( side ^ 1 )];
  for ( int n = Position + 9; n <= RIGHT_UPPER[Position]; n += 9 ) {
    if ( qb & POW2[n] )
      return true;
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = Position - 9; n >= RIGHT_LOWER[Position]; n -= 9 ) {
    if ( qb & POW2[n] )
      return true;
    if ( allpieces & POW2[n] )
      break;
  }
  // left
  for ( int n = Position + 7; n <= LEFT_UPPER[Position]; n += 7 ) {
    if ( qb & POW2[n] )
      return true;
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = Position - 7; n >= LEFT_LOWER[Position]; n -= 7 ) {
    if ( qb & POW2[n] )
      return true;
    if ( allpieces & POW2[n] )
      break;
  }

  //file
  u64 qr = chessboard[QUEEN_BLACK + ( side ^ 1 )] | chessboard[ROOK_BLACK + ( side ^ 1 )];
  for ( int n = Position + 8; n <= VERT_UPPER[Position]; n += 8 ) {
    if ( qr & POW2[n] ) {
      return true;
    }
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = Position - 8; n >= VERT_LOWER[Position]; n -= 8 ) {
    if ( qr & POW2[n] ) {
      return true;
    }
    if ( allpieces & POW2[n] )
      break;

  }
  //rank
  for ( int n = Position + 1; n <= ORIZ_LEFT[Position]; n++ ) {
    if ( qr & POW2[n] ) {
      return true;
    }
    if ( allpieces & POW2[n] )
      break;
  }
  for ( int n = Position - 1; n >= ORIZ_RIGHT[Position]; n-- ) {
    if ( qr & POW2[n] ) {
      return true;
    }
    if ( allpieces & POW2[n] )
      break;

  }
  return false;
}

void
GenMoves::unPerformCastle ( const int side, const uchar type ) {
  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( getPieceAt ( side, POW2_1 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, POW2_0 ) == 12 );
      ASSERT ( getPieceAt ( side, POW2_3 ) == 12 );
      ASSERT ( getPieceAt ( side, POW2_2 ) == ROOK_WHITE );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | POW2_3 ) & NOTPOW2_1;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | POW2_0 ) & NOTPOW2_2;
    }
    else {
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | POW2_3 ) & NOTPOW2_5;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | POW2_7 ) & NOTPOW2_4;
    }
  }
  else {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | POW2_59 ) & NOTPOW2_57;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | POW2_56 ) & NOTPOW2_58;
    }
    else {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | POW2_59 ) & NOTPOW2_61;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | POW2_63 ) & NOTPOW2_60;
    }
  }
}

template < int side > bool
GenMoves::inCheck ( const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, int promotionPiece ) {
  ASSERT ( !( type & 0xC ) );
  bool result = 0;
  if ( ( type & 0x3 ) == STANDARD_MOVE_MASK ) {
    u64 from1, to1 = -1;
    ASSERT ( pieceFrom != SQUARE_FREE );
    ASSERT ( pieceTo != KING_BLACK );
    ASSERT ( pieceTo != KING_WHITE );

    from1 = chessboard[pieceFrom];
    if ( pieceTo != SQUARE_FREE ) {
      to1 = chessboard[pieceTo];
      chessboard[pieceTo] &= NOTPOW2[to];
    };
    chessboard[pieceFrom] &= NOTPOW2[from];
    chessboard[pieceFrom] |= POW2[to];

    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );

    result = attackSquare < side > ( BITScanForward ( chessboard[KING_BLACK + side] ) );
    chessboard[pieceFrom] = from1;
    if ( pieceTo != SQUARE_FREE )
      chessboard[pieceTo] = to1;
  }
  else if ( ( type & 0x3 ) == PROMOTION_MOVE_MASK ) {
    u64 to1 = 0;
    if ( pieceTo != SQUARE_FREE )
      to1 = chessboard[pieceTo];
    u64 from1 = chessboard[pieceFrom];
    u64 p1 = chessboard[promotionPiece];
    chessboard[pieceFrom] &= NOTPOW2[from];
    if ( pieceTo != SQUARE_FREE )
      chessboard[pieceTo] &= NOTPOW2[to];
    chessboard[promotionPiece] = chessboard[promotionPiece] | POW2[to];
    result = attackSquare < side > ( BITScanForward ( chessboard[KING_BLACK + side] ) );
    if ( pieceTo != SQUARE_FREE )
      chessboard[pieceTo] = to1;
    chessboard[pieceFrom] = from1;
    chessboard[promotionPiece] = p1;
  }
  else if ( ( type & 0x3 ) == ENPASSANT_MOVE_MASK ) {
    u64 to1 = chessboard[side ^ 1];
    u64 from1 = chessboard[side];
    chessboard[side] &= NOTPOW2[from];
    chessboard[side] |= POW2[to];
    if ( side )
      chessboard[side ^ 1] &= NOTPOW2[to - 8];
    else
      chessboard[side ^ 1] &= NOTPOW2[to + 8];
    result = attackSquare < side > ( BITScanForward ( chessboard[KING_BLACK + side] ) );
    chessboard[side ^ 1] = to1;
    chessboard[side] = from1;
  }
#ifdef DEBUG_MODE
  else
    ASSERT ( 0 );
#endif
  return result;
}

void
GenMoves::takeback ( _Tmove * move, u64 * key, const u64 oldkey, bool rep ) {
  if ( rep )
    popStackMove (  );
  *key = oldkey;
  enpassantPosition = -1;
  int pieceFrom, posTo, posFrom, movecapture;
  RIGHT_CASTLE = move->type & 0 b11110000;
  if ( ( move->type & 0 b00000011 ) == STANDARD_MOVE_MASK || ( move->type & 0 b00000011 ) == ENPASSANT_MOVE_MASK ) {
    posTo = move->to;
    posFrom = move->from;
    movecapture = move->capturedPiece;
    ASSERT ( posFrom >= 0 && posFrom < 64 );
    ASSERT ( posTo >= 0 && posTo < 64 );
    pieceFrom = move->pieceFrom;
    chessboard[pieceFrom] = ( chessboard[pieceFrom] & NOTPOW2[posTo] ) | POW2[posFrom];
    if ( movecapture != SQUARE_FREE ) {
      if ( ( ( move->type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) ) {
	chessboard[movecapture] |= POW2[posTo];
      }
      else {
	ASSERT ( movecapture == ( move->side ^ 1 ) );
	if ( move->side ) {
	  chessboard[movecapture] |= POW2[posTo - 8];
	}
	else {
	  chessboard[movecapture] |= POW2[posTo + 8];
	}
      }
    }

  }
  else if ( ( move->type & 0 b00000011 ) == PROMOTION_MOVE_MASK ) {
    posTo = move->to;
    posFrom = move->from;
    movecapture = move->capturedPiece;
    ASSERT ( posTo >= 0 );
    chessboard[( uchar ) move->side] |= POW2[posFrom];
    chessboard[move->promotionPiece] &= NOTPOW2[posTo];
    if ( movecapture != SQUARE_FREE ) {
      chessboard[movecapture] |= POW2[posTo];
    }
  }
  else if ( move->type & 0 b00001100 ) {	//castle
    unPerformCastle ( move->side, move->type );
  }
}
template < uchar type > bool GenMoves::pushmove ( const int from, const int to, const int side, int promotionPiece, int pieceFrom ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  /*if( !perftMode) {
     if (evaluateMobilityMode) {
     if (!(type & 0b00001100)) { //no castle
     ASSERT(from >= 0 && to >= 0);
     structure.mobility[from] |= POW2[to];
     if ((POW2[to] & chessboard[KING_BLACK + (side ^ 1)]))
     structure.kingAttacked[from] = 1;
     ASSERT(listId < MAX_PLY && listId >= 0);
     ASSERT(gen_list[listId][0].score < MAX_MOVE);
     gen_list[listId][0].score++;
     }
     return false;
     }
     } */
  int
    piece_captured = SQUARE_FREE;
  bool
    res = false;
  if ( ( ( type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) && !( type & 0 b00001100 ) ) {
    piece_captured = side ^ 1 ? getPieceAt < WHITE > ( POW2[to] ) : getPieceAt < BLACK > ( POW2[to] );

    if ( piece_captured == KING_BLACK + ( side ^ 1 ) )
      res = true;
  }
  else if ( !( type & 0 b00001100 ) )	//no castle
    piece_captured = side ^ 1;

  if ( !( type & 0 b00001100 ) && perftMode ) {	//no castle
    if ( side == WHITE && inCheck < WHITE > ( from, to, type, pieceFrom, piece_captured, promotionPiece ) )
      return false;
    if ( side == BLACK && inCheck < BLACK > ( from, to, type, pieceFrom, piece_captured, promotionPiece ) )
      return false;
  }
  _Tmove *
    mos;
  ASSERT ( listId >= 0 );
  ASSERT ( listId < MAX_PLY );
  ASSERT ( gen_list[listId].size < MAX_MOVE );
  mos = &gen_list[listId].moveList[gen_list[listId].size];
  ++gen_list[listId].size;
  mos->type = RIGHT_CASTLE | type;
  mos->side = ( char ) side;

  mos->capturedPiece = piece_captured;
  if ( type & 0 b00000011 ) {
    mos->from = ( uchar ) from;
    mos->to = ( uchar ) to;
    mos->pieceFrom = pieceFrom;
    mos->promotionPiece = ( char ) promotionPiece;
    if ( !perftMode ) {
      if ( res == true ) {
	mos->score = _INFINITE;
      }
      else {
	mos->score = 0;
	mos->score += killerHeuristic[from][to];
	mos->score += ( PIECES_VALUE[piece_captured] >= PIECES_VALUE[pieceFrom] ) ? ( PIECES_VALUE[piece_captured] - PIECES_VALUE[pieceFrom] ) * 2 : PIECES_VALUE[piece_captured];
	ASSERT ( pieceFrom >= 0 && pieceFrom < 12 && to >= 0 && to < 64 && from >= 0 && from < 64 );
      }
    }
  }

  else if ( type & 0 b00001100 ) {	//castle
    ASSERT ( RIGHT_CASTLE );
    mos->score = 100;		//TODO OPEN END MIDDLE
  }
  mos->used = false;
  /*u128* a =&gen_list[listId].used;//TODO
     assert(a);
     setBit(a,gen_list[listId].size-1) ; */

  ASSERT ( gen_list[listId].size < MAX_MOVE );
  return res;
}

void
GenMoves::makemove ( _Tmove * move, u64 * key, bool rep ) {
  ASSERT ( move );
  ASSERT ( _bits::bitCount ( chessboard[KING_WHITE] ) == 1 && _bits::bitCount ( chessboard[KING_BLACK] ) == 1 );
  int pieceFrom = SQUARE_FREE, posTo, posFrom, movecapture = SQUARE_FREE;
  uchar RIGHT_CASTLE_old = RIGHT_CASTLE;
  if ( !( move->type & 0 b00001100 ) ) {	//no castle
    posTo = move->to;
    posFrom = move->from;
    ASSERT ( posTo >= 0 );
    movecapture = move->capturedPiece;
    ASSERT ( posFrom >= 0 && posFrom < 64 );
    ASSERT ( posTo >= 0 && posTo < 64 );
    pieceFrom = move->pieceFrom;
    if ( ( move->type & 0 b00000011 ) == PROMOTION_MOVE_MASK ) {
      chessboard[pieceFrom] &= NOTPOW2[posFrom];
      updateZobristKey ( key, pieceFrom, posFrom );
      chessboard[move->promotionPiece] |= POW2[posTo];
      updateZobristKey ( key, move->promotionPiece, posTo );
    }
    else {
      chessboard[pieceFrom] = ( chessboard[pieceFrom] | POW2[posTo] ) & NOTPOW2[posFrom];
      updateZobristKey ( key, pieceFrom, posFrom );
      updateZobristKey ( key, pieceFrom, posTo );
    }

    if ( movecapture != SQUARE_FREE ) {
      if ( ( move->type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) {
	chessboard[movecapture] &= NOTPOW2[posTo];
	updateZobristKey ( key, movecapture, posTo );
      }
      else {
	ASSERT ( movecapture == ( move->side ^ 1 ) );
	if ( move->side ) {
	  chessboard[movecapture] &= NOTPOW2[posTo - 8];
	  updateZobristKey ( key, movecapture, posTo - 8 );
	}
	else {
	  chessboard[movecapture] &= NOTPOW2[posTo + 8];
	  updateZobristKey ( key, movecapture, posTo + 8 );
	}
      }
    }

    //lost castle right

    switch ( pieceFrom ) {
    case KING_WHITE:{
      RIGHT_CASTLE &= 0 b11001111;
    }
      break;
    case KING_BLACK:{
      RIGHT_CASTLE &= 0 b00111111;
    }
      break;

    case ROOK_WHITE:
      if ( posFrom == 0 ) {
	RIGHT_CASTLE &= 0 b11101111;
      }
      else if ( posFrom == 7 ) {
	RIGHT_CASTLE &= 0 b11011111;
      }
      break;
    case ROOK_BLACK:
      if ( posFrom == 56 ) {
	RIGHT_CASTLE &= 0 b10111111;
      }
      else if ( posFrom == 63 ) {
	RIGHT_CASTLE &= 0 b01111111;
      }
      break;
      //en passant
    case PAWN_WHITE:
      if ( ( RANK_1 & POW2[posFrom] ) && ( RANK_3 & POW2[posTo] ) ) {
	enpassantPosition = posTo;
	updateZobristKey ( key, 13, enpassantPosition );
      }
      break;

    case PAWN_BLACK:
      if ( ( RANK_6 & POW2[posFrom] ) && ( RANK_4 & POW2[posTo] ) ) {
	enpassantPosition = posTo;
	updateZobristKey ( key, 13, enpassantPosition );
      }
      break;
    default:
      ;
    }
  }
  else if ( move->type & 0 b00001100 ) {	//castle
    performCastle ( move->side, move->type, key );
    if ( move->side == WHITE ) {
      RIGHT_CASTLE &= 0 b11001111;
    }
    else {
      RIGHT_CASTLE &= 0 b00111111;
    }
  }
  u64 x2 = RIGHT_CASTLE_old ^ RIGHT_CASTLE;
  while ( x2 ) {
    int position = BITScanForward ( x2 );
    updateZobristKey ( key, 14, position );
    x2 &= NOTPOW2[position];
  }

  if ( rep ) {
    if ( movecapture != SQUARE_FREE || pieceFrom == WHITE || pieceFrom == BLACK || move->type & 0 b00001100 )
      pushStackMove ( 0 );
    pushStackMove ( *key );
  }
}

void
GenMoves::init (  ) {
  numMoves = numMovesq = 0;
#ifdef DEBUG_MODE
#ifndef NO_FP_MODE
  nCutFp = nCutRazor = 0;
#endif
  betaEfficency = betaEfficencyCumulative = 0.0;
  nCutAB = 0;
  nNullMoveCut = 0;
#endif
  listId = 0;
}

u64
GenMoves::getTotMoves (  ) {
  return numMoves + numMovesq;
}

void
GenMoves::setRepetitionMapCount ( int i ) {
  repetitionMapCount = i;
}

int
GenMoves::loadFen (  ) {
  return loadFen ( "" );
}

int
GenMoves::loadFen ( string fen ) {
  repetitionMapCount = 0;
  int side = ChessBoard::loadFen ( fen );
  return side;
}

void
GenMoves::makemove ( _Tmove * move ) {
  makemove ( move, &zobristKey, true );
}

int
GenMoves::getMoveFromSan ( const string fenStr, _Tmove * move ) {
  enpassantPosition = -1;
  memset ( move, 0, sizeof ( _Tmove ) );
  static const string MATCH_QUEENSIDE = "O-O-O e1c1 e8c8";
  static const string MATCH_QUEENSIDE_WHITE = "O-O-O e1c1";
  static const string MATCH_KINGSIDE_WHITE = "O-O e1g1";
  static const string MATCH_QUEENSIDE_BLACK = "O-O-O e8c8";
  static const string MATCH_KINGSIDE_BLACK = "O-O e8g8";

  if ( ( ( MATCH_QUEENSIDE_WHITE.find ( fenStr ) != string::npos || MATCH_KINGSIDE_WHITE.find ( fenStr ) != string::npos ) && getPieceAt < WHITE > ( POW2[E1] ) == KING_WHITE )
       || ( ( MATCH_QUEENSIDE_BLACK.find ( fenStr ) != string::npos || MATCH_KINGSIDE_BLACK.find ( fenStr ) != string::npos )
	    && getPieceAt < BLACK > ( POW2[E8] ) == KING_BLACK )
     ) {
    if ( MATCH_QUEENSIDE.find ( fenStr ) != string::npos ) {
      move->type = QUEEN_SIDE_CASTLE_MOVE_MASK;
      move->from = QUEEN_SIDE_CASTLE_MOVE_MASK;
    }
    else {
      move->from = KING_SIDE_CASTLE_MOVE_MASK;
      move->type = KING_SIDE_CASTLE_MOVE_MASK;
    }
    if ( fenStr.find ( "1" ) != string::npos ) {
      move->side = WHITE;

    }
    else if ( fenStr.find ( "8" ) != string::npos ) {
      move->side = BLACK;

    }
    else
      assert ( 0 );
    move->from = -1;
    move->capturedPiece = SQUARE_FREE;
    return move->side;
  }

  int from = -1;
  int to = -1;
  for ( int i = 0; i < 64; i++ ) {
    if ( !fenStr.compare ( 0, 2, BOARD[i] ) ) {
      from = i;
      break;
    }
  }
  if ( from == -1 ) {
    cout << fenStr << endl;
    assert ( 0 );
  }

  for ( int i = 0; i < 64; i++ ) {
    if ( !fenStr.compare ( 2, 2, BOARD[i] ) ) {
      to = i;
      break;
    }
  }
  if ( to == -1 ) {
    cout << fenStr << endl;
    assert ( 0 );
  }

  int pieceFrom;
  if ( ( pieceFrom = getPieceAt < WHITE > ( POW2[from] ) ) != 12 ) {
    move->side = WHITE;
  }
  else if ( ( pieceFrom = getPieceAt < BLACK > ( POW2[from] ) ) != 12 ) {
    move->side = BLACK;
  }
  else {
    display (  );
    cout << "fenStr: " << fenStr << " from: " << from << endl;
    assert ( 0 );
  }
  move->from = from;
  move->to = to;
  if ( fenStr.length (  ) == 4 ) {
    move->type = STANDARD_MOVE_MASK;
    if ( pieceFrom == PAWN_WHITE || pieceFrom == PAWN_BLACK ) {
      if ( FILE_AT[from] != FILE_AT[to] && ( move->side ^ 1 ? getPieceAt < WHITE > ( POW2[to] ) : getPieceAt < BLACK > ( POW2[to] ) ) == SQUARE_FREE ) {
	move->type = ENPASSANT_MOVE_MASK;
      }
    }
  }
  else if ( fenStr.length (  ) == 5 ) {
    move->type = PROMOTION_MOVE_MASK;
    if ( move->side == WHITE )
      move->promotionPiece = INV_FEN[toupper ( fenStr.at ( 4 ) )];
    else
      move->promotionPiece = INV_FEN[( uchar ) fenStr.at ( 4 )];
    ASSERT ( move->promotionPiece != 0xFF );
  }
  if ( move->side == WHITE ) {
    move->capturedPiece = getPieceAt < BLACK > ( POW2[move->to] );
    move->pieceFrom = getPieceAt < WHITE > ( POW2[move->from] );
  }
  else {
    move->capturedPiece = getPieceAt < WHITE > ( POW2[move->to] );
    move->pieceFrom = getPieceAt < BLACK > ( POW2[move->from] );
  }
  return move->side;
}
