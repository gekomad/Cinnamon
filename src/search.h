#if !defined(SEARCH_H)
#define SEARCH_H
#include "gen.h"
#include "zobrist.h"
#include "extern.h"
#include "debug.h"

int ael ( const int alphabeta_side, int depth
#ifndef PERFT_MODE
	  , int alpha, int beta, LINE * pline, u64, int
#endif
   );

FORCEINLINE void
makemove ( Tmove * mossa, u64 * key ) {
  ASSERT ( mossa );
  int pezzoda, mossaa, mossada, mossacapture;
  int SIDE = mossa->side;
  int XSIDE = change_side ( SIDE );
  if ( mossa->type == STANDARD || mossa->type == ENPASSANT || mossa->type == PROMOTION ) {
    mossaa = mossa->to;
    mossada = mossa->from;
    ASSERT ( mossaa >= 0 );
    if ( mossa->type != ENPASSANT )
      mossacapture = get_piece_at ( XSIDE, tablog ( mossaa ) );
    else
      mossacapture = XSIDE;
    mossa->capture = ( char ) mossacapture;
    ASSERT ( mossacapture != KING_BLACK );
    ASSERT ( mossacapture != KING_WHITE );
    ASSERT ( mossada >= 0 && mossada < 64 );
    ASSERT ( mossaa >= 0 && mossaa < 64 );
    pezzoda = get_piece_at ( SIDE, tablog ( mossada ) );
    if ( mossa->type == PROMOTION ) {
      chessboard[pezzoda] = Chessboard ( pezzoda ) & NOTTABLOG[mossada];
      chessboard[mossa->promotion_piece] |= tablog ( mossaa );
    }
    else {
#ifdef DEBUG_MODE
      if ( pezzoda == 12 ) {
	printf ( "\nERROR  mossa->from%d mossa->capture%d mossa->promotion_piece%d mossa->score&d mossa->side%d mossa->to%d mossa->type%d", mossa->from, mossa->capture, mossa->promotion_piece, mossa->score, mossa->side, mossa->to, mossa->type );
	fflush ( stdout );
	assert ( 0 );
      }

#endif
      chessboard[pezzoda] = ( Chessboard ( pezzoda ) | tablog ( mossaa ) ) & NOTTABLOG[mossada];
    }

    if ( mossacapture != SQUARE_FREE ) {
      if ( mossa->type != ENPASSANT )
	chessboard[mossacapture] &= NOTTABLOG[mossaa];
      else {
	ASSERT ( mossacapture == ( change_side ( SIDE ) ) );
	if ( SIDE )
	  chessboard[mossacapture] &= NOTTABLOG[mossaa - 8];
	else
	  chessboard[mossacapture] &= NOTTABLOG[mossaa + 8];
      }
    }
    if ( ( ( pezzoda == KING_WHITE ) && ( mossada == 3 ) ) || ( ( pezzoda == KING_BLACK ) && ( mossada == 59 ) ) ) {
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
    if ( pezzoda == PAWN_WHITE && RANK_1 & tablog ( mossada ) && RANK_3 & tablog ( mossaa ) )
      ENP_POSSIBILE = mossaa;

    else if ( pezzoda == PAWN_BLACK && RANK_6 & tablog ( mossada ) && RANK_4 & tablog ( mossaa ) )
      ENP_POSSIBILE = mossaa;
  }
  else if ( mossa->type == CASTLE ) {
#ifdef DEBUG_MODE
    ASSERT ( SIDE == 0 || SIDE == 1 );
    ASSERT ( mossa->side == SIDE );
    ASSERT ( CASTLE_DONE[SIDE] == 0 );
    ASSERT ( CASTLE_NOT_POSSIBLE[SIDE] == 0 );
    if ( mossa->from == QUEENSIDE )
      ASSERT ( CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] == 0 );
    if ( mossa->from == KINGSIDE )
      ASSERT ( CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] == 0 );
#endif
    perform_castle ( mossa->from, SIDE );
    CASTLE_DONE[SIDE] = 1;
    CASTLE_NOT_POSSIBLE[SIDE] = 1;
    if ( mossa->from == QUEENSIDE )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[SIDE] = 1;
    if ( mossa->from == KINGSIDE )
      CASTLE_NOT_POSSIBLE_KINGSIDE[SIDE] = 1;
  }
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
#ifndef PERFT_MODE
  //*key = makeZobristKey();
  //ASSERT(stack_move1.next>=0 && stack_move1.next<MAX_PLY );
  //stack_move1.board[stack_move1.next++] = *key;
