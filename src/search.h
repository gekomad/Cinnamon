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

#if !defined(SEARCH_H)
#define SEARCH_H
#include "gen.h"
#include "zobrist.h"
#include "extern.h"

int ael ( const int alphabeta_side, int depth
#ifndef PERFT_MODE
	  , int alpha, int beta, LINE * pline
#endif
   );
int quiescence ( int, const int, int, const int, LINE * );

FORCEINLINE int
makemove ( Tmove * mossa ) {
#ifdef DEBUG_MODE
  assert ( mossa );

#endif
  int pezzoda, mossaa, mossada, mossacapture;
  int SIDE = mossa->side;
  int XSIDE = SIDE ^ 1;
  if ( mossa->type == STANDARD || mossa->type == ENPASSANT || mossa->type == PROMOTION ) {
    mossaa = mossa->to;
    mossada = mossa->from;
    if ( mossada == pvv_from && mossaa == pvv_to )
      path_pvv = 1;
#ifdef DEBUG_MODE
    assert ( mossaa >= 0 );
#endif
    if ( mossa->type != ENPASSANT )
      mossacapture = get_piece_at ( XSIDE, TABLOG[mossaa] );
    else
      mossacapture = XSIDE;
    mossa->capture = ( char ) mossacapture;
#ifdef DEBUG_MODE

    assert ( mossacapture != KING_BLACK );
    assert ( mossacapture != KING_WHITE );
    if ( mossada < 0 )
      mossada = mossada;
    assert ( mossada >= 0 );
#endif
    pezzoda = get_piece_at ( SIDE, TABLOG[mossada] );
    if ( mossa->type == PROMOTION ) {
      chessboard[pezzoda] = chessboard[pezzoda] & NOTTABLOG[mossada];

      chessboard[mossa->promotion_piece] = chessboard[mossa->promotion_piece] | TABLOG[mossaa];
    }
    else
      chessboard[pezzoda] = ( chessboard[pezzoda] | TABLOG[mossaa] ) & NOTTABLOG[mossada];
    if ( mossacapture != SQUARE_FREE ) {

      if ( mossa->type != ENPASSANT )
	chessboard[mossacapture] &= NOTTABLOG[mossaa];
      else {

#ifdef DEBUG_MODE
	assert ( mossacapture == ( SIDE ^ 1 ) );
#endif
	if ( SIDE )
	  chessboard[mossacapture] &= NOTTABLOG[mossaa - 8];
	else
	  chessboard[mossacapture] &= NOTTABLOG[mossaa + 8];
      }
    }
    if ( pezzoda == KING_WHITE && mossada == 3 || pezzoda == KING_BLACK && mossada == 59 ) {
      CASTLE_NOT_POSSIBLE[SIDE] = 1;
      CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] = 1;
      CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] = 1;
    }
    else if ( pezzoda == ROOK_WHITE && mossada == 0 && !CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] = 1;
    else if ( pezzoda == ROOK_WHITE && mossada == 7 && !CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] = 1;
    else if ( pezzoda == ROOK_BLACK && mossada == 63 && !CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] = 1;
    else if ( pezzoda == ROOK_BLACK && mossada == 56 && !CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] = 1;

    if ( pezzoda == PAWN_WHITE && RANK_1 & TABLOG[mossada]
	 && RANK_3 & TABLOG[mossaa] )
      ENP_POSSIBILE = mossaa;
    else if ( pezzoda == PAWN_BLACK && RANK_6 & TABLOG[mossada]
	      && RANK_4 & TABLOG[mossaa] )
      ENP_POSSIBILE = mossaa;
  }
  else if ( mossa->type == CASTLE ) {
    //chessboard[12]=makeZobristKey();
#ifdef DEBUG_MODE
    assert ( SIDE == 0 || SIDE == 1 );
    assert ( mossa->capture == SQUARE_FREE );
    assert ( mossa->side == SIDE );
    assert ( CASTLE_DONE[SIDE] == 0 );
    assert ( CASTLE_NOT_POSSIBLE[SIDE] == 0 );
    if ( mossa->from == QUEENSIDE )
      assert ( CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] == 0 );
    if ( mossa->from == KINGSIDE )
      assert ( CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] == 0 );
