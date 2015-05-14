#include "GenMoves.h"
#include "utils.h"

GenMoves::GenMoves (  ) {
  perftMode = false;
  currentPly = 0;
  evaluateMobilityMode = false;
  gen_list = ( _Tmove ** ) calloc ( MAX_PLY, sizeof ( _Tmove ** ) );
  for ( int i = 0; i < MAX_PLY; i++ ) {
    gen_list[i] = ( _Tmove * ) calloc ( MAX_MOVE, sizeof ( _Tmove ) );
  }
  list_id = -1;
}

/*
void GenMoves::resetRepetitionCount() {
    repetitionMapCount=0;

}*/
void
GenMoves::setPerft ( bool b ) {
  perftMode = b;
}

void
GenMoves::setKillerHeuristic ( int from, int to, int value ) {
  if ( from < 0 || from > 63 || to < 0 || to > 63 )
    return;
  killerHeuristic[from][to] = value;
}

int
GenMoves::evaluateMobility ( const int side ) {
  evaluateMobilityMode = true;
  incListId (  );
  u64 dummy = 0;
  generateCap ( side, structure.allPiecesSide[side ^ 1], structure.allPiecesSide[side], &dummy );
  generateMoves ( side, structure.allPieces );
  int listcount = getListCount (  );
  if ( listcount == -1 )
    listcount = 0;
  resetList (  );
  decListId (  );
  evaluateMobilityMode = false;
  return listcount;
}

void
GenMoves::incKillerHeuristic ( int from, int to, int value ) {
  if ( from == to || ( from < 0 || from > 63 || to < 0 || to > 63 ) )
    return;
#ifdef DEBUG_MODE
  if ( killerHeuristic[from][to] > killerHeuristic[from][to] + value ) {
    cout << killerHeuristic[from][to] << " " << value << endl;
    ASSERT ( 0 );
  }
#endif
  killerHeuristic[from][to] += value;
}

void
GenMoves::clearKillerHeuristic (  ) {
  memset ( killerHeuristic, 0, sizeof ( killerHeuristic ) );
}

int
GenMoves::getNextMove ( _Tmove * gen_list1 ) {

  int listcount = gen_list1[0].score;
  int bestId = -1;
  int j;
  for ( j = 1; j <= listcount; j++ )
    if ( !gen_list1[j].used ) {
      bestId = j;
      break;
    }
  if ( bestId == -1 )
    return -1;
  for ( int i = j; i <= listcount; i++ ) {
    if ( !gen_list1[i].used && gen_list1[i].score > gen_list1[bestId].score )
      bestId = i;
  }
  gen_list1[bestId].used = true;
  return bestId;
}


GenMoves::~GenMoves (  ) {

  for ( int i = 0; i < MAX_PLY; i++ )
    free ( gen_list[i] );
  free ( gen_list );
}

void
GenMoves::performCastle ( const int side, const uchar type, u64 * key ) {

  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( getPieceAt ( side, TABLOG_3 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, TABLOG_1 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_2 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_0 ) == ROOK_WHITE );

      updateZobristKey ( key, KING_WHITE, 3 );
      updateZobristKey ( key, KING_WHITE, 1 );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_1 ) & NOTTABLOG_3;

      updateZobristKey ( key, ROOK_WHITE, 2 );
      updateZobristKey ( key, ROOK_WHITE, 0 );
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_2 ) & NOTTABLOG_0;

    }
    else {
      ASSERT ( type & QUEEN_SIDE_CASTLE_MOVE_MASK );

      ASSERT ( getPieceAt ( side, TABLOG_3 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, TABLOG_4 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_5 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_6 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_7 ) == ROOK_WHITE );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_5 ) & NOTTABLOG_3;

      updateZobristKey ( key, KING_WHITE, 5 );
      updateZobristKey ( key, KING_WHITE, 3 );
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_4 ) & NOTTABLOG_7;

      updateZobristKey ( key, ROOK_WHITE, 4 );
      updateZobristKey ( key, ROOK_WHITE, 7 );
    }
  }
  else {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( getPieceAt ( side, TABLOG_59 ) == KING_BLACK );
      ASSERT ( getPieceAt ( side, TABLOG_58 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_57 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_56 ) == ROOK_BLACK );

      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_57 ) & NOTTABLOG_59;

      updateZobristKey ( key, KING_BLACK, 57 );
      updateZobristKey ( key, KING_BLACK, 59 );
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_58 ) & NOTTABLOG_56;

      updateZobristKey ( key, ROOK_BLACK, 58 );
      updateZobristKey ( key, ROOK_BLACK, 56 );
    }
    else {
      ASSERT ( type & QUEEN_SIDE_CASTLE_MOVE_MASK );
      ASSERT ( getPieceAt ( side, TABLOG_59 ) == KING_BLACK );
      ASSERT ( getPieceAt ( side, TABLOG_60 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_61 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_62 ) == SQUARE_FREE );
      ASSERT ( getPieceAt ( side, TABLOG_63 ) == ROOK_BLACK );

      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_61 ) & NOTTABLOG_59;

      updateZobristKey ( key, KING_BLACK, 61 );
      updateZobristKey ( key, KING_BLACK, 59 );
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_60 ) & NOTTABLOG_63;

      updateZobristKey ( key, ROOK_BLACK, 60 );
      updateZobristKey ( key, ROOK_BLACK, 63 );
    }
  }
}

