#include "GenMoves.h"
#include "Bits.h"

GenMoves::GenMoves ( char *FEN ):
ChessBoard ( FEN ) {
  evaluateMobilityMode = false;
  gen_list = ( Tmove ** ) calloc ( MAX_PLY, sizeof ( Tmove ** ) );
  for ( int i = 0; i < MAX_PLY; i++ ) {
    gen_list[i] = ( Tmove * ) calloc ( MAX_MOVE, sizeof ( Tmove ) );
#ifdef DEBUG_MODE
#ifndef PERFT_MODE
    for ( int j = 0; j < MAX_MOVE; j++ ) {
      gen_list[i][j].stack_move = ( Tchessboard * ) calloc ( MAX_PLY, sizeof ( Tchessboard ) );
    }
#endif
#endif
  }
  list_id = -1;
}

int
GenMoves::getNextMove ( Tmove * gen_list ) {
  int listcount = gen_list[0].score;
  int bestId = -1;
  int j;
  for ( j = 1; j <= listcount; j++ )
    if ( !gen_list[j].used ) {
      bestId = j;
      break;
    }
  if ( bestId == -1 )
    return -1;
  for ( int i = j; i <= listcount; i++ ) {
    if ( !gen_list[i].used && gen_list[i].score > gen_list[bestId].score )
      bestId = i;
  }
  gen_list[bestId].used = true;
  return bestId;
}

void
GenMoves::setKillerHeuristic ( int from, int to, int value ) {
  KillerHeuristic[from][to] += value;
}

void
GenMoves::initKillerHeuristic (  ) {
  memset ( KillerHeuristic, 0, sizeof ( KillerHeuristic ) );
}

int
GenMoves::generateCap (  ) {
  return generateCap ( STANDARD_MOVE_MASK, !isBlackMove (  ) );
}

void
GenMoves::generateMoves (  ) {
  generateMoves ( STANDARD_MOVE_MASK, !isBlackMove (  ) );
}

uchar
GenMoves::rotateBoardLeft45 ( const u64 ss, const int pos ) {
  u64 xx = ss & LEFT[pos];
  return TABLOG_VERT45[pos] | Bits::ROTATE_LEFT[( xx >> SHIFT_LEFT[pos] ) & 0xFFFFULL] | Bits::ROTATE_LEFT[( ( xx ) >> 16 ) & 0xFFFFULL]
    | Bits::ROTATE_LEFT[( ( xx ) >> 32 ) & 0xFFFFULL] | Bits::ROTATE_LEFT[( ( xx ) >> 48 ) & 0xFFFFULL];
}

uchar
GenMoves::rotateBoardRight45 ( const u64 ss, const int pos ) {
  u64 xx = ( ss & RIGHT[pos] );
  return TABLOG_VERT45[pos] | Bits::ROTATE_RIGHT[( xx >> SHIFT_RIGHT[pos] ) & 0xFFFFULL] | Bits::ROTATE_RIGHT[( ( xx ) >> 16 ) & 0xFFFFULL]
    | Bits::ROTATE_RIGHT[( ( xx ) >> 32 ) & 0xFFFFULL] | Bits::ROTATE_RIGHT[( ( xx ) >> 48 ) & 0xFFFFULL];
}
GenMoves::~GenMoves (  ) {
#ifdef DEBUG_MODE
#ifndef PERFT_MODE
  for ( int i = 0; i < MAX_PLY; i++ )
    for ( int j = 0; j < MAX_MOVE; j++ ) {
      free ( gen_list[i][j].stack_move );
    }
#endif
#endif
  for ( int i = 0; i < MAX_PLY; i++ )
    free ( gen_list[i] );
  free ( gen_list );
}

void
GenMoves::performCastle ( const int side, const uchar type ) {

  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( get_piece_at ( side, TABLOG_3 ) == KING_WHITE );
      ASSERT ( get_piece_at ( side, TABLOG_1 ) == SQUARE_FREE );
      ASSERT ( get_piece_at ( side, TABLOG_2 ) == SQUARE_FREE );
      ASSERT ( get_piece_at ( side, TABLOG_0 ) == ROOK_WHITE );

      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_1 ) & NOTTABLOG_3;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_2 ) & NOTTABLOG_0;
    }
    else {
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_5 ) & NOTTABLOG_3;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_4 ) & NOTTABLOG_7;
    }
  }
  else {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_57 ) & NOTTABLOG_59;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_58 ) & NOTTABLOG_56;
    }
    else {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_61 ) & NOTTABLOG_59;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_60 ) & NOTTABLOG_63;
    }
  }
}