#endif
    perform_castle ( mossa->from, SIDE );
    CASTLE_DONE[SIDE] = 1;
    CASTLE_NOT_POSSIBLE[SIDE] = 1;
    if ( mossa->from == QUEENSIDE )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] = 1;
    if ( mossa->from == KINGSIDE )
      CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] = 1;
  }
#ifdef DEBUG_MODE
  //else {printf("\n%d %d",ENPASSANT,mossa->type);assert(0);}
#endif
#ifdef DEBUG_MODE
  assert ( chessboard[KING_BLACK] );
  assert ( chessboard[KING_WHITE] );
#endif
  //if(mply>4)print();
  return 0;

}

FORCEINLINE void
takeback ( const Tmove * mossa ) {
  ENP_POSSIBILE = -1;
  int side, pezzoda, mossaa, mossada, mossacapture;
  enpas = -1;
  side = mossa->side;
  if ( mossa->type == STANDARD || mossa->type == ENPASSANT ) {
    mossaa = mossa->to;
    mossada = mossa->from;
    if ( mossada == pvv_from && mossaa == pvv_to )
      path_pvv = 0;
    mossacapture = mossa->capture;
#ifdef DEBUG_MODE
    assert ( mossaa >= 0 );
#endif
    pezzoda = get_piece_at ( side, TABLOG[mossaa] );
    chessboard[pezzoda] = ( chessboard[pezzoda] & NOTTABLOG[mossaa] ) | TABLOG[mossada];
    if ( mossacapture != SQUARE_FREE ) {
      if ( mossa->type != ENPASSANT )
	chessboard[mossacapture] |= TABLOG[mossaa];
      else {
#ifdef DEBUG_MODE
	assert ( mossacapture == ( side ^ 1 ) );
#endif
	if ( side )
	  chessboard[mossacapture] |= TABLOG[mossaa - 8];
	else
	  chessboard[mossacapture] |= TABLOG[mossaa + 8];
      }
    }
    if ( pezzoda == KING_WHITE && mossada == 3 || pezzoda == KING_BLACK && mossada == 59 ) {
      if ( CASTLE_NOT_POSSIBLE[side] )
	CASTLE_NOT_POSSIBLE[side] = 0;
      if ( CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
	CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = 0;
      if ( CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
	CASTLE_NOT_POSSIBLE_KINGSIDE[side] = 0;
    }
    else if ( pezzoda == ROOK_WHITE && mossada == 0 && CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = 0;
    else if ( pezzoda == ROOK_WHITE && mossada == 7 && CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = 0;
    else if ( pezzoda == ROOK_BLACK && mossada == 63 && CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = 0;
    else if ( pezzoda == ROOK_BLACK && mossada == 56 && CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = 0;

  }
  else if ( mossa->type == PROMOTION ) {
    //chessboard[12]=makeZobristKey();
    mossaa = mossa->to;
    mossada = mossa->from;
    mossacapture = mossa->capture;
#ifdef DEBUG_MODE
    assert ( mossaa >= 0 );
#endif
    pezzoda = get_piece_at ( side, TABLOG[mossaa] );
    chessboard[side] = chessboard[side] | TABLOG[mossada];
    chessboard[mossa->promotion_piece] = chessboard[mossa->promotion_piece] & NOTTABLOG[mossaa];
    if ( mossacapture != SQUARE_FREE )
      chessboard[mossacapture] |= TABLOG[mossaa];

  }
  else if ( mossa->type == CASTLE ) {
    //chessboard[12]=makeZobristKey();
    un_perform_castle ( mossa->from, side );
    CASTLE_DONE[side] = 0;
    CASTLE_NOT_POSSIBLE[side] = 0;
    if ( mossa->from == QUEENSIDE )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = 0;
    if ( mossa->from == KINGSIDE )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = 0;
  }

}

#endif