void
GenMoves::checkJumpPawn ( const u64 sc, const int side, const u64 XALLPIECES ) {

  int TT;
  register int o;
  u64 x;
  x = sc & TABJUMPPAWN;
  if ( side ) {
    x <<= 8;
    x &= XALLPIECES;
    x <<= 8;
    x &= XALLPIECES;
    TT = -16;
  }
  else {
    x >>= 8;
    x &= XALLPIECES;
    x >>= 8;
    x &= XALLPIECES;
    TT = 16;
  };
  while ( x ) {
    o = BITScanForward ( x );
    pushmove ( STANDARD_MOVE_MASK, o + TT, o, side, NO_PROMOTION, side );
    x &= NOTTABLOG[o];
  };
}

void
GenMoves::performRookQueenShift ( const int piece, const int side, const u64 ALLPIECES ) {

  ASSERT ( piece >= 0 && piece < 12 );
  u64 x, x2;
  int n, position;
  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    x = ORIZZONTAL[position] & ALLPIECES;
    for ( n = position + 1; n <= ORIZ_LEFT[position]; n++ ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    for ( n = position - 1; n >= ORIZ_RIGHT[position]; n-- ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x = VERTICAL[position] & ALLPIECES;
    for ( n = position + 8; n <= VERT_UPPER[position]; n += 8 ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    for ( n = position - 8; n >= VERT_LOWER[position]; n -= 8 ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x2 &= NOTTABLOG[position];
  };
}

void
GenMoves::performBishopShift ( const int piece, const int side, const u64 ALLPIECES ) {
  ASSERT ( piece >= 0 && piece < 12 );
  u64 x, x2;
  int position;
  int n;

  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    x = LEFT[position] & ALLPIECES;

    for ( n = position + 7; n <= LEFT_UPPER[position]; n += 7 ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }

    for ( n = position - 7; n >= LEFT_LOWER[position]; n -= 7 ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x = RIGHT[position] & ALLPIECES;
    for ( n = position + 9; n <= RIGHT_UPPER[position]; n += 9 ) {
      if ( ( x & TABLOG[n] ) == 0 ) {

	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      }
      else {

	break;
      }
    }

    for ( n = position - 9; n >= RIGHT_LOWER[position]; n -= 9 ) {
      if ( ( x & TABLOG[n] ) == 0 )
	pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece );
      else
	break;
    }
    x2 &= NOTTABLOG[position];
  };
}

void
GenMoves::tryAllCastle ( const int side, const u64 ALLPIECES ) {
  if ( side == WHITE ) {
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x6ULL ) && RIGHT_CASTLE & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & TABLOG_0 && !attackSquare ( side, 1 ) && !attackSquare ( side, 2 ) && !attackSquare ( side, 3 ) ) {
      pushmove ( KING_SIDE_CASTLE_MOVE_MASK, -1, -1, WHITE, NO_PROMOTION, -1 );
    }
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x70ULL ) && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & TABLOG_7 && !attackSquare ( side, 3 ) && !attackSquare ( side, 4 ) && !attackSquare ( side, 5 ) ) {
      pushmove ( QUEEN_SIDE_CASTLE_MOVE_MASK, -1, -1, WHITE, NO_PROMOTION, -1 );
    }
  }
  else {
    if ( TABLOG_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_KING_CASTLE_BLACK_MASK && !( ALLPIECES & 0x600000000000000ULL )
	 && chessboard[ROOK_BLACK] & TABLOG_56 && !attackSquare ( side, 57 ) && !attackSquare ( side, 58 ) && !attackSquare ( side, 59 ) ) {
      pushmove ( KING_SIDE_CASTLE_MOVE_MASK, -1, -1, BLACK, NO_PROMOTION, -1 );
    }
    if ( TABLOG_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_BLACK_MASK && !( ALLPIECES & 0x7000000000000000ULL )
	 && chessboard[ROOK_BLACK] & TABLOG_63 && !attackSquare ( side, 59 ) && !attackSquare ( side, 60 ) && !attackSquare ( side, 61 ) ) {
      pushmove ( QUEEN_SIDE_CASTLE_MOVE_MASK, -1, -1, BLACK, NO_PROMOTION, -1 );
    }
  }
}

