#include "stdafx.h"
#include "eval.h"
#include "bitmap.h"
#include "maindefine.h"
#include "utility.h"
#include "gen.h"
#include "search.h"
#include "zobrist.h"
#include "winboard.h"
#include <stdio.h>
#include <sys/timeb.h>
#ifdef _MSC_VER
#include "LIMITS.H"
#endif
#include "extern.h"
#ifdef _MSC_VER
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif

int
performPawnCapture ( const int tipomove, const u64 enemies, const int SIDE ) {
  check_side ( SIDE );
  if ( !Chessboard ( SIDE ) ) {
    ENP_POSSIBILE = -1;
    return 0;
  }
  u64 x;
  int o, GG;
  x = Chessboard ( SIDE );
  if ( SIDE ) {
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
      if ( SIDE == WHITE && o > 55 || SIDE == BLACK && o < 8 ) {
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, QUEEN_BLACK + SIDE ) )
	  return 1;		//queen
#ifdef PERFT_MODE
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, KNIGHT_BLACK + SIDE ) )
	  return 1;		//knight
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, ROOK_BLACK + SIDE ) )
	  return 1;		//rock
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, BISHOP_BLACK + SIDE ) )
	  return 1;		//bishop
#endif
      }
    }
    else if ( pushmove ( tipomove, o + GG, o, SIDE ) )
      return 1;
    x &= NOTTABLOG[o];
  };
  x = Chessboard ( SIDE );
  if ( SIDE ) {
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
      if ( SIDE == WHITE && o > 55 || SIDE == BLACK && o < 8 ) {
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, QUEEN_BLACK + SIDE ) )
	  return 1;		//queen
#ifdef PERFT_MODE
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, KNIGHT_BLACK + SIDE ) )

	  return 1;		//knight
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, BISHOP_BLACK + SIDE ) )

	  return 1;		//bishop
	if ( pushmove ( PROMOTION, o + GG, o, SIDE, ROOK_BLACK + SIDE ) )

	  return 1;		//rock
#endif
      }

    }
    else if ( pushmove ( tipomove, o + GG, o, SIDE ) )
      return 1;
    x &= NOTTABLOG[o];
  };

  int to;			//ENPASSANT
  if ( ( ( SIDE && ENP_POSSIBILE <= 39 && ENP_POSSIBILE >= 32 ) || ( !SIDE && ENP_POSSIBILE >= 24 && ENP_POSSIBILE <= 31 ) ) && ( ( x = EN_PASSANT_MASK[change_side ( SIDE )][ENP_POSSIBILE] & Chessboard ( SIDE ) ) ) ) {
    if ( SIDE )
      to = ENP_POSSIBILE + 8;
    else
      to = ENP_POSSIBILE - 8;
    while ( x ) {
      o = BITScanForward ( x );
      if ( pushmove ( ENPASSANT, o, to, SIDE ) )
	return 1;
      x &= NOTTABLOG[o];
    }
  }
  ENP_POSSIBILE = -1;
  return 0;
}

