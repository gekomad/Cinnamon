/*
Copyright (C) 2008
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

#ifdef CHECKMOVE_MODE
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
#include <sys/timeb.h>

#include "LIMITS.H"
#include "extern.h"
#include "swapoff.h"
#include "openbook.h"

int
__controlla_mossa ( int side, int da, int a, u64 PAWN_BLACK1, u64 PAWN_WHITE1, u64 TOWER_BLACK1, u64 TOWER_WHITE1, u64 BISHOP_BLACK1, u64 BISHOP_WHITE1, u64 KNIGHT_BLACK1, u64 KNIGHT_WHITE1, u64 KING_BLACK1, u64 KING_WHITE1, u64 QUEEN_BLACK1, u64 QUEEN_WHITE1 ) {


//      for ( list_id = 0; list_id < 0x10000; list_id++)
  //      _LSB[list_id] = slowLSB (list_id);
  chessboard[PAWN_BLACK] = PAWN_BLACK1;
  chessboard[PAWN_WHITE] = PAWN_WHITE1;
  chessboard[TOWER_BLACK] = TOWER_BLACK1;
  chessboard[TOWER_WHITE] = TOWER_WHITE1;
  chessboard[BISHOP_BLACK] = BISHOP_BLACK1;
  chessboard[BISHOP_WHITE] = BISHOP_WHITE1;
  chessboard[KNIGHT_BLACK] = KNIGHT_BLACK1;
  chessboard[KNIGHT_WHITE] = KNIGHT_WHITE1;
  chessboard[KING_BLACK] = KING_BLACK1;
  chessboard[KING_WHITE] = KING_WHITE1;
  chessboard[QUEEN_BLACK] = QUEEN_BLACK1;
  chessboard[QUEEN_WHITE] = QUEEN_WHITE1;
  list_id = 0;
  Tmove *mossa;
  //printf("\nda: %d a: %d",da,a);
  generateCap ( STANDARD, side );
  generateMoves ( STANDARD, side );
  int listcount = moveListCount[list_id];
  for ( int i = 1; i <= listcount; i++ ) {
    mossa = &gen_list[list_id][i];
    if ( mossa->da < 0 ) {
      if ( mossa->side == WHITE ) {
	if ( mossa->da == KINGSIDE ) {
	  mossa->da = 3;
	  mossa->a = 1;
	}
	else {
	  mossa->da = 3;
	  mossa->a = 5;
	}
      }
      else {
	if ( mossa->da == KINGSIDE ) {
	  mossa->da = 59;
	  mossa->a = 57;
	}
	else {
	  mossa->da = 59;
	  mossa->a = 61;
	}
      };
    }
    //printf("\n%d %d",mossa->da,mossa->a);
    if ( da == mossa->da && a == mossa->a ) {
      moveListCount[list_id] = 0;
      return 1;
    }

  }
  moveListCount[list_id] = 0;
  return 0;
}

int
__is_mate ( int side, u64 PAWN_BLACK1, u64 PAWN_WHITE1, u64 TOWER_BLACK1, u64 TOWER_WHITE1, u64 BISHOP_BLACK1, u64 BISHOP_WHITE1, u64 KNIGHT_BLACK1, u64 KNIGHT_WHITE1, u64 KING_BLACK1, u64 KING_WHITE1, u64 QUEEN_BLACK1, u64 QUEEN_WHITE1 ) {


//      for ( list_id = 0; list_id < 0x10000; list_id++)
//              _LSB[list_id] = slowLSB (list_id);
  chessboard[PAWN_BLACK] = PAWN_BLACK1;
  chessboard[PAWN_WHITE] = PAWN_WHITE1;
  chessboard[TOWER_BLACK] = TOWER_BLACK1;
  chessboard[TOWER_WHITE] = TOWER_WHITE1;
  chessboard[BISHOP_BLACK] = BISHOP_BLACK1;
  chessboard[BISHOP_WHITE] = BISHOP_WHITE1;
  chessboard[KNIGHT_BLACK] = KNIGHT_BLACK1;
  chessboard[KNIGHT_WHITE] = KNIGHT_WHITE1;
  chessboard[KING_BLACK] = KING_BLACK1;
  chessboard[KING_WHITE] = KING_WHITE1;
  chessboard[QUEEN_BLACK] = QUEEN_BLACK1;
  chessboard[QUEEN_WHITE] = QUEEN_WHITE1;
  list_id = 0;
  moveListCount[list_id] = 0;
  generateCap ( STANDARD, side );
  generateMoves ( STANDARD, side );
  if ( !moveListCount[list_id] )
    return 1;

  return 0;
}
#endif