void
GenMoves::checkJumpPawn ( const uchar tipomove, const u64 sc, const int side, const u64 XALLPIECES ) {

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
    pushmove ( tipomove, o + TT, o, side, NO_PROMOTION );
    x &= NOTTABLOG[o];
  };
}

void
GenMoves::performRookQueenShift ( const uchar tipomove, const int piece, const int side, const u64 ALLPIECES ) {

  ASSERT ( piece >= 0 && piece < 12 );
  u64 xx = 0, x2;
  int o, pos;
  x2 = chessboard[piece];
  while ( x2 ) {
    pos = BITScanForward ( x2 );
    if ( ( ORIZZ_BOUND[pos] & ALLPIECES ) != ORIZZ_BOUND[pos] )
      xx = MASK_MOV[( uchar ) ( ALLPIECES >> pos_posMod8[pos] )][pos];
    if ( ( VERT_BOUND[pos] & ALLPIECES ) != VERT_BOUND[pos] ) {
      ASSERT ( rotateBoard90 ( ALLPIECES & VERTICAL[pos] ) > 0 );
      xx |= inv_raw90MOV[rotateBoard90 ( ALLPIECES & VERTICAL[pos] )][pos];
    }
    while ( xx ) {
      o = BITScanForward ( xx );
      pushmove ( tipomove, pos, o, side, NO_PROMOTION );
      xx &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[pos];
  };
}

void
GenMoves::performBishopShift ( const uchar tipomove, const int piece, const int side, const u64 ALLPIECES ) {

  ASSERT ( piece >= 0 && piece < 12 );
  u64 x = 0, x2;
  int o, position;
  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( ( LEFT_BOUND[position] & ALLPIECES ) != LEFT_BOUND[position] ) {
      x = inv_raw_leftMOV_45[rotateBoardLeft45 ( ALLPIECES, position )][position];
      ASSERT ( x );

    }
    if ( ( RIGHT_BOUND[position] & ALLPIECES ) != RIGHT_BOUND[position] ) {
      /////right
      x |= inv_raw_rightMOV_45[rotateBoardRight45 ( ALLPIECES, position )][position];
    }
    while ( x ) {
      o = BITScanForward ( x );
      ASSERT ( position != -1 );
      ASSERT ( o != -1 );
      pushmove ( tipomove, position, o, side, NO_PROMOTION );
      x &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[position];
  };
}

void
GenMoves::tryAllCastle ( const int side, const u64 ALLPIECES ) {
  if ( side == WHITE ) {
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x6ULL ) && RIGHT_CASTLE & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & TABLOG_0 && !attackSquare ( side, 1 ) && !attackSquare ( side, 2 ) && !attackSquare ( side, 3 ) )
      pushmove ( KING_SIDE_CASTLE_MOVE_MASK, -1, -1, WHITE, NO_PROMOTION );
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x70ULL ) && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & TABLOG_7 && !attackSquare ( side, 3 ) && !attackSquare ( side, 4 ) && !attackSquare ( side, 5 ) )
      pushmove ( QUEEN_SIDE_CASTLE_MOVE_MASK, -1, -1, WHITE, NO_PROMOTION );
  }
  else {
    if ( TABLOG_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_KING_CASTLE_BLACK_MASK && !( ALLPIECES & 0x600000000000000ULL )
	 && chessboard[ROOK_BLACK] & TABLOG_56 && !attackSquare ( side, 57 ) && !attackSquare ( side, 58 ) && !attackSquare ( side, 59 ) )
      pushmove ( KING_SIDE_CASTLE_MOVE_MASK, -1, -1, BLACK, NO_PROMOTION );
    if ( TABLOG_59 & chessboard[KING_BLACK] && RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_BLACK_MASK && !( ALLPIECES & 0x7000000000000000ULL )
	 && chessboard[ROOK_BLACK] & TABLOG_63 && !attackSquare ( side, 59 ) && !attackSquare ( side, 60 ) && !attackSquare ( side, 61 ) )
      pushmove ( QUEEN_SIDE_CASTLE_MOVE_MASK, -1, -1, BLACK, NO_PROMOTION );
  }
}