#endif
}

FORCEINLINE void
takeback ( const Tmove * mossa ) {
#ifndef PERFT_MODE
  //stack_move1.next--;
  //ASSERT (stack_move1.next >= 0);
#endif
  ENP_POSSIBILE = -1;
  int side, pezzoda, mossaa, mossada, mossacapture;
  enpas = -1;
  side = mossa->side;
  if ( mossa->type == STANDARD || mossa->type == ENPASSANT ) {
    mossaa = mossa->to;
    mossada = mossa->from;
    mossacapture = mossa->capture;
    ASSERT ( mossada >= 0 && mossada < 64 );
    ASSERT ( mossaa >= 0 && mossaa < 64 );
    pezzoda = get_piece_at ( side, TABLOG[mossaa] );
    chessboard[pezzoda] = ( chessboard[pezzoda] & NOTTABLOG[mossaa] ) | tablog ( mossada );
    if ( mossacapture != SQUARE_FREE ) {
      if ( mossa->type != ENPASSANT )
	chessboard[mossacapture] |= tablog ( mossaa );
      else {
	ASSERT ( mossacapture == ( change_side ( side ) ) );
	if ( side )
	  chessboard[mossacapture] |= tablog ( mossaa - 8 );
	else
	  chessboard[mossacapture] |= tablog ( mossaa + 8 );
      }
    }
    if ( ( ( pezzoda == KING_WHITE ) && ( mossada == 3 ) ) || ( ( pezzoda == KING_BLACK ) && ( mossada == 59 ) ) ) {
      if ( CASTLE_NOT_POSSIBLE[side] )
	CASTLE_NOT_POSSIBLE[side] = START_CASTLE_NOT_POSSIBLE[side];
      if ( CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
	CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = START_CASTLE_NOT_POSSIBLE_QUEENSIDE[side];
      if ( CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
	CASTLE_NOT_POSSIBLE_KINGSIDE[side] = START_CASTLE_NOT_POSSIBLE_KINGSIDE[side];
    }
    else if ( pezzoda == ROOK_WHITE && mossada == 0 && CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = START_CASTLE_NOT_POSSIBLE_KINGSIDE[side];
    else if ( pezzoda == ROOK_WHITE && mossada == 7 && CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = START_CASTLE_NOT_POSSIBLE_QUEENSIDE[side];
    else if ( pezzoda == ROOK_BLACK && mossada == 63 && CASTLE_NOT_POSSIBLE_QUEENSIDE[side] )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = START_CASTLE_NOT_POSSIBLE_QUEENSIDE[side];
    else if ( pezzoda == ROOK_BLACK && mossada == 56 && CASTLE_NOT_POSSIBLE_KINGSIDE[side] )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = START_CASTLE_NOT_POSSIBLE_KINGSIDE[side];
  }
  else if ( mossa->type == PROMOTION ) {
    mossaa = mossa->to;
    mossada = mossa->from;
    mossacapture = mossa->capture;
    ASSERT ( mossaa >= 0 );
    pezzoda = get_piece_at ( side, tablog ( mossaa ) );
    chessboard[side] = Chessboard ( side ) | tablog ( mossada );
    chessboard[mossa->promotion_piece] = chessboard[mossa->promotion_piece] & NOTTABLOG[mossaa];
    if ( mossacapture != SQUARE_FREE )
      chessboard[mossacapture] |= tablog ( mossaa );
  }
  else if ( mossa->type == CASTLE ) {
    un_perform_castle ( mossa->from, side );
    CASTLE_DONE[side] = START_CASTLE_DONE[side];
    CASTLE_NOT_POSSIBLE[side] = START_CASTLE_NOT_POSSIBLE[side];
    if ( mossa->from == QUEENSIDE )
      CASTLE_NOT_POSSIBLE_QUEENSIDE[side] = START_CASTLE_NOT_POSSIBLE_QUEENSIDE[side];
    if ( mossa->from == KINGSIDE )
      CASTLE_NOT_POSSIBLE_KINGSIDE[side] = START_CASTLE_NOT_POSSIBLE_KINGSIDE[side];
  }
}

#endif
