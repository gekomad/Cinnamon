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
#include "cap.h"
#include "bitmap.h"
#include "maindefine.h"
#include "utility.h"
#include "search.h"
#include "zobrist.h"
#include "winboard.h"
#include <stdio.h>
#include <assert.h>
#include <sys/timeb.h>
#ifdef _MSC_VER
#include "LIMITS.H"
#endif
#include "extern.h"
#include "openbook.h"

void
perform_castle ( const int da, const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  if ( SIDE == WHITE ) {
    if ( da == KINGSIDE ) {
#ifdef DEBUG_MODE
      assert ( get_piece_at ( SIDE, TABLOG_3 ) == KING_WHITE );
      assert ( get_piece_at ( SIDE, TABLOG_1 ) == SQUARE_FREE );
      assert ( get_piece_at ( SIDE, TABLOG_2 ) == SQUARE_FREE );
      assert ( get_piece_at ( SIDE, TABLOG_0 ) == ROOK_WHITE );
#endif
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_1 ) & NOTTABLOG_3;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_2 ) & NOTTABLOG_0;
    }
    else {
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_5 ) & NOTTABLOG_3;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_4 ) & NOTTABLOG_7;
    }
  }
  else {
    if ( da == KINGSIDE ) {
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
checkJumpPawn ( const int tipomove, const u64 sc, const int SIDE, const u64 XALLPIECES ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  int TT;
  register int o;
  u64 x;
  x = sc & TABSALTOPAWN;
  if ( SIDE ) {
    x = shl8 ( x );
    x &= XALLPIECES;
    x = shl8 ( x );
    x &= XALLPIECES;
    TT = -16;
  }
  else {

    x = shr8 ( x );
    x &= XALLPIECES;
    x = shr8 ( x );

    x &= XALLPIECES;
    TT = 16;
  };
  while ( x ) {
    o = BITScanForward ( x );

    pushmove ( tipomove, o + TT, o, SIDE );
    x &= NOTTABLOG[o];
  };
};


void
performTowerQueenShift ( const int tipomove, const int pezzo, const int SIDE, const u64 ALLPIECES ) {
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
    if ( ( ORIZZ_BOUND[pos] & ALLPIECES ) != ORIZZ_BOUND[pos] )
      xx = MOVIMENTO_MASK_MOV[( uchar ) ( ALLPIECES >> ( pos_posMod8[pos] ) )][pos];
    if ( ( VERT_BOUND[pos] & ALLPIECES ) != VERT_BOUND[pos] ) {
#ifdef DEBUG_MODE
      assert ( rotate_board_90 ( ALLPIECES, pos ) > 0 );
#endif
      xx |= inv_raw90MOV[rotate_board_90 ( ALLPIECES, pos )][pos];
    }
    while ( xx ) {
      o = BITScanForward ( xx );
      pushmove ( tipomove, pos, o, SIDE );
      xx &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[pos];
  };
};

void
performBishopShift ( const int tipomove, const int pezzo, const int SIDE, const u64 ALLPIECES ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  u64 x = 0, x2;
  int o, position;
  x2 = chessboard[pezzo];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( ( LEFT_BOUND[position] & ALLPIECES ) != LEFT_BOUND[position] ) {
#ifdef DEBUG_MODE
      assert ( rotate_board_left_45 ( ALLPIECES, position ) != 0 );
      if ( ( ( uchar ) ( rotate_board_left_45 ( ALLPIECES, position ) & MOVES_BISHOP_LEFT_MASK[position] ) ) != rotate_board_left_45 ( ALLPIECES, position ) ) {
	rotate_board_left_45 ( ALLPIECES, position );
	printf ( "\n 22 %d", position );
	assert ( 0 );
      }
#endif
      x = inv_raw_leftMOV_45[rotate_board_left_45 ( ALLPIECES, position )][position];

#ifdef DEBUG_MODE
      assert ( x );
#endif

    }
    if ( ( RIGHT_BOUND[position] & ALLPIECES ) != RIGHT_BOUND[position] ) {
      /////right
#ifdef DEBUG_MODE
      assert ( ALLPIECES != 0 );
      if ( ( ( uchar ) ( rotate_board_right_45 ( ALLPIECES, position ) & MOVES_BISHOP_RIGHT_MASK[position] ) ) != rotate_board_right_45 ( ALLPIECES, position ) ) {
	rotate_board_right_45 ( ALLPIECES, position );
	printf ( "\n 33 %d", position );
	assert ( 0 );
      }
#endif
      x |= inv_raw_rightMOV_45[rotate_board_right_45 ( ALLPIECES, position )][position];
#ifdef DEBUG_MODE
      assert ( inv_raw_rightMOV_45[rotate_board_right_45 ( ALLPIECES, position )][position] != -1 );
#endif
    }
    while ( x ) {
      o = BITScanForward ( x );
#ifdef DEBUG_MODE
      assert ( position != -1 );
      assert ( o != -1 );
      // assert (get_piece_at (SIDE, TABLOG[position]) != SQUARE_FREE);
      // assert (get_piece_at (XSIDE, TABLOG[position]) == SQUARE_FREE);
#endif
      pushmove ( tipomove, position, o, SIDE );
      x &= NOTTABLOG[o];
    };

    x2 &= NOTTABLOG[position];
  };
};


void
try_all_castle ( const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  //return;
  //#ifndef PERFT_MODE
  if ( SIDE == WHITE ) {
    if ( !CASTLE_DONE[WHITE] && !CASTLE_NOT_POSSIBLE[WHITE] && !CASTLE_NOT_POSSIBLE_KINGSIDE[WHITE] && get_piece_at ( 1, TABLOG_1 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_2 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_1 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_2 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_0 ) == ROOK_WHITE && get_piece_at ( 1, TABLOG_3 ) == KING_WHITE && !attack_square ( SIDE, 1 ) && !attack_square ( SIDE, 2 ) && !attack_square ( SIDE, 3 ) )
      pushmove ( CASTLE, KINGSIDE, WHITE, -1 );

    if ( !CASTLE_DONE[WHITE] && !CASTLE_NOT_POSSIBLE[WHITE] && !CASTLE_NOT_POSSIBLE_QUEENSIDE[WHITE] && get_piece_at ( 1, TABLOG_5 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_6 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_5 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_6 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_4 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_4 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_7 ) == ROOK_WHITE && get_piece_at ( 1, TABLOG_3 ) == KING_WHITE && !attack_square ( SIDE, 3 ) && !attack_square ( SIDE, 4 ) && !attack_square ( SIDE, 5 ) )
      pushmove ( CASTLE, QUEENSIDE, WHITE, -1 );
  }
  else {
    if ( !CASTLE_DONE[BLACK] && !CASTLE_NOT_POSSIBLE[BLACK] && !CASTLE_NOT_POSSIBLE_KINGSIDE[BLACK] && get_piece_at ( 0, TABLOG_57 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_58 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_57 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_58 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_56 ) == ROOK_BLACK && get_piece_at ( 0, TABLOG_59 ) == KING_BLACK && !attack_square ( SIDE, 57 ) && !attack_square ( SIDE, 58 ) && !attack_square ( SIDE, 59 ) )
      pushmove ( CASTLE, KINGSIDE, BLACK, SIDE );

    if ( !CASTLE_DONE[BLACK] && !CASTLE_NOT_POSSIBLE[BLACK] && !CASTLE_NOT_POSSIBLE_QUEENSIDE[BLACK] && get_piece_at ( 0, TABLOG_60 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_61 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_60 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_61 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_62 ) == SQUARE_FREE && get_piece_at ( 1, TABLOG_62 ) == SQUARE_FREE && get_piece_at ( 0, TABLOG_63 ) == ROOK_BLACK && get_piece_at ( 0, TABLOG_59 ) == KING_BLACK && !attack_square ( SIDE, 59 ) && !attack_square ( SIDE, 60 ) && !attack_square ( SIDE, 61 ) )
      pushmove ( CASTLE, QUEENSIDE, BLACK, SIDE );
  }
  //#endif
}


void
performPawnShift ( const int tipomove, const int SIDE, const u64 XALLPIECES ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  int o, tt;
  u64 x;
  x = chessboard[SIDE];

  if ( x & PAWNS_7_2[SIDE] )
    checkJumpPawn ( tipomove, chessboard[SIDE], SIDE, XALLPIECES );
  if ( SIDE ) {
    x = shl8 ( x );
    tt = -8;
  }
  else {
    tt = 8;
    x = shr8 ( x );
  };
  x &= XALLPIECES;
  while ( x ) {
    o = BITScanForward ( x );
#ifdef DEBUG_MODE
    assert ( get_piece_at ( SIDE, TABLOG[o + tt] ) != SQUARE_FREE );
    assert ( square_bit_occupied ( SIDE ) & TABLOG[o + tt] );
#endif
    if ( o > 55 || o < 8 ) {
      //int pezzoda=get_piece_at(o + tt);
      if ( SIDE == WHITE && o > 55 || SIDE == BLACK && o < 8 ) {

	pushmove ( PROMOTION, o + tt, o, SIDE, QUEEN_BLACK + SIDE );	//queen
#ifdef PERFT_MODE
	pushmove ( PROMOTION, o + tt, o, SIDE, KNIGHT_BLACK + SIDE );	//knight
	pushmove ( PROMOTION, o + tt, o, SIDE, BISHOP_BLACK + SIDE );	//bishop
	pushmove ( PROMOTION, o + tt, o, SIDE, ROOK_BLACK + SIDE );	//rock
#endif
      }
    }
    else
      pushmove ( tipomove, o + tt, o, SIDE );
    x &= NOTTABLOG[o];
  };
};


void
generateMoves ( const int tipomove, const int SIDE ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
#ifdef DEBUG_MODE
  assert ( chessboard[KING_BLACK] );
  assert ( chessboard[KING_WHITE] );
#endif
  u64 ALLPIECES = square_all_bit_occupied (  );

  if ( SIDE == BLACK ) {
    try_all_castle ( SIDE );
    if ( chessboard[BISHOP_BLACK] )
      performBishopShift ( tipomove, BISHOP_BLACK, SIDE, ALLPIECES );
    if ( chessboard[ROOK_BLACK] )
      performTowerQueenShift ( tipomove, ROOK_BLACK, SIDE, ALLPIECES );
    if ( chessboard[QUEEN_BLACK] ) {
      performTowerQueenShift ( tipomove, QUEEN_BLACK, SIDE, ALLPIECES );
      performBishopShift ( tipomove, QUEEN_BLACK, SIDE, ALLPIECES );
    };
    if ( chessboard[BLACK] )
      performPawnShift ( tipomove, SIDE, ~ALLPIECES );
    if ( chessboard[KNIGHT_BLACK] )
      performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, ~ALLPIECES, SIDE );
    performKing_Shift_Capture ( tipomove, KING_BLACK, ~ALLPIECES, SIDE );
  }

  else {
#ifdef DEBUG_MODE
    assert ( SIDE == WHITE );
#endif


    try_all_castle ( SIDE );
    if ( chessboard[BISHOP_WHITE] ) {
      performBishopShift ( tipomove, BISHOP_WHITE, SIDE, ALLPIECES );
    }
    if ( chessboard[ROOK_WHITE] )
      performTowerQueenShift ( tipomove, ROOK_WHITE, SIDE, ALLPIECES );
    if ( chessboard[QUEEN_WHITE] ) {
      performTowerQueenShift ( tipomove, QUEEN_WHITE, SIDE, ALLPIECES );
      performBishopShift ( tipomove, QUEEN_WHITE, SIDE, ALLPIECES );
    };

    if ( chessboard[WHITE] )
      performPawnShift ( tipomove, SIDE, ~ALLPIECES );

    if ( chessboard[KNIGHT_WHITE] )
      performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, ~ALLPIECES, SIDE );
    performKing_Shift_Capture ( tipomove, KING_WHITE, ~ALLPIECES, SIDE );
  };

};

//#endif