void
GenMoves::performPawnShift ( const uchar tipomove, const int side, const u64 XALLPIECES ) {

  int o, tt;
  u64 x;
  x = chessboard[side];
  if ( x & PAWNS_7_2[side] )
    checkJumpPawn ( tipomove, chessboard[side], side, XALLPIECES );
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
    ASSERT ( get_piece_at ( side, TABLOG[o + tt] ) != SQUARE_FREE );
    ASSERT ( squareBitOccupied ( side ) & TABLOG[o + tt] );
    if ( o > 55 || o < 8 ) {
      if ( ( ( side == WHITE ) && ( o > 55 ) ) || ( ( side == BLACK ) && ( o < 8 ) ) ) {
	pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, QUEEN_BLACK + side );	//queen
#ifdef PERFT_MODE
	pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, KNIGHT_BLACK + side );	//knight
	pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, BISHOP_BLACK + side );	//bishop
	pushmove ( PROMOTION_MOVE_MASK, o + tt, o, side, ROOK_BLACK + side );	//rock
#endif
      }
    }
    else
      pushmove ( tipomove, o + tt, o, side, NO_PROMOTION );
    x &= NOTTABLOG[o];
  };
}

void
GenMoves::generateMoves ( const uchar tipomove, const int side ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  u64 ALLPIECES = squareAllBitOccupied (  );
  if ( side == BLACK ) {
    tryAllCastle ( BLACK, ALLPIECES );
    performBishopShift ( tipomove, BISHOP_BLACK, side, ALLPIECES );
    performRookQueenShift ( tipomove, ROOK_BLACK, side, ALLPIECES );
    performRookQueenShift ( tipomove, QUEEN_BLACK, side, ALLPIECES );
    performBishopShift ( tipomove, QUEEN_BLACK, side, ALLPIECES );
    performPawnShift ( tipomove, side, ~ALLPIECES );
    performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, ~ALLPIECES, side );
    performKing_Shift_Capture ( tipomove, KING_BLACK, ~ALLPIECES, side );
  }
  else {
    ASSERT ( side == WHITE );
    tryAllCastle ( WHITE, ALLPIECES );
    performBishopShift ( tipomove, BISHOP_WHITE, side, ALLPIECES );
    performRookQueenShift ( tipomove, ROOK_WHITE, side, ALLPIECES );
    performRookQueenShift ( tipomove, QUEEN_WHITE, side, ALLPIECES );
    performBishopShift ( tipomove, QUEEN_WHITE, side, ALLPIECES );
    performPawnShift ( tipomove, side, ~ALLPIECES );
    performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, ~ALLPIECES, side );
    performKing_Shift_Capture ( tipomove, KING_WHITE, ~ALLPIECES, side );
  };
}

int
GenMoves::performPawnCapture ( const uchar tipomove, const u64 enemies, const int side ) {

  if ( !chessboard[side] ) {
    enpassantPosition = -1;
    return 0;
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
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, QUEEN_BLACK + side ) )
	  return 1;		//queen
#ifdef PERFT_MODE
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, KNIGHT_BLACK + side ) )
	  return 1;		//knight
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, ROOK_BLACK + side ) )
	  return 1;		//rock
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, BISHOP_BLACK + side ) )
	  return 1;		//bishop
#endif
      }
    }
    else if ( pushmove ( tipomove, o + GG, o, side, NO_PROMOTION ) )
      return 1;
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
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, QUEEN_BLACK + side ) )
	  return 1;		//queen
#ifdef PERFT_MODE
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, KNIGHT_BLACK + side ) )

	  return 1;		//knight
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, BISHOP_BLACK + side ) )

	  return 1;		//bishop
	if ( pushmove ( PROMOTION_MOVE_MASK, o + GG, o, side, ROOK_BLACK + side ) )

	  return 1;		//rock
