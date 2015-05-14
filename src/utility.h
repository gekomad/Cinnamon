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

#pragma warning( disable : 4146 )
#pragma warning( disable : 4514 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4706 )
#pragma warning( disable : 4996 )

#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>

#include <stdlib.h>
#include <math.h>
#include "maindefine.h"
#include <string.h>
#include "bitmap.h"
#include "extern.h"
#ifdef DEBUG_MODE
#include <assert.h>
#endif
void pop_fen (  );

int fen2pos ( char *fen, int *from, int *to, int, u64 key );
int wc ( const char * );
void controlloRipetizioni ( Tmove * myarray, int right );
void myassert ( const void *a, const char *b );
char getFenInv ( const char a );
char getFen ( const int a );
int wc ( const char *a );
int BitCountSlow ( const u64 aBoard );
char decodeBoard ( char *a );
char *decodeBoardinv ( const int, const int );
int diff_time ( struct timeb a, struct timeb b );
int fileLung ( char *a );
int loadfen ( char *ss );

int attack_square ( const int, const int );
#ifdef PERFT_MODE
int inCheck ( const int, const int, const int, const int, const int, const int, int );
#endif
int pushmove ( const int tipomove, const int da, const int a, const int SIDE );
int pushmove ( const int tipomove, const int da, const int a, const int SIDE, int promotion_piece );
void print (  );
void init (  );
void push_fen (  );
void free_fen_stack (  );
long getms (  );
const static unsigned magictable[64] = {
  0, 1, 2, 7, 3, 13, 8, 19,
  4, 25, 14, 28, 9, 34, 20, 40,
  5, 17, 26, 38, 15, 46, 29, 48,
  10, 31, 35, 54, 21, 50, 41, 57,
  63, 6, 12, 18, 24, 27, 33, 39,
  16, 37, 45, 47, 30, 53, 49, 56,
  62, 11, 23, 32, 36, 44, 52, 55,
  61, 22, 43, 51, 60, 42, 59, 58
};

const static unsigned rot90[64] = {
  4, 8, 4, 16, 8, 8, 64, 32, 128, 16, 16, 16, 1, 128, 8, 64, 1, 2, 32, 64,
  128, 32, 32, 1, 128, 2, 8, 1, 4, 16,
  64, 128, 2, 2, 4, 32, 64, 8, 128, 4, 1, 32, 64, 128, 64, 4, 2, 32, 1, 16, 4,
  2, 16, 64, 2, 16, 8, 1, 32, 8, 128, 4, 2, 1
};

// int piece_attack_defence (const int a,const int coloreAttaccato);
void BoardToFEN ( char *FEN );
#ifndef PERFT_MODE
void update_pv ( LINE * pline, const LINE * line, const Tmove * mossa, const int depth );
FORCEINLINE void
Sort ( Tmove * a, int l, int r ) {
  if ( l >= r )
    return;
  int i, j;
  int num_left, num_right;
  Tmove pivot, temp;

  while ( l < r ) {
    i = l;
    j = r + 1;
    memcpy ( &pivot, &a[l], sizeof ( Tmove ) );

    while ( true ) {
      do {
	i = i + 1;
      } while ( a[i].score > pivot.score );

      do {
	j = j - 1;
      } while ( a[j].score < pivot.score );

      if ( i >= j )
	break;

      memcpy ( &temp, &a[i], sizeof ( Tmove ) );
      memcpy ( &a[i], &a[j], sizeof ( Tmove ) );
      memcpy ( &a[j], &temp, sizeof ( Tmove ) );
    };

    memcpy ( &a[l], &a[j], sizeof ( Tmove ) );
    memcpy ( &a[j], &pivot, sizeof ( Tmove ) );

    num_left = ( j - 1 ) - l;
    num_right = r - ( j + 1 );

    if ( num_left <= num_right ) {
      Sort ( a, l, j - 1 );
      l = j + 1;
    }
    else {
      Sort ( a, j + 1, r );
      r = j - 1;
    }
  }
}

FORCEINLINE int
still_time (  ) {
  // return 1;
  struct timeb t_current;
  ftime ( &t_current );
  if ( ( ( int ) ( 1000.0 * ( t_current.time - start_time.time ) + ( t_current.millitm - start_time.millitm ) ) ) >= MAX_TIME_MILLSEC )
    return 0;
  return 1;
}
#endif

#ifndef HAS_64BITS
FORCEINLINE u64
shl7 ( const u64 ss ) {
  unsigned s1 = ( unsigned ) ss;
  u64 x = s1 << 7;
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( shift32 ( ss ) << 7 ) | ( s1 >> 25 );
  return x;
}

FORCEINLINE u64
shl9 ( const u64 ss ) {
  unsigned s1 = ( unsigned ) ss;
  u64 x = s1 << 9;
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( shift32 ( ss ) << 9 ) | ( s1 >> 23 );
  return x;
}

FORCEINLINE u64
shl8 ( const u64 ss ) {
  unsigned s1 = ( unsigned ) ss;
  u64 x = s1 << 8;
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( shift32 ( ss ) << 8 ) | ( s1 >> 24 );
  return x;
}