void
GenMoves::performPawnShift ( const int side, const u64 XALLPIECES ) {

  int o, tt;
  u64 x;
  x = chessboard[side];
  if ( x & PAWNS_7_2[side] )
    checkJumpPawn ( chessboard[side], side, XALLPIECES );
  if ( side ) {
    x <<= 8;
    tt = -8;
  }
  else {
    tt = 8;
    x >>= 8;
  };
  x &= XALLPIECES;
  while ( x ) {
    o = BITScanForward ( x );
    ASSERT ( getPieceAt ( side, TABLOG[o + tt] ) != SQUARE_FREE );
    ASSERT ( getBitBoard ( side ) & TABLOG[o + tt] );
    if ( o > 55 || o < 8 ) {
      if ( ( ( side == WHITE ) && ( o > 55 ) ) || ( ( side == BLACK ) && ( o < 8 ) ) ) {
	pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, QUEEN_BLACK + side, side );	//queen
	if ( perftMode ) {
	  pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, KNIGHT_BLACK + side, side );	//knight
	  pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, BISHOP_BLACK + side, side );	//bishop
	  pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, ROOK_BLACK + side, side );	//rock
	}
      }
    }
    else
      pushmove ( STANDARD_MOVE_MASK, o + tt, o, side, NO_PROMOTION, side );
    x &= NOTTABLOG[o];
  };
}

void
GenMoves::generateMoves ( const int side, u64 ALLPIECES ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( side == BLACK ) {
    tryAllCastle ( BLACK, ALLPIECES );
    performBishopShift ( BISHOP_BLACK, side, ALLPIECES );
    performRookQueenShift ( ROOK_BLACK, side, ALLPIECES );
    performRookQueenShift ( QUEEN_BLACK, side, ALLPIECES );
    performBishopShift ( QUEEN_BLACK, side, ALLPIECES );
    performPawnShift ( side, ~ALLPIECES );
    performKnightShiftCapture ( KNIGHT_BLACK, ~ALLPIECES, side );
    performKingShiftCapture ( KING_BLACK, ~ALLPIECES, side );
  }
  else {
    ASSERT ( side == WHITE );
    tryAllCastle ( WHITE, ALLPIECES );
    performBishopShift ( BISHOP_WHITE, side, ALLPIECES );
    performRookQueenShift ( ROOK_WHITE, side, ALLPIECES );
    performRookQueenShift ( QUEEN_WHITE, side, ALLPIECES );
    performBishopShift ( QUEEN_WHITE, side, ALLPIECES );
    performPawnShift ( side, ~ALLPIECES );
    performKnightShiftCapture ( KNIGHT_WHITE, ~ALLPIECES, side );
    performKingShiftCapture ( KING_WHITE, ~ALLPIECES, side );
  };
}