#endif
      }

    }
    else if ( pushmove ( tipomove, o + GG, o, side, NO_PROMOTION ) )
      return 1;
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
      if ( pushmove ( ENPASSANT_MOVE_MASK, o, to, side, NO_PROMOTION ) )
	return 1;
      x &= NOTTABLOG[o];
    }
  }
  enpassantPosition = -1;
  return 0;
}

int
GenMoves::performRookQueenCapture ( const uchar tipomove, const int piece, const u64 enemies, const int side, const u64 ALLPIECES ) {

  u64 xx = 0, x2;
  int o, pos;
  x2 = chessboard[piece];
  while ( x2 ) {
    pos = BITScanForward ( x2 );
    ASSERT ( pos != -1 );
    ASSERT ( pos >= 0 && pos < 64 );
    if ( enemies & ORIZZONTAL[pos] ) {
      xx = MASK_CAPT_MOV2[( uchar ) ( ( ALLPIECES >> pos_posMod8[pos] ) )][pos] & enemies;
    }
    if ( enemies & VERTICAL[pos] ) {
      xx |= enemies & inv_raw90CAPT[rotateBoard90 ( ALLPIECES & VERTICAL[pos] )][pos];
    }
    while ( xx ) {
      o = BITScanForward ( xx );
      if ( pushmove ( tipomove, pos, o, side, NO_PROMOTION ) )
	return 1;
      xx &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[pos];
  };				//while
  return 0;
}

int
GenMoves::performBishopCapture ( const uchar tipomove, const int piece, const u64 enemies, const int side, const u64 ALLPIECES ) {

  u64 x = 0, x2;
  int o, position;
  x2 = chessboard[piece];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( enemies & LEFT[position] ) {
      /////left
      x = inv_raw_leftCAPT_45[rotateBoardLeft45 ( ALLPIECES, position )][position] & enemies;
      ASSERT ( x != -1 );
    };
    if ( enemies & RIGHT[position] ) {
      /////right
      x |= inv_raw_rightCAPT_45[rotateBoardRight45 ( ALLPIECES, position )][position] & enemies;
      ASSERT ( ( inv_raw_rightCAPT_45[rotateBoardRight45 ( ALLPIECES, position )][position] & enemies ) != -1 );
    };
    while ( x ) {
      o = BITScanForward ( x );
      ASSERT ( position != -1 );
      ASSERT ( o != -1 );
      ASSERT ( chessboard[KING_BLACK] );
      ASSERT ( chessboard[KING_WHITE] );
      if ( pushmove ( tipomove, position, o, side, NO_PROMOTION ) )
	return 1;
      x &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[position];
  };				//while
  return 0;
}

int
GenMoves::performKnight_Shift_Capture ( const uchar tipomove, const int piece, const u64 enemies, const int side ) {

  u64 x1, x;
  int o, pos;
  x = chessboard[piece];
  while ( x ) {
    pos = BITScanForward ( x );
    x1 = enemies & KNIGHT_MASK[pos];
    while ( x1 ) {
      o = BITScanForward ( x1 );
      if ( pushmove ( tipomove, pos, o, side, NO_PROMOTION ) )
	return 1;
      x1 &= NOTTABLOG[o];
    };
    x &= NOTTABLOG[pos];
  }
  return 0;
}

int
GenMoves::performKing_Shift_Capture ( const uchar tipomove, const int piece, const u64 enemies, const int side ) {

  ASSERT ( piece >= 0 && piece < 12 );
  u64 x1;
  int o, pos;
  pos = BITScanForward ( chessboard[piece] );
  ASSERT ( pos != -1 );
  x1 = enemies & NEAR_MASK[pos];
  while ( x1 ) {
    o = BITScanForward ( x1 );
    if ( pushmove ( tipomove, pos, o, side, NO_PROMOTION ) )
      return 1;
    x1 &= NOTTABLOG[o];
  };
  return 0;
}

int
GenMoves::generateCap ( const uchar tipomove, const int side ) {
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  u64 ALLPIECES = squareAllBitOccupied (  );
  u64 enemies = squareBitOccupied ( ( side ^ 1 ) );
  if ( side == BLACK ) {
    if ( performPawnCapture ( tipomove, enemies, side ) )
      return 1;
    if ( performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, enemies, side ) )
      return 1;
    if ( performBishopCapture ( tipomove, BISHOP_BLACK, enemies, side, ALLPIECES ) )
      return 1;
    if ( performRookQueenCapture ( tipomove, ROOK_BLACK, enemies, side, ALLPIECES ) )
      return 1;
    if ( performRookQueenCapture ( tipomove, QUEEN_BLACK, enemies, side, ALLPIECES ) )
      return 1;
    if ( performBishopCapture ( tipomove, QUEEN_BLACK, enemies, side, ALLPIECES ) )
      return 1;

    if ( performKing_Shift_Capture ( tipomove, KING_BLACK, enemies, side ) )
      return 1;
  }
  else {
    ASSERT ( side == WHITE );
    if ( performPawnCapture ( tipomove, enemies, side ) )
      return 1;
    if ( performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, enemies, side ) )
      return 1;
    if ( performBishopCapture ( tipomove, BISHOP_WHITE, enemies, side, ALLPIECES ) )
      return 1;
    if ( performRookQueenCapture ( tipomove, ROOK_WHITE, enemies, side, ALLPIECES ) )
      return 1;
    if ( performRookQueenCapture ( tipomove, QUEEN_WHITE, enemies, side, ALLPIECES ) )
      return 1;
    if ( performBishopCapture ( tipomove, QUEEN_WHITE, enemies, side, ALLPIECES ) )
      return 1;
    if ( performKing_Shift_Capture ( tipomove, KING_WHITE, enemies, side ) )
      return 1;
  };
  return 0;
}

