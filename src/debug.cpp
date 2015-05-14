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
#ifdef DEBUG_MODE
#include "stdafx.h"
#include "bitmap.h"
#include "maindefine.h"
#include "utility.h"
#include <stdio.h>

u64
Chessboard ( int x ) {
  assert ( x >= 0 && x < 12 );
  return chessboard[x];
}

u64
tablog ( int x ) {
  assert ( x >= 0 && x < 64 );
  return TABLOG[x];

}

int
change_side ( int side ) {

  assert ( side == WHITE || side == BLACK );
  return side ^ 1;
}

uchar
get_piece_at ( int side, u64 tablogpos ) {

  assert ( side == WHITE || side == BLACK );
  return ( ( side == WHITE ) ? ( ( chessboard[PAWN_WHITE] & tablogpos ) ? PAWN_WHITE : ( ( chessboard[KING_WHITE] & tablogpos ) ? KING_WHITE : ( ( chessboard[ROOK_WHITE] & tablogpos ) ? ROOK_WHITE : ( ( chessboard[BISHOP_WHITE] & tablogpos ) ? BISHOP_WHITE : ( ( chessboard[KNIGHT_WHITE] & tablogpos ) ? KNIGHT_WHITE : ( ( chessboard[QUEEN_WHITE] & tablogpos ) ? QUEEN_WHITE : SQUARE_FREE ) ) ) ) ) ) : ( ( chessboard[PAWN_BLACK] & tablogpos ) ? PAWN_BLACK : ( ( chessboard[KING_BLACK] & tablogpos ) ? KING_BLACK : ( ( chessboard[ROOK_BLACK] & tablogpos ) ? ROOK_BLACK : ( ( chessboard[BISHOP_BLACK] & tablogpos ) ? BISHOP_BLACK : ( ( chessboard[KNIGHT_BLACK] & tablogpos ) ? KNIGHT_BLACK : ( ( chessboard[QUEEN_BLACK] & tablogpos ) ? QUEEN_BLACK : SQUARE_FREE ) ) ) ) ) ) );
}
#endif
