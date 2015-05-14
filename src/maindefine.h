#ifndef MAINDEFINE_H_
#define MAINDEFINE_H_

#include <sys/stat.h>
#include <sys/timeb.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stddef.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#ifdef DEBUG_MODE
#include <assert.h>
#endif

/*
 8| 63 62 61 60 59 58 57 56
 7| 55 54 53 52 51 50 49 48
 6| 47 46 45 44 43 42 41 40
 5| 39 38 37 36 35 34 33 32
 4| 31 30 29 28 27 26 25 24
 3| 23 22 21 20 19 18 17 16
 2| 15 14 13 12 11 10 09 08
 1| 07 06 05 04 03 02 01 00
 ...a  b  c  d  e  f  g  h


 Depth 	Perft
 1 		20 				verified
 2  	400 			verified
 3 		8902 			verified
 4 		197281 			verified
 5 		4865609 		verified
 6 		119060324 		verified
 7 		3195901860 		verified
 8 		84998978956 	verified
 9		2439530234167	verified

r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -
 Depth 	Perft
 1		48              verified
 2		2039            verified
 3		97862           verified
 4		4085603         verified
 5		193690690       verified
 6		8031647685      verified
 7      374190009323    verified

rnbqkbnr/pp1ppppp/8/2pP4/8/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 2
Depth 	Perft
 1      30              verified
 2      631             verified
 3      18825           verified
 4      437149          verified
 5      13787913        verified

 */

using namespace std;

#define NAME "Butterfly 0.6"
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

typedef unsigned char uchar;
#ifndef NO_FP_MODE
#define FUTIL_MARGIN 198
#define EXT_FUTILY_MARGIN 515
#define RAZOR_MARGIN 1071
#endif
#define MAX_MOVE 130
#define MAX_PLY 64
#define _INFINITE 32000
#define BLACK 0
#define WHITE 1

#define valWINDOW 50
typedef long long unsigned u64;

#if UINTPTR_MAX == 0xffffffffffffffff
#define BITScanForward(bits) (__builtin_ffsll(bits) - 1)
#else
#define  BITScanForward(bits) (((unsigned) (bits)) ? __builtin_ffs((unsigned) (bits)) - 1 : __builtin_ffs((bits) >> 32) + 31)
#endif

#define diff_time(a,b) ((int) (1000 * (((struct timeb)a).time - ((struct timeb)b).time) + (((struct timeb)a).millitm - ((struct timeb)b).millitm)))
#define myassert(a) if(!(a)){cout<<endl<<_time::getLocalTime()<<" **********************************assert error IN "<<__FILE__<< " on line "<<__LINE__<<" **********************************\n"<<flush;exit(1);};

#ifdef DEBUG_MODE
#define ASSERT(a) myassert(a)
#define INC(a) (a++)
#else
#define ASSERT(a)
#define INC(a)
#endif
typedef u64 Tchessboard[12];
typedef struct {
  uchar promotionPiece;
  char pieceFrom;
  uchar capturedPiece;
  uchar from;
  uchar to;
  char side;
  uchar type;
  int score;
  bool used;
} _Tmove;

#define VALUEPAWN 100
#define VALUEROOK 520
#define VALUEBISHOP 335
#define VALUEKNIGHT 330
#define VALUEQUEEN 980
#define VALUEKING _INFINITE

#define E1 3
#define E8 59
#define B1 6
#define G1 1
#define B8 62
#define G8 57
#define C1 5
#define F1 2
#define C8 58
#define F8 61

#define PAWN_CAPTUKING_BLACK 12
#define PAWN_CAPTUKING_WHITE 13
#define NO_PROMOTION -1
#define rotateBoard90(x) (ROTATE90[(x) & 0xFFFFULL] | ROTATE90[((x) >> 16) & 0xFFFFULL]<<2 | ROTATE90[((x) >> 32)	& 0xFFFFULL]<<4 | ROTATE90[((x) >> 48) & 0xFFFFULL]<<6)

#endif
