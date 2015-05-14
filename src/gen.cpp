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
  check_side ( SIDE );
  if ( SIDE == WHITE ) {
    if ( da == KINGSIDE ) {
      ASSERT ( get_piece_at ( SIDE, TABLOG_3 ) == KING_WHITE );
      ASSERT ( get_piece_at ( SIDE, TABLOG_1 ) == SQUARE_FREE );
      ASSERT ( get_piece_at ( SIDE, TABLOG_2 ) == SQUARE_FREE );
      ASSERT ( get_piece_at ( SIDE, TABLOG_0 ) == ROOK_WHITE );

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
  check_side ( SIDE );
  int TT;
  register int o;
  u64 x;
  x = sc & TABJUMPPAWN;
  if ( SIDE ) {
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
    pushmove ( tipomove, o + TT, o, SIDE );
    x &= NOTTABLOG[o];
  };
}

void
performTowerQueenShift ( const int tipomove, const int pezzo, const int SIDE, const u64 ALLPIECES ) {
  check_side ( SIDE );
  ASSERT ( pezzo >= 0 && pezzo < 12 );
  u64 xx = 0, x2;
  int o, pos;
  x2 = chessboard[pezzo];
  while ( x2 ) {
    pos = BITScanForward ( x2 );
    if ( ( ORIZZ_BOUND[pos] & ALLPIECES ) != ORIZZ_BOUND[pos] )
      xx = MASK_MOV[( uchar ) ( ALLPIECES >> pos_posMod8[pos] )][pos];
    if ( ( VERT_BOUND[pos] & ALLPIECES ) != VERT_BOUND[pos] ) {
      ASSERT ( rotate_board_90 ( ALLPIECES & VERTICAL[pos] ) > 0 );
      xx |= inv_raw90MOV[rotate_board_90 ( ALLPIECES & VERTICAL[pos] )][pos];
    }
    while ( xx ) {
      o = BITScanForward ( xx );
      pushmove ( tipomove, pos, o, SIDE );
      xx &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[pos];
  };
}

void
performBishopShift ( const int tipomove, const int pezzo, const int SIDE, const u64 ALLPIECES ) {
  check_side ( SIDE );
  ASSERT ( pezzo >= 0 && pezzo < 12 );
  u64 x = 0, x2;
  int o, position;
  x2 = chessboard[pezzo];
  while ( x2 ) {
    position = BITScanForward ( x2 );
    if ( ( LEFT_BOUND[position] & ALLPIECES ) != LEFT_BOUND[position] ) {
      x = inv_raw_leftMOV_45[rotate_board_left_45 ( ALLPIECES, position )][position];
      ASSERT ( x );

    }
    if ( ( RIGHT_BOUND[position] & ALLPIECES ) != RIGHT_BOUND[position] ) {
      /////right
      x |= inv_raw_rightMOV_45[rotate_board_right_45 ( ALLPIECES, position )][position];
    }
    while ( x ) {
      o = BITScanForward ( x );
      ASSERT ( position != -1 );
      ASSERT ( o != -1 );
      pushmove ( tipomove, position, o, SIDE );
      x &= NOTTABLOG[o];
    };
    x2 &= NOTTABLOG[position];
  };
}

void
try_all_castle ( const int SIDE, const u64 ALLPIECES ) {
  check_side ( SIDE );

  if ( SIDE == WHITE ) {
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x6 ) && !CASTLE_DONE[WHITE] && !CASTLE_NOT_POSSIBLE[WHITE] && !CASTLE_NOT_POSSIBLE_KINGSIDE[WHITE] && chessboard[ROOK_WHITE] & TABLOG_0 && !attack_square ( SIDE, 1 ) && !attack_square ( SIDE, 2 ) && !attack_square ( SIDE, 3 ) )
      pushmove ( CASTLE, KINGSIDE, WHITE, -1 );
    if ( TABLOG_3 & chessboard[KING_WHITE] && !( ALLPIECES & 0x70 ) && !CASTLE_DONE[WHITE] && !CASTLE_NOT_POSSIBLE[WHITE] && !CASTLE_NOT_POSSIBLE_QUEENSIDE[WHITE] && chessboard[ROOK_WHITE] & TABLOG_7 && !attack_square ( SIDE, 3 ) && !attack_square ( SIDE, 4 ) && !attack_square ( SIDE, 5 ) )
      pushmove ( CASTLE, QUEENSIDE, WHITE, -1 );
  }
  else {
    if ( TABLOG_59 & chessboard[KING_BLACK] && !CASTLE_DONE[BLACK] && !CASTLE_NOT_POSSIBLE[BLACK] && !CASTLE_NOT_POSSIBLE_KINGSIDE[BLACK] && !( ALLPIECES & 0x600000000000000 ) && chessboard[ROOK_BLACK] & TABLOG_56 && !attack_square ( SIDE, 57 ) && !attack_square ( SIDE, 58 ) && !attack_square ( SIDE, 59 ) )
      pushmove ( CASTLE, KINGSIDE, BLACK, -1 );
    if ( TABLOG_59 & chessboard[KING_BLACK] && !CASTLE_DONE[BLACK] && !CASTLE_NOT_POSSIBLE[BLACK] && !CASTLE_NOT_POSSIBLE_QUEENSIDE[BLACK] && !( ALLPIECES & 0x7000000000000000 ) && chessboard[ROOK_BLACK] & TABLOG_63 && !attack_square ( SIDE, 59 ) && !attack_square ( SIDE, 60 ) && !attack_square ( SIDE, 61 ) )
      pushmove ( CASTLE, QUEENSIDE, BLACK, -1 );
  }
}