bool
GenMoves::performPawnCapture ( const u64 enemies, const int side, u64 * key ) {

  if ( !chessboard[side] ) {
    if ( enpassantPosition != -1 )
      updateZobristKey ( key, 13, enpassantPosition );
    enpassantPosition = -1;
    return false;
  }
  u64 x;
  int o, GG;
  x = chessboard[side];
  if ( side ) {
    x = ( x << 7 ) & TABCAPTUREPAWN_LEFT;
    GG = -7;
  }
  else {
    x = ( x >> 7 ) & TABCAPTUREPAWN_RIGHT;
    GG = 7;
  };
  x &= enemies;
  while ( x ) {
    o = BITScanForward ( x );
    if ( o > 55 || o < 8 ) {	//PROMOTION
      if ( ( side == WHITE && o > 55 ) || ( side == BLACK && o < 8 ) ) {
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, QUEEN_BLACK + side, side ) )
	  return true;		//queen
	if ( perftMode ) {
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, KNIGHT_BLACK + side, side ) )
	    return true;	//knight
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, ROOK_BLACK + side, side ) )
	    return true;	//rock
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, BISHOP_BLACK + side, side ) )
	    return true;	//bishop
	}
      }
    }
    else if ( pushmove ( STANDARD_MOVE_MASK, o + GG, o, side, NO_PROMOTION, side ) )
      return true;
    x &= NOTTABLOG[o];
  };
  x = chessboard[side];
  if ( side ) {
    x <<= 9;
    GG = -9;
    x &= TABCAPTUREPAWN_RIGHT;
  }
  else {
    x >>= 9;
    GG = 9;
    x &= TABCAPTUREPAWN_LEFT;
  };
  x &= enemies;
  while ( x ) {
    o = BITScanForward ( x );
    if ( o > 55 || o < 8 ) {	//PROMOTION
      if ( ( side == WHITE && o > 55 ) || ( side == BLACK && o < 8 ) ) {
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, QUEEN_BLACK + side, side ) )
	  return true;		//queen
	if ( perftMode ) {
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, KNIGHT_BLACK + side, side ) )

	    return true;	//knight
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, BISHOP_BLACK + side, side ) )

	    return true;	//bishop
	  if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, ROOK_BLACK + side, side ) )

	    return true;	//rock
	}
      }
    }
    else if ( pushmove ( STANDARD_MOVE_MASK, o + GG, o, side, NO_PROMOTION, side ) )
      return true;
    x &= NOTTABLOG[o];
  };

  int to;			//ENPASSANT
  if ( ( ( side && enpassantPosition <= 39 && enpassantPosition >= 32 ) || ( !side && enpassantPosition >= 24 && enpassantPosition <= 31 ) )
       && ( ( x = EN_PASSANT_MASK[side ^ 1][enpassantPosition] & chessboard[side] ) ) ) {
    if ( side )
      to = enpassantPosition + 8;
    else
      to = enpassantPosition - 8;
    while ( x ) {
      o = BITScanForward ( x );
      if ( pushmove ( ENPASSANT_MOVE_MASK, o, to, side, NO_PROMOTION, side ) )
	return true;
      x &= NOTTABLOG[o];
    }
  }

  if ( enpassantPosition != -1 )
    updateZobristKey ( key, 13, enpassantPosition );
  enpassantPosition = -1;

  return false;
}

