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

#if !defined(SWAPOFF_H)
#define SWAPOFF_H
#include "extern.h"
#include "maindefine.h"
//#ifndef PERFT_MODE

/*
__inline void
sort ( int *a, int l, int r ) {
  if ( l >= r )
    return;
  int i, j;
  int num_left, num_right;
  int pivot, temp;
  while ( l < r ) {
    i = l;
    j = r + 1;
    pivot = a[l];
    while ( true ) {
      while ( ++i<l && a[i] > pivot );
      while ( --j > l && a[j] < pivot );
      if ( i >= j )
	break;
      temp = a[i];
      a[i] = a[j];
      a[j] = temp;
    };
    a[l] = a[j];
    a[j] = pivot;
    num_left = ( j - 1 ) - l;
    num_right = r - ( j + 1 );
    if ( num_left <= num_right ) {
      sort ( a, l, j - 1 );
      l = j + 1;
    }
    else {
      sort ( a, j + 1, r );
      r = j - 1;
    }
  }
}
*/
__inline u64
calcola_attaccanti ( const int pos, const int colore_attaccato ) {
  int o, y, xcolore_attaccato, re_position_mod_8, bit_raw;
  u64 allpieces2, x;
  u64 case_attaccanti = 0;
  xcolore_attaccato = colore_attaccato ^ 1;
  if ( colore_attaccato == BLACK ) {
    x = ( PAWN_CAPTURE_MASK[BLACK][pos] & chessboard[PAWN_WHITE] );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  }
  else {
    x = ( PAWN_CAPTURE_MASK[WHITE][pos] & chessboard[PAWN_BLACK] );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  };
  x = ( KNIGHT_MASK[pos] & chessboard[KNIGHT_BLACK + xcolore_attaccato] );
  while ( x ) {

    o = BitScanForward ( x );
    case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
    assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
    x &= NOTTABLOG[o];
  };
  allpieces2 = case_all_bit_occupate (  );

  re_position_mod_8 = ROT45[pos];

  bit_raw = ( uchar ) ( ( allpieces2 >> ( pos_posMod8[pos] ) ) );
  y = MOVIMENTO_MASK_CAT[bit_raw][re_position_mod_8];
  if ( y ) {
    x = y & ( chessboard[TOWER_BLACK + ( xcolore_attaccato )] >> ( pos_posMod8[pos] ) & 255 );
    if ( x )
      x = ( x << ( ( get_row[pos] ) << 3 ) );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
    x = y & ( chessboard[QUEEN_BLACK + ( xcolore_attaccato )] >> ( pos_posMod8[pos] ) & 255 );
    if ( x )
      x = ( x << ( ( get_row[pos] ) << 3 ) );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  };
  //left  

  bit_raw = rotate_board_left_45 ( case_all_bit_occupate (  ), pos );
#ifdef DEBUG_MODE
  assert ( bit_raw != -1 );
  assert ( bit_raw == ( bit_raw & MOVES_BISHOP_LEFT_MASK[pos] ) );
#endif

  y = MOVIMENTO_MASK_CAT[bit_raw][re_position_mod_8];
  if ( y ) {

    x = y & ( rotate_board_left_45 ( chessboard[BISHOP_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x != -1 )
      x = inv_raw_left_45[x][pos];
    while ( x > 0 ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
    x = y & ( rotate_board_left_45 ( chessboard[QUEEN_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x != -1 )
      x = inv_raw_left_45[x][pos];
    while ( x > 0 ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  };
  /*right \ */
  bit_raw = rotate_board_right_45 ( case_all_bit_occupate (  ), pos );

#ifdef DEBUG_MODE
  assert ( bit_raw != -1 );
  assert ( bit_raw == ( bit_raw & MOVES_BISHOP_RIGHT_MASK[pos] ) );
#endif
  bit_raw |= TABLOG[re_position_mod_8];
  y = MOVIMENTO_MASK_CAT[bit_raw][re_position_mod_8];
  if ( y ) {

    x = y & ( rotate_board_right_45 ( chessboard[BISHOP_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x != -1 )
      x = inv_raw_right_45[x][pos];
    while ( x > 0 ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
    x = y & ( rotate_board_right_45 ( chessboard[QUEEN_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x != -1 )
      x = inv_raw_right_45[x][pos];
    while ( x > 0 ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  };
  // } right*/ 

  bit_raw = rotate_board_90 ( case_all_bit_occupate (  ), pos );
  y = MOVIMENTO_MASK_CAT[bit_raw][ROT45ROT_90_MASK[pos]];
  if ( y ) {

    x = y & ( rotate_board_90 ( chessboard[TOWER_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x > 0 )
      x = ( inv_raw90[x][ROT45[pos]] );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
    x = y & ( rotate_board_90 ( chessboard[QUEEN_BLACK + ( xcolore_attaccato )], pos ) );
    if ( x > 0 )
      x = ( inv_raw90[x][ROT45[pos]] );
    while ( x ) {

      o = BitScanForward ( x );
      case_attaccanti |= TABLOG[o];
#ifdef DEBUG_MODE
      assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
      x &= NOTTABLOG[o];
    };
  };
  if ( KING_MASK[pos] & chessboard[KING_BLACK + xcolore_attaccato] ) {

    case_attaccanti |= chessboard[KING_BLACK + xcolore_attaccato];
#ifdef DEBUG_MODE
    assert ( !( case_attaccanti & TABLOG[pos] ) );
#endif
  }
#ifdef DEBUG_MODE
  if ( ( case_attaccanti & TABLOG[pos] ) ) {
    print (  );
    printf ( "\n%d", pos );
#ifdef DEBUG_MODE
    assert ( 0 );
#endif
  }
#endif
  return case_attaccanti;
}

int compare_int ( const void *a, const void *b );

__inline int
see ( const int a, const int coloreAttaccato ) {
  //restituisce +-= se � conveniente fare la mossa 
  int c, rr;
  int d0, primo = 1;
  const int lung = 1000;
  Tchessboard chessboard2;
  d0 = PIECES_VALUE[get_piece_at ( coloreAttaccato, TABLOG[a] )];
  memcpy ( chessboard2, chessboard, sizeof ( Tchessboard ) );
  int xside;
  u64 attaccanti;
  u64 difensori;
  xside = coloreAttaccato ^ 1;
  attaccanti = 0;
  difensori = 0;
  int A[lung];
  int D[lung];
  int count_Aside = 0;
  int count_side = 0;
  int t;
  int da_xside = 0;
  int da_side = 0;

  do {
    attaccanti = calcola_attaccanti ( a, coloreAttaccato );
    if ( !attaccanti && primo == 1 )
      return 0;
    primo++;
    c = 0;
    if ( attaccanti )
      da_xside = count_Aside;
    while ( attaccanti ) {
      c = 1;
      t = BitScanForward ( attaccanti );
      rr = get_piece_at ( xside, TABLOG[t] );
#ifdef DEBUG_MODE
      assert ( rr != SQUARE_FREE );
      assert ( count_Aside < lung );
#endif
      A[count_Aside++] = 0x3FF & PIECES_VALUE[rr];
      chessboard[rr] &= NOTTABLOG[t];
      attaccanti &= NOTTABLOG[t];
    }
  }
  while ( c );
  //count_Aside--;

  if ( count_Aside - da_xside > 1 )
    qsort ( A + da_xside, count_Aside - da_xside, sizeof ( int ), compare_int );

  //sort ( A, da_xside, count_Aside - 1 );
  memcpy ( chessboard, chessboard2, sizeof ( Tchessboard ) );
  do {
    difensori = calcola_attaccanti ( a, xside );
    c = 0;
    if ( difensori )
      da_side = count_side;
    while ( difensori ) {
      c = 1;
      t = BitScanForward ( difensori );
      rr = get_piece_at ( coloreAttaccato, TABLOG[t] );
#ifdef DEBUG_MODE
      assert ( rr != SQUARE_FREE );
      assert ( count_side < lung );
#endif

      D[count_side++] = 0x3FF & PIECES_VALUE[rr];
      chessboard[rr] &= NOTTABLOG[t];
      difensori &= NOTTABLOG[t];
    }
  }
  while ( c && count_side <= count_Aside );
  count_side--;

  if ( count_side - da_side > 1 )
    qsort ( D + da_side, count_side - da_side, sizeof ( int ), compare_int );
  //sort ( D, da_side, count_side - 1 );
  int AA = 0;
  int DD = 0;
  if ( count_Aside > count_side ) {
    for ( c = 0; c < count_side; c++ )
      DD += D[c];
    for ( c = 0; c < count_side; c++ )
      AA += A[c];
  }
  else {
    for ( c = 0; c < count_Aside - 1; c++ )
      DD += D[c];
    for ( c = 0; c < count_Aside; c++ )
      AA += A[c];
  }
  memcpy ( chessboard, chessboard2, sizeof ( Tchessboard ) );
  return DD + d0 - AA;

}


#endif
