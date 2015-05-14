/*
Copyright (C) 2008-2010
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
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
#include <pthread.h>
#include <unistd.h>
#endif

int
performPawnCapture ( const int tipomove, const u64 enemies, const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  if ( !chessboard[SIDE] ) {
    ENP_POSSIBILE = -1;
    return 0;
  }
  u64 x;
  int o, GG;
  x = chessboard[SIDE];
  if ( SIDE ) {
    x = shl7 ( x ) & TABCAPTUREPAWN_LEFT;
    GG = -7;
  }
  else {
    x = shr7 ( x ) & TABCAPTUREPAWN_RIGHT;
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
  x = chessboard[SIDE];
  if ( SIDE ) {
    x = shl9 ( x );
    GG = -9;
    x &= TABCAPTUREPAWN_RIGHT;
  }
  else {
    x = shr9 ( x );
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
  if ( ( ( SIDE && ENP_POSSIBILE <= 39 && ENP_POSSIBILE >= 32 )
	 || ( !SIDE && ENP_POSSIBILE >= 24 && ENP_POSSIBILE <= 31 ) )
       && ( ( x = EN_PASSANT_MASK[SIDE ^ 1][ENP_POSSIBILE] & chessboard[SIDE] ) ) ) {

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
};

int
performTowerQueenCapture ( const int tipomove, const int pezzo, const u64 enemies, const int SIDE, const u64 ALLPIECES ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  u64 xx = 0, x2;
  int o, pos;
  x2 = chessboard[pezzo];
#ifdef DEBUG_MODE
  assert ( x2 != 0 );
#endif
  while ( x2 ) {
    pos = BITScanForward ( x2 );
#ifdef DEBUG_MODE
    assert ( pos != -1 );
#endif

    if ( enemies & ORIZZONTAL[pos] ) {
      xx = MASK_CAPT_MOV2[( uchar ) ( shr ( ALLPIECES, pos_posMod8[pos] ) )]
	[pos] & enemies;
    }
    if ( enemies & VERTICAL[pos] ) {
      xx |= enemies & inv_raw90CAPT[rotate_board_90 ( ALLPIECES, pos )][pos];
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
};

int
performBishopCapture ( const int tipomove, const int pezzo, const u64 enemies, const int SIDE, const u64 ALLPIECES ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  u64 x = 0, x2;
  int o, position;
  x2 = chessboard[pezzo];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( enemies & LEFT[position] ) {
      /////left
#ifdef DEBUG_MODE
      assert ( rotate_board_left_45 ( ALLPIECES, position ) != 0 );
      if ( ( ( uchar )
	     ( rotate_board_left_45 ( ALLPIECES, position ) & MOVES_BISHOP_LEFT_MASK[position] ) ) != ( uchar ) rotate_board_left_45 ( ALLPIECES, position ) ) {

	rotate_board_left_45 ( ALLPIECES, position );
	printf ( "\n1performBishopCapture %d", position );
	assert ( 0 );
      }
      assert ( rotate_board_left_45 ( ALLPIECES, position ) != 0 );
#endif
      x = inv_raw_leftCAPT_45[rotate_board_left_45 ( ALLPIECES, position )]
	[position] & enemies;
#ifdef DEBUG_MODE
      assert ( x != -1 );
#endif
    };

    if ( enemies & RIGHT[position] ) {
      /////right
#ifdef DEBUG_MODE
      assert ( ALLPIECES != 0 );
      assert ( rotate_board_right_45 ( ALLPIECES, position ) != 0 );
      if ( ( rotate_board_right_45 ( ALLPIECES, position ) & MOVES_BISHOP_RIGHT_MASK[position] ) != rotate_board_right_45 ( ALLPIECES, position ) ) {
	rotate_board_right_45 ( ALLPIECES, position );
	printf ( "\n2performBishopCapture %d", position );
	assert ( 0 );
      }
#endif
      x |= inv_raw_rightCAPT_45[rotate_board_right_45 ( ALLPIECES, position )]
	[position] & enemies;
#ifdef DEBUG_MODE
      assert ( ( inv_raw_rightCAPT_45[rotate_board_right_45 ( ALLPIECES, position )][position] & enemies ) != -1 );
#endif
    };

    while ( x ) {
      o = BITScanForward ( x );
#ifdef DEBUG_MODE
      assert ( position != -1 );
      assert ( o != -1 );	//print();
      // assert (get_piece_at (SIDE, TABLOG[position]) != SQUARE_FREE);
      //assert (get_piece_at (XSIDE, TABLOG[position]) == SQUARE_FREE);
      assert ( chessboard[KING_BLACK] );
      assert ( chessboard[KING_WHITE] );
#endif

      if ( pushmove ( tipomove, position, o, SIDE ) )
	return 1;
      x &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[position];
  };				//while
  return 0;
};

int
performKnight_Shift_Capture ( const int tipomove, const int pezzo, const u64 pieces, const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  u64 x1, x;
  int o, pos;
  x = chessboard[pezzo];
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
};



int
performKing_Shift_Capture ( const int tipomove, const int pezzo, const u64 pieces, const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  u64 x1;
  int o, pos;
  pos = BITScanForward ( chessboard[pezzo] );
#ifdef DEBUG_MODE
  assert ( pos != -1 );
#endif
  x1 = pieces & KING_MASK[pos];
  while ( x1 ) {
    o = BITScanForward ( x1 );
    if ( pushmove ( tipomove, pos, o, SIDE ) )
      return 1;
    x1 &= NOTTABLOG[o];
  };
  return 0;
};

int
generateCap ( const int tipomove, const int SIDE ) {

#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
#ifdef DEBUG_MODE
  assert ( chessboard[KING_BLACK] );
  assert ( chessboard[KING_WHITE] );
#endif
  u64 ALLPIECES = square_all_bit_occupied (  );
  u64 enemies = square_bit_occupied ( ( SIDE ^ 1 ) );
  if ( SIDE == BLACK ) {
    if ( performPawnCapture ( tipomove, enemies, SIDE ) )
      return 1;
    if ( chessboard[KNIGHT_BLACK] ) {

      if ( performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, enemies, SIDE ) )
	return 1;
    };
    if ( chessboard[BISHOP_BLACK] ) {

      if ( performBishopCapture ( tipomove, BISHOP_BLACK, enemies, SIDE, ALLPIECES ) )
	return 1;
    };

    if ( chessboard[ROOK_BLACK] )
      if ( performTowerQueenCapture ( tipomove, ROOK_BLACK, enemies, SIDE, ALLPIECES ) )
	return 1;
    if ( chessboard[QUEEN_BLACK] ) {
      if ( performTowerQueenCapture ( tipomove, QUEEN_BLACK, enemies, SIDE, ALLPIECES ) )
	return 1;
      if ( performBishopCapture ( tipomove, QUEEN_BLACK, enemies, SIDE, ALLPIECES ) )
	return 1;
    };
    if ( performKing_Shift_Capture ( tipomove, KING_BLACK, enemies, SIDE ) )
      return 1;

  }
  else {

#ifdef DEBUG_MODE
    assert ( SIDE == WHITE );
#endif

    if ( performPawnCapture ( tipomove, enemies, SIDE ) )
      return 1;
    if ( chessboard[KNIGHT_WHITE] )
      if ( performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, enemies, SIDE ) )
	return 1;
    if ( chessboard[BISHOP_WHITE] )
      if ( performBishopCapture ( tipomove, BISHOP_WHITE, enemies, SIDE, ALLPIECES ) )
	return 1;
    if ( chessboard[ROOK_WHITE] )
      if ( performTowerQueenCapture ( tipomove, ROOK_WHITE, enemies, SIDE, ALLPIECES ) )
	return 1;
    if ( chessboard[QUEEN_WHITE] ) {

      if ( performTowerQueenCapture ( tipomove, QUEEN_WHITE, enemies, SIDE, ALLPIECES ) )
	return 1;
      if ( performBishopCapture ( tipomove, QUEEN_WHITE, enemies, SIDE, ALLPIECES ) )
	return 1;
    };
    if ( performKing_Shift_Capture ( tipomove, KING_WHITE, enemies, SIDE ) )
      return 1;
  };
  return 0;
};