int
performTowerQueenCapture ( const int tipomove, const int pezzo, const u64 enemies, const int SIDE, const u64 ALLPIECES ) {
  check_side ( SIDE );
  u64 xx = 0, x2;
  int o, pos;
  x2 = Chessboard ( pezzo );
  while ( x2 ) {
    pos = BITScanForward ( x2 );
    ASSERT ( pos != -1 );
    ASSERT ( pos >= 0 && pos < 64 );
    if ( enemies & ORIZZONTAL[pos] ) {
      xx = MASK_CAPT_MOV2[( uchar ) ( ( ALLPIECES >> pos_posMod8[pos] ) )][pos] & enemies;
    }
    if ( enemies & VERTICAL[pos] ) {
      xx |= enemies & inv_raw90CAPT[rotate_board_90 ( ALLPIECES & VERTICAL[pos] )][pos];
    }
    while ( xx ) {
      o = BITScanForward ( xx );
      if ( pushmove ( tipomove, pos, o, SIDE ) )
	return 1;
      xx &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[pos];
  };				//while
  return 0;
}

int
performBishopCapture ( const int tipomove, const int pezzo, const u64 enemies, const int SIDE, const u64 ALLPIECES ) {
  check_side ( SIDE );
  u64 x = 0, x2;
  int o, position;
  x2 = Chessboard ( pezzo );
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( enemies & LEFT[position] ) {
      /////left
      x = inv_raw_leftCAPT_45[rotate_board_left_45 ( ALLPIECES, position )][position] & enemies;
      ASSERT ( x != -1 );
    };
    if ( enemies & RIGHT[position] ) {
      /////right
      x |= inv_raw_rightCAPT_45[rotate_board_right_45 ( ALLPIECES, position )][position] & enemies;
      ASSERT ( ( inv_raw_rightCAPT_45[rotate_board_right_45 ( ALLPIECES, position )][position] & enemies ) != -1 );
    };
    while ( x ) {
      o = BITScanForward ( x );
      ASSERT ( position != -1 );
      ASSERT ( o != -1 );
      ASSERT ( chessboard[KING_BLACK] );
      ASSERT ( chessboard[KING_WHITE] );
      if ( pushmove ( tipomove, position, o, SIDE ) )
	return 1;
      x &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[position];
  };				//while
  return 0;
}

int
performKnight_Shift_Capture ( const int tipomove, const int pezzo, const u64 pieces, const int SIDE ) {
  check_side ( SIDE );
  u64 x1, x;
  int o, pos;
  x = Chessboard ( pezzo );
  while ( x ) {
    pos = BITScanForward ( x );
    x1 = pieces & KNIGHT_MASK[pos];
    while ( x1 ) {
      o = BITScanForward ( x1 );
      if ( pushmove ( tipomove, pos, o, SIDE ) )
	return 1;
      x1 &= NOTTABLOG[o];
    };
    x &= NOTTABLOG[pos];
  }
  return 0;
}

int
performKing_Shift_Capture ( const int tipomove, const int pezzo, const u64 pieces, const int SIDE ) {
  check_side ( SIDE );
  ASSERT ( pezzo >= 0 && pezzo < 12 );
  u64 x1;
  int o, pos;
  pos = BITScanForward ( chessboard[pezzo] );
  ASSERT ( pos != -1 );
  x1 = pieces & NEAR_MASK[pos];
  while ( x1 ) {
    o = BITScanForward ( x1 );
    if ( pushmove ( tipomove, pos, o, SIDE ) )
      return 1;
    x1 &= NOTTABLOG[o];
  };
  return 0;
}

int
generateCap ( const int tipomove, const int SIDE ) {
  check_side ( SIDE );
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( !running )
    return 0;
  u64 ALLPIECES = square_all_bit_occupied (  );
  u64 enemies = square_bit_occupied ( ( change_side ( SIDE ) ) );
  if ( SIDE == BLACK ) {
    if ( performPawnCapture ( tipomove, enemies, SIDE ) )
      return 1;
    if ( performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, enemies, SIDE ) )
      return 1;
    if ( performBishopCapture ( tipomove, BISHOP_BLACK, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performTowerQueenCapture ( tipomove, ROOK_BLACK, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performTowerQueenCapture ( tipomove, QUEEN_BLACK, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performBishopCapture ( tipomove, QUEEN_BLACK, enemies, SIDE, ALLPIECES ) )
      return 1;

    if ( performKing_Shift_Capture ( tipomove, KING_BLACK, enemies, SIDE ) )
      return 1;
  }
  else {
    ASSERT ( SIDE == WHITE );
    if ( performPawnCapture ( tipomove, enemies, SIDE ) )
      return 1;
    if ( performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, enemies, SIDE ) )
      return 1;
    if ( performBishopCapture ( tipomove, BISHOP_WHITE, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performTowerQueenCapture ( tipomove, ROOK_WHITE, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performTowerQueenCapture ( tipomove, QUEEN_WHITE, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performBishopCapture ( tipomove, QUEEN_WHITE, enemies, SIDE, ALLPIECES ) )
      return 1;
    if ( performKing_Shift_Capture ( tipomove, KING_WHITE, enemies, SIDE ) )
      return 1;
  };
  return 0;
}