bool
GenMoves::performRookQueenCapture ( const int piece, const u64 enemies, const int side, const u64 ALLPIECES ) {
  ASSERT ( piece >= 0 && piece < 12 );
  u64 x, x2;
  int n, position;
  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    x = ORIZZONTAL[position] & ALLPIECES;
    for ( n = position + 1; n <= ORIZ_LEFT[position]; n++ ) {
      if ( x & TABLOG[n] ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    for ( n = position - 1; n >= ORIZ_RIGHT[position]; n-- ) {
      if ( x & TABLOG[n] ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x = VERTICAL[position] & ALLPIECES;
    for ( n = position + 8; n <= VERT_UPPER[position]; n += 8 ) {
      if ( x & TABLOG[n] ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    for ( n = position - 8; n >= VERT_LOWER[position]; n -= 8 ) {
      if ( x & TABLOG[n] ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x2 &= NOTTABLOG[position];
  };
  return false;
}

bool
GenMoves::performBishopCapture ( const int piece, const u64 enemies, const int side, const u64 ALLPIECES ) {
  ASSERT ( piece >= 0 && piece < 12 );
  u64 x, x2;
  int position;
  int n;

  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    x = LEFT[position] & ALLPIECES;

    for ( n = position + 7; n <= LEFT_UPPER[position]; n += 7 ) {
      if ( x & TABLOG[n] ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position - 7; n >= LEFT_LOWER[position]; n -= 7 ) {
      if ( ( x & TABLOG[n] ) ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x = RIGHT[position] & ALLPIECES;
    for ( n = position + 9; n <= RIGHT_UPPER[position]; n += 9 ) {
      if ( ( x & TABLOG[n] ) ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }

    for ( n = position - 9; n >= RIGHT_LOWER[position]; n -= 9 ) {
      if ( ( x & TABLOG[n] ) ) {
	if ( enemies & TABLOG[n] )
	  if ( pushmove ( STANDARD_MOVE_MASK, position, n, side, NO_PROMOTION, piece ) )
	    return true;
	break;
      }
    }
    x2 &= NOTTABLOG[position];
  };
  return false;
}

bool
GenMoves::performKnightShiftCapture ( const int piece, const u64 enemies, const int side ) {

  u64 x1, x;
  int o, pos;
  x = chessboard[piece];
  while ( x ) {
    pos = BITScanForward ( x );
    x1 = enemies & KNIGHT_MASK[pos];
    while ( x1 ) {
      o = BITScanForward ( x1 );
      if ( pushmove ( STANDARD_MOVE_MASK, pos, o, side, NO_PROMOTION, piece ) )
	return true;
      x1 &= NOTTABLOG[o];
    };
    x &= NOTTABLOG[pos];
  }
  return false;
}

bool
GenMoves::performKingShiftCapture ( const int piece, const u64 enemies, const int side ) {

  ASSERT ( piece >= 0 && piece < 12 );
  u64 x1;
  int o, pos;
  pos = BITScanForward ( chessboard[piece] );
  ASSERT ( pos != -1 );
  x1 = enemies & NEAR_MASK[pos];
  while ( x1 ) {
    o = BITScanForward ( x1 );
    if ( pushmove ( STANDARD_MOVE_MASK, pos, o, side, NO_PROMOTION, piece ) )
      return true;
    x1 &= NOTTABLOG[o];
  };
  return false;
}


bool
GenMoves::generateCap ( const int side, u64 enemies, u64 friends, u64 * key ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  u64 ALLPIECES = enemies | friends;
  if ( side == BLACK ) {

    if ( performPawnCapture ( enemies, side, key ) )
      return true;

    if ( performKnightShiftCapture ( KNIGHT_BLACK, enemies, side ) )
      return true;

    if ( performBishopCapture ( BISHOP_BLACK, enemies, side, ALLPIECES ) )
      return true;

    if ( performRookQueenCapture ( ROOK_BLACK, enemies, side, ALLPIECES ) )
      return true;

    if ( performRookQueenCapture ( QUEEN_BLACK, enemies, side, ALLPIECES ) )
      return true;
    if ( performBishopCapture ( QUEEN_BLACK, enemies, side, ALLPIECES ) )
      return true;

    if ( performKingShiftCapture ( KING_BLACK, enemies, side ) )
      return true;
  }
  else {
    ASSERT ( side == WHITE );

    if ( performPawnCapture ( enemies, side, key ) )
      return true;

    if ( performKnightShiftCapture ( KNIGHT_WHITE, enemies, side ) )
      return true;

    if ( performBishopCapture ( BISHOP_WHITE, enemies, side, ALLPIECES ) )
      return true;

    if ( performRookQueenCapture ( ROOK_WHITE, enemies, side, ALLPIECES ) )
      return true;

    if ( performRookQueenCapture ( QUEEN_WHITE, enemies, side, ALLPIECES ) )
      return true;
    if ( performBishopCapture ( QUEEN_WHITE, enemies, side, ALLPIECES ) )
      return true;

    if ( performKingShiftCapture ( KING_WHITE, enemies, side ) )
      return true;
  }
  return false;
}

bool
GenMoves::attackSquare ( const int side, const uchar Position ) {
  int Position_Position_mod_8, xside;
  u64 ALLPIECES;
  xside = side ^ 1;
  if ( KNIGHT_MASK[Position] & chessboard[KNIGHT_BLACK + xside] ) {
    return true;
  }
  if ( NEAR_MASK[Position] & chessboard[KING_BLACK + xside] ) {
    return true;
  }
//enpassant
  if ( PAWN_CAPTURE_MASK[side][Position] & chessboard[PAWN_BLACK + xside] ) {
    return true;
  }
  ASSERT ( Position < 64 );
  if ( !( ( VERT_ORIZZ[Position] & ( chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside] ) )
	  | ( LEFT_RIGHT[Position] & ( chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside] ) ) )
     ) {
    return false;
  }
  ALLPIECES = getBitBoard ( WHITE ) | getBitBoard ( BLACK ) | TABLOG[Position];
  uchar Position_mod_8 = ROT45[Position];

  ASSERT ( Position < 64 );

  Position_Position_mod_8 = pos_posMod8[Position];
  if ( MASK_CAPT_MOV[( uchar ) ( ( ( ALLPIECES >> Position_Position_mod_8 ) ) )][Position_mod_8]
       & ( ( ( chessboard[ROOK_BLACK + xside] >> Position_Position_mod_8 ) & 255 )
	   | ( ( chessboard[QUEEN_BLACK + xside] >> Position_Position_mod_8 ) & 255 ) ) ) {
    return true;
  }
  //left
  if ( MASK_CAPT_MOV[rotateBoardLeft45 ( ALLPIECES, Position )][Position_mod_8]
       & ( ( rotateBoardLeft45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotateBoardLeft45 ( chessboard[QUEEN_BLACK + xside], Position ) ) ) ) {
    return true;
  }
  //right
  if ( MASK_CAPT_MOV[rotateBoardRight45 ( ALLPIECES, Position ) | TABLOG[Position_mod_8]][Position_mod_8]
       & ( rotateBoardRight45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotateBoardRight45 ( chessboard[QUEEN_BLACK + ( xside )], Position ) ) ) {
    return true;
  }
  ASSERT ( Position < 64 );
  if ( MASK_CAPT_MOV[rotateBoard90 ( ALLPIECES & VERTICAL[Position] )][ROT45ROT_90_MASK[Position]] & ( ( rotateBoard90 ( chessboard[ROOK_BLACK + xside]
															 & VERTICAL[Position] ) | rotateBoard90 ( chessboard[QUEEN_BLACK + xside] & VERTICAL[Position] ) ) ) ) {
    return true;
  }
  return false;
}

void
GenMoves::unPerformCastle ( const int side, const uchar type ) {
  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( getPieceAt ( side, TABLOG_1 ) == KING_WHITE );
      ASSERT ( getPieceAt ( side, TABLOG_0 ) == 12 );
      ASSERT ( getPieceAt ( side, TABLOG_3 ) == 12 );
      ASSERT ( getPieceAt ( side, TABLOG_2 ) == ROOK_WHITE );
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_3 ) & NOTTABLOG_1;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_0 ) & NOTTABLOG_2;
    }
    else {
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_3 ) & NOTTABLOG_5;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_7 ) & NOTTABLOG_4;
    }
  }
  else {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_59 ) & NOTTABLOG_57;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_56 ) & NOTTABLOG_58;
    }
    else {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_59 ) & NOTTABLOG_61;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_63 ) & NOTTABLOG_60;
    }
  }
}

