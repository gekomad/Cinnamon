#ifndef UTILITY_H
#define UTILITY_H
#include <iostream>
#include <bitset>
#include <limits>
#include <stdio.h>
#if defined  _MSC_VER
#include <intrin.h>
#endif
#include <stdlib.h>
#include <math.h>
#include "maindefine.h"
#include <string.h>
#include "bitmap.h"
#include "extern.h"
#ifdef DEBUG_MODE
#include <assert.h>
#endif
#ifndef PERFT_MODE
int check_draw ( int n_pieces_side, int n_pieces_x_side, int side );
#endif
#ifdef DEBUG_MODE
void print ( Tchessboard s );
#endif
int fen2pos ( char *fen, int *from, int *to, int, u64 key );
void myassert ( int a, const char *b );

char getFen ( const int a );
int BitCountSlow ( const u64 aBoard );
char decodeBoard ( char *a );
const char *decodeBoardinv ( const int, const int );
int fileLung ( char *a );
int loadfen ( char *ss );
int loadfen ( char *ss, int check );
int attack_square ( const int, const int );

#ifdef PERFT_MODE
int inCheck ( const int, const int, const int, const int, const int, const int, int );

#else
int compare_move ( const void *a, const void *b );
#endif
int pushmove ( const int tipomove, const int da, const int a, const int SIDE, int promotion_piece );
void print (  );
void print ( char *s );
void init (  );
long getms (  );
const static unsigned rot90[64] = { 4, 8, 4, 16, 8, 8, 64, 32, 128, 16, 16, 16, 1, 128, 8, 64, 1, 2, 32, 64, 128, 32, 32, 1, 128, 2, 8, 1, 4, 16, 64, 128, 2, 2, 4, 32, 64, 8, 128, 4, 1, 32, 64, 128, 64, 4, 2, 32, 1, 16, 4, 2, 16, 64, 2, 16, 8, 1, 32, 8, 128, 4, 2, 1 };

void BoardToFEN ( char *FEN );

FORCEINLINE int
pushmove ( const int tipomove, const int da, const int a, const int SIDE ) {
#ifdef DEBUG_MODE
  if ( tipomove == STANDARD )
    ASSERT ( da >= 0 && da < 64 && a >= 0 && a < 64 && ( SIDE == WHITE || SIDE == BLACK ) );
#endif
  return pushmove ( tipomove, da, a, SIDE, -1 );
}

#ifndef PERFT_MODE
void update_pv ( LINE * pline, const LINE * line, const Tmove * mossa, const int depth );

FORCEINLINE int
still_time (  ) {		//return 1;
  struct timeb t_current;
  ftime ( &t_current );
  return ( ( int ) ( 1000 * ( t_current.time - start_time.time ) ) ) >= MAX_TIME_MILLSEC ? 0 : 1;
}
#endif

#ifndef _MSC_VER
#ifndef HAS_64BITS
FORCEINLINE int
BITScanForward ( u64 bits ) {
  int dummy1, dummy2, dummy3;

asm ( "        bsf     %2, %0      " "\n\t" "        jnz     2f          " "\n\t" "        bsf     %1, %0      " "\n\t" "        jnz     1f          " "\n\t" "        movl    $64, %0     " "\n\t" "        jmp     2f          " "\n\t" "1:      addl    $32,%0      " "\n\t" "2:                          " "\n\t":"=&q" ( dummy1 ), "=&q" ( dummy2 ), "=&q" ( dummy3 )
:	"1" ( ( int ) ( bits >> 32 ) ), "2" ( ( int ) bits )
:	"cc" );
  return ( dummy1 );
}

#else
FORCEINLINE int
BITScanForward ( u64 bits ) {
  long dummy, dummy2;

asm ( "          bsfq    %1, %0     " "\n\t" "          jnz     1f         " "\n\t" "          movq    $64, %0    " "\n\t" "1:                           " "\n\t":"=&r" ( dummy ), "=&r" ( dummy2 )
:	"1" ( ( long ) ( bits ) )
:	"cc" );
  return ( dummy );
}

/*
 #ifndef HAS_64BITS
 FORCEINLINE int BitCount(const u64 bits) {
 register unsigned b=bits>>32;
 return _mm_popcnt_u32(bits)+_mm_popcnt_u32(b);// only intel SSE4
 }
 */
#endif
#endif
FORCEINLINE int
BitCount ( const u64 bits ) {
  return BITCOUNT[( unsigned short ) bits] + BITCOUNT[( ( unsigned ) bits ) >> 16] + BITCOUNT[( unsigned short ) ( bits >> 32 )] + BITCOUNT[bits >> 48];
}

FORCEINLINE uchar
rotate_board_90 ( u64 x ) {
  register uchar result = 0;
  int o;
  while ( x ) {
    o = BITScanForward ( x );
    result |= TABLOG_VERT90[o];
    x &= NOTTABLOG[o];
  };
  return result;
}

FORCEINLINE uchar
rotate_board_left_45 ( const u64 ss, const int pos ) {
#ifdef DEBUG_MODE
  ASSERT ( pos >= 0 && pos < 64 );
#endif
  register uchar result = TABLOG_VERT45[pos];
  u64 x = ss & LEFT[pos];
  int o;
  while ( x ) {
    o = BITScanForward ( x );
    result |= TABLOG_VERT45[o];
    x &= NOTTABLOG[o];
  };
  return result;
}

FORCEINLINE uchar
rotate_board_right_45 ( const u64 ss, const int pos ) {
#ifdef DEBUG_MODE
  ASSERT ( pos >= 0 && pos < 64 );
#endif
  register uchar result = TABLOG_VERT45[pos];
  u64 x = ss & RIGHT[pos];
  int o;
  while ( x ) {
    o = BITScanForward ( x );
    result |= TABLOG_VERT45[o];
    x &= NOTTABLOG[o];
  };
  return result;
}

#endif