int
GenMoves::attackSquare ( const int side, const int Position ) {
  ASSERT ( Position != -1 );
  int Position_Position_mod_8, xside;
  char Position_mod_8;
  u64 ALLPIECES;
  xside = side ^ 1;
  if ( KNIGHT_MASK[Position] & chessboard[KNIGHT_BLACK + xside] ) {
    return 1;
  }
  if ( NEAR_MASK[Position] & chessboard[KING_BLACK + xside] ) {
    return 1;
  }
  //enpassant
  if ( PAWN_CAPTURE_MASK[side][Position] & chessboard[PAWN_BLACK + xside] ) {
    return 1;
  }

  ASSERT ( Position >= 0 && Position < 64 );

  if ( !( VERT_ORIZZ[Position] & ( chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside] )
	  | LEFT_RIGHT[Position] & ( chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside] ) ) ) {
    return 0;
  }
  ALLPIECES = squareAllBitOccupied (  ) | TABLOG[Position];
  Position_mod_8 = ROT45[Position];

  ASSERT ( Position >= 0 && Position < 64 );

  Position_Position_mod_8 = pos_posMod8[Position];
  if ( MASK_CAPT_MOV[( uchar ) ( ( ( ALLPIECES >> Position_Position_mod_8 ) ) )][Position_mod_8]
       & ( ( ( chessboard[ROOK_BLACK + xside] >> Position_Position_mod_8 ) & 255 )
	   | ( ( chessboard[QUEEN_BLACK + xside] >> Position_Position_mod_8 ) & 255 ) ) ) {
    return 1;
  }
  //left
  if ( MASK_CAPT_MOV[rotateBoardLeft45 ( ALLPIECES, Position )][Position_mod_8]
       & ( ( rotateBoardLeft45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotateBoardLeft45 ( chessboard[QUEEN_BLACK + xside], Position ) ) ) ) {
    return 1;
  }
  /*right \ */
  if ( MASK_CAPT_MOV[rotateBoardRight45 ( ALLPIECES, Position ) | TABLOG[Position_mod_8]][Position_mod_8]
       & ( rotateBoardRight45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotateBoardRight45 ( chessboard[QUEEN_BLACK + ( xside )], Position ) ) ) {
    return 1;
  }
  ASSERT ( Position >= 0 && Position < 64 );
  if ( MASK_CAPT_MOV[rotateBoard90 ( ALLPIECES & VERTICAL[Position] )][ROT45ROT_90_MASK[Position]] & ( ( rotateBoard90 ( chessboard[ROOK_BLACK + xside]
															 & VERTICAL[Position] ) | rotateBoard90 ( chessboard[QUEEN_BLACK + xside] & VERTICAL[Position] ) ) ) ) {
    return 1;
  }
  return 0;
}