FORCEINLINE u64
shr8 ( const u64 ss ) {
  unsigned s2 = shift32 ( ss );
  u64 x = ( ( ( unsigned ) ss ) >> 8 ) | ( s2 << 24 );
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( s2 >> 8 );
  return x;
}

FORCEINLINE u64
shr9 ( const u64 ss ) {
  unsigned s2 = shift32 ( ss );
  u64 x = ( ( ( unsigned ) ss ) >> 9 ) | ( s2 << 23 );
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( s2 >> 9 );
  return x;
}

FORCEINLINE u64
shr7 ( const u64 bits ) {
  unsigned s2 = shift32 ( bits );
  u64 x = ( ( ( unsigned ) bits ) >> 7 ) | ( s2 << 25 );
  ( ( unsigned * ) ( &x ) + 1 )[0] = ( s2 >> 7 );
  return x;
}

FORCEINLINE u64
shr ( const u64 bits, const int N ) {
  if ( !N )
    return bits;
  if ( N < 32 ) {
    unsigned s2 = shift32 ( bits );
    u64 x = ( ( ( unsigned ) bits ) >> N ) | ( s2 << ( 32 - N ) );
    ( ( unsigned * ) ( &x ) + 1 )[0] = ( s2 >> N );
    return x;
  }
  u64 x = shift32 ( bits ) >> ( N - 32 );
  return x;
}

FORCEINLINE int
BitScanForward ( u64 bits ) {
  return magictable[( ( ( unsigned * ) &( bits = ( bits & -bits ) * 0x0218a392cd3d5dbfULL ) )[1] ) >> 26];
}

FORCEINLINE int
BitCount ( const u64 bits ) {

  unsigned lo = ( unsigned ) bits;
  unsigned hi = shift32 ( bits );
  int result = 0;
  if ( lo ) {
    result = BITCOUNT[( unsigned short ) lo] + BITCOUNT[lo >> 16];

  }
  if ( hi ) {
    result += BITCOUNT[( unsigned short ) hi] + BITCOUNT[hi >> 16];

  }

  return result;
}

#else

FORCEINLINE int
BitCount ( const u64 bits ) {
  u64 dummy, dummy2, dummy3;
asm ( "xorq %0, %0" "\n\t" " testq   %1, %1" "\n\t" " jz 2f" "\n\t" "1: leaq -1(%1),%2" "\n\t" "          incq    %0" "\n\t" "          andq    %2, %1" "\n\t" "          jnz     1b" "\n\t" "2:                      " "\n\t": "=&r" ( dummy ), "=&r" ( dummy2 ), "=&r" ( dummy3 ): "1" ( ( u64 ) ( bits ) ):"cc" );
  return ( dummy );
}

#define shl7(ss) (ss<<7)
#define shl9(ss) (ss<<9)
#define shl8(ss) (ss<<8)
#define shr8(ss) (ss>>8)
#define shr9(ss) (ss>>9)
#define shr7(ss) (ss>>7)
#define shr(ss,N) (ss>>N)



FORCEINLINE int
BitScanForward ( u64 bits ) {
  return magictable[( ( bits & -bits ) * 0x0218a392cd3d5dbf ) >> 58];
}
#endif

#ifndef PERFT_MODE
/*
FORCEINLINE int
recognizer ()
{
  u64 pn = chessboard[PAWN_BLACK];
  if (pn)
    return 999;
  u64 pb = chessboard[PAWN_WHITE];
  if (pb)
    return 999;
  u64 tn = chessboard[TOWER_BLACK];
  if (tn)
    return 999;
  u64 tb = chessboard[TOWER_WHITE];
  if (tb)
    return 999;
  u64 rn = chessboard[QUEEN_BLACK];
  if (rn)
    return 999;
  u64 rb = chessboard[QUEEN_WHITE];
  if (rb)
    return 999;

  u64 an = BitCount (chessboard[BISHOP_BLACK]);
  u64 ab = BitCount (chessboard[BISHOP_WHITE]);
  u64 cn = BitCount (chessboard[KNIGHT_BLACK]);
  u64 cb = BitCount (chessboard[KNIGHT_WHITE]);

  if (an == 0 && ab == 0 && cn == 0 && cb == 0)
    return 0;
  if (an == 0 && ab == 0 && (cb == 0 && cn == 1 || cb == 1 && cn == 0))
    return 0;
  if (cn == 0 && cb == 0 && (ab == 0 && an == 1 || ab == 1 && an == 0))
    return 0;
  if (cn == 0 && cb == 0 && (ab == 1 && an == 1))
    {
      int b = BitScanForward (chessboard[BISHOP_WHITE]);
      int n = BitScanForward (chessboard[BISHOP_BLACK]);
      if (COLORS[b] == COLORS[n])
	return 0;
    }
  return 999;
}
*/


#endif


uchar rotate_board_left_45 ( const u64, const int );
uchar rotate_board_right_45 ( const u64, const int );
uchar rotate_board_90 ( const u64, const int );
u64 rotate_board_right_45_inv_mov ( const u64 ss, const int pos );
char *lowercase ( char *a );
char *trim ( char *str );

#endif
