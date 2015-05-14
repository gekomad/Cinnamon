#ifdef DEBUG_MODE
#include "stdafx.h"
#include "bitmap.h"
#include "maindefine.h"
#include "utility.h"
#include <stdio.h>

Tmove *
get_gen_list ( int list_id, int i ) {
  assert ( list_id >= 0 );
  assert ( list_id < MAX_PLY );
  assert ( i >= 0 );
  assert ( list_id < MAX_MOVE );
  return &gen_list[list_id][i];
}

int
get_gen_list_count ( int list_id ) {
  assert ( list_id >= 0 );
  assert ( list_id < MAX_PLY );
  return gen_list[list_id][0].score;
}

u64
Chessboard ( int x ) {
  if ( x < 0 || x >= 12 ) {
    ASSERT ( 0 );
  }
  ASSERT ( x >= 0 && x < 12 );
  return chessboard[x];
}

u64
tablog ( int x ) {
  ASSERT ( x >= 0 && x < 64 );
  return TABLOG[x];

}

int
change_side ( int side ) {
  ASSERT ( side == WHITE || side == BLACK );
  return side ^ 1;
}

uchar
get_piece_at ( int side, u64 tablogpos ) {
  ASSERT ( side == WHITE || side == BLACK );
  return ( ( side == WHITE ) ? ( ( chessboard[PAWN_WHITE] & tablogpos ) ? PAWN_WHITE : ( ( chessboard[KING_WHITE] & tablogpos ) ? KING_WHITE : ( ( chessboard[ROOK_WHITE] & tablogpos ) ? ROOK_WHITE : ( ( chessboard[BISHOP_WHITE] & tablogpos ) ? BISHOP_WHITE : ( ( chessboard[KNIGHT_WHITE] & tablogpos ) ? KNIGHT_WHITE : ( ( chessboard[QUEEN_WHITE]
																																								   & tablogpos ) ? QUEEN_WHITE : SQUARE_FREE ) ) ) ) ) )
	   : ( ( chessboard[PAWN_BLACK] & tablogpos ) ? PAWN_BLACK : ( ( chessboard[KING_BLACK] & tablogpos ) ? KING_BLACK : ( ( chessboard[ROOK_BLACK] & tablogpos ) ? ROOK_BLACK : ( ( chessboard[BISHOP_BLACK] & tablogpos ) ? BISHOP_BLACK : ( ( chessboard[KNIGHT_BLACK]
																														     & tablogpos ) ? KNIGHT_BLACK : ( ( chessboard[QUEEN_BLACK]
																																			& tablogpos ) ? QUEEN_BLACK : SQUARE_FREE ) ) ) ) ) ) );

}

#endif