void
GenMoves::unperformCastle ( const int side, const uchar type ) {
  if ( side == WHITE ) {
    if ( type & KING_SIDE_CASTLE_MOVE_MASK ) {
      ASSERT ( get_piece_at ( side, TABLOG_1 ) == KING_WHITE );
      ASSERT ( get_piece_at ( side, TABLOG_0 ) == 12 );
      ASSERT ( get_piece_at ( side, TABLOG_3 ) == 12 );
      ASSERT ( get_piece_at ( side, TABLOG_2 ) == ROOK_WHITE );
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

#ifdef PERFT_MODE
int
GenMoves::inCheck ( const int da, const int a, const uchar tipo, const int piece_from, const int piece_to, const int SIDE, int promotion_piece ) {
  ASSERT ( !( tipo & 0xC ) );
  int result = 0;
  if ( ( tipo & 0x3 ) == STANDARD_MOVE_MASK ) {
    u64 da1, a1 = -1;
    ASSERT ( piece_from != SQUARE_FREE );
    ASSERT ( piece_to != KING_BLACK );
    ASSERT ( piece_to != KING_WHITE );

    da1 = chessboard[piece_from];
    if ( piece_to != SQUARE_FREE ) {
      a1 = chessboard[piece_to];
      chessboard[piece_to] &= NOTTABLOG[a];
    };
    chessboard[piece_from] &= NOTTABLOG[da];
    chessboard[piece_from] |= TABLOG[a];

    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );

    result = attackSquare ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    chessboard[piece_from] = da1;
    if ( piece_to != SQUARE_FREE )
      chessboard[piece_to] = a1;
  }
  else if ( ( tipo & 0x3 ) == PROMOTION_MOVE_MASK ) {
    u64 a1 = 0;
    if ( piece_to != SQUARE_FREE )
      a1 = chessboard[piece_to];
    u64 da1 = chessboard[piece_from];
    u64 p1 = chessboard[promotion_piece];
    chessboard[piece_from] &= NOTTABLOG[da];
    if ( piece_to != SQUARE_FREE )
      chessboard[piece_to] &= NOTTABLOG[a];
    chessboard[promotion_piece] = chessboard[promotion_piece] | TABLOG[a];
    result = attackSquare ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    if ( piece_to != SQUARE_FREE )
      chessboard[piece_to] = a1;
    chessboard[piece_from] = da1;
    chessboard[promotion_piece] = p1;
  }
  else if ( ( tipo & 0x3 ) == ENPASSANT_MOVE_MASK ) {
    u64 a1 = chessboard[SIDE ^ 1];
    u64 da1 = chessboard[SIDE];
    chessboard[SIDE] &= NOTTABLOG[da];
    chessboard[SIDE] |= TABLOG[a];
    if ( SIDE )
      chessboard[SIDE ^ 1] &= NOTTABLOG[a - 8];
    else
      chessboard[SIDE ^ 1] &= NOTTABLOG[a + 8];
    result = attackSquare ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    chessboard[SIDE ^ 1] = a1;
    chessboard[SIDE] = da1;
  }
#ifdef DEBUG_MODE
  else
    ASSERT ( 0 );
#endif
  return result;
}
#endif
void
GenMoves::init (  ) {
  enpassantPosition = -1;
  num_moves = num_movesq = 0;
#ifdef FP_MODE
#ifdef DEBUG_MODE
  n_cut_fp = n_cut_razor = 0;
#endif
#endif
  list_id = 0;
#ifndef PERFT_MODE

#ifdef DEBUG_MODE
  beta_efficency = 0.0;
  n_cut = 0;
  null_move_cut = 0;
#endif
#endif
#ifdef HASH_MODE
  n_record_hash_eval = n_cut_hash_eval = 0;
#ifdef DEBUG_MODE
  n_cut_hash = n_record_hash = collisions = 0;
#endif
#endif

}

u64
GenMoves::getTonMoves (  ) {
  return num_moves + num_movesq;
}