bool
GenMoves::inCheck ( const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, const int side, int promotionPiece ) {
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
      chessboard[pieceTo] &= NOTTABLOG[to];
    };
    chessboard[pieceFrom] &= NOTTABLOG[from];
    chessboard[pieceFrom] |= TABLOG[to];

    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );

    result = attackSquare ( side, BITScanForward ( chessboard[KING_BLACK + side] ) );
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
    chessboard[pieceFrom] &= NOTTABLOG[from];
    if ( pieceTo != SQUARE_FREE )
      chessboard[pieceTo] &= NOTTABLOG[to];
    chessboard[promotionPiece] = chessboard[promotionPiece] | TABLOG[to];
    result = attackSquare ( side, BITScanForward ( chessboard[KING_BLACK + side] ) );
    if ( pieceTo != SQUARE_FREE )
      chessboard[pieceTo] = to1;
    chessboard[pieceFrom] = from1;
    chessboard[promotionPiece] = p1;
  }
  else if ( ( type & 0x3 ) == ENPASSANT_MOVE_MASK ) {
    u64 to1 = chessboard[side ^ 1];
    u64 from1 = chessboard[side];
    chessboard[side] &= NOTTABLOG[from];
    chessboard[side] |= TABLOG[to];
    if ( side )
      chessboard[side ^ 1] &= NOTTABLOG[to - 8];
    else
      chessboard[side ^ 1] &= NOTTABLOG[to + 8];
    result = attackSquare ( side, BITScanForward ( chessboard[KING_BLACK + side] ) );
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
GenMoves::init (  ) {
  numMoves = numMovesq = 0;
#ifndef NO_FP_MODE
#ifdef DEBUG_MODE
  n_cut_fp = n_cut_razor = 0;
#endif
#endif
  list_id = 0;
#ifdef DEBUG_MODE
  beta_efficency = beta_efficency_tot = 0.0;
  n_cut = 0;
  null_move_cut = 0;
#endif
}

u64
GenMoves::getTotMoves (  ) {
  return numMoves + numMovesq;
}