void
performPawnShift ( const int tipomove, const int SIDE, const u64 XALLPIECES ) {
  check_side ( SIDE );
  int o, tt;
  u64 x;
  x = chessboard[SIDE];
  if ( x & PAWNS_7_2[SIDE] )
    checkJumpPawn ( tipomove, chessboard[SIDE], SIDE, XALLPIECES );
  if ( SIDE ) {
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
    ASSERT ( get_piece_at ( SIDE, TABLOG[o + tt] ) != SQUARE_FREE );
    ASSERT ( square_bit_occupied ( SIDE ) & TABLOG[o + tt] );
    if ( o > 55 || o < 8 ) {
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
}

void
generateMoves ( const int tipomove, const int SIDE ) {
  check_side ( SIDE );
  if ( !running )
    return;
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  u64 ALLPIECES = square_all_bit_occupied (  );
  if ( SIDE == BLACK ) {
    try_all_castle ( BLACK, ALLPIECES );
    performBishopShift ( tipomove, BISHOP_BLACK, SIDE, ALLPIECES );
    performTowerQueenShift ( tipomove, ROOK_BLACK, SIDE, ALLPIECES );
    performTowerQueenShift ( tipomove, QUEEN_BLACK, SIDE, ALLPIECES );
    performBishopShift ( tipomove, QUEEN_BLACK, SIDE, ALLPIECES );
    performPawnShift ( tipomove, SIDE, ~ALLPIECES );
    performKnight_Shift_Capture ( tipomove, KNIGHT_BLACK, ~ALLPIECES, SIDE );
    performKing_Shift_Capture ( tipomove, KING_BLACK, ~ALLPIECES, SIDE );
  }
  else {
    ASSERT ( SIDE == WHITE );
    try_all_castle ( WHITE, ALLPIECES );
    performBishopShift ( tipomove, BISHOP_WHITE, SIDE, ALLPIECES );
    performTowerQueenShift ( tipomove, ROOK_WHITE, SIDE, ALLPIECES );
    performTowerQueenShift ( tipomove, QUEEN_WHITE, SIDE, ALLPIECES );
    performBishopShift ( tipomove, QUEEN_WHITE, SIDE, ALLPIECES );
    performPawnShift ( tipomove, SIDE, ~ALLPIECES );
    performKnight_Shift_Capture ( tipomove, KNIGHT_WHITE, ~ALLPIECES, SIDE );
    performKing_Shift_Capture ( tipomove, KING_WHITE, ~ALLPIECES, SIDE );
  };
}
