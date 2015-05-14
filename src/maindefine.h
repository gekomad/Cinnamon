#ifndef MAINDEFINE_H_
#define MAINDEFINE_H_
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
 */
#include <sys/stat.h>
#define INITIAL_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#include <sys/timeb.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <string>
#include <stddef.h>
#include <stdlib.h>
#include <sstream>
#ifdef DEBUG_MODE
#include <assert.h>
#endif
using namespace std;
typedef unsigned char uchar;

#define MAX_MOVE 130
#define MAX_PLY 64
#define _INFINITE 2000000000
#define BLACK 0
#define WHITE 1
#define valWINDOW 50
typedef long long unsigned u64;
#define diff_time(a,b) ((int) (1000 * (((struct timeb)a).time - ((struct timeb)b).time) + (((struct timeb)a).millitm - ((struct timeb)b).millitm)))
#define _max(a,b) (a>b?a:b)

#if defined (__WIN32__)
#define sleep(n) Sleep(1000 * n)
#endif

#ifdef DEBUG_MODE
#define ASSERT(a) assert(a)
#define INC(a) (a++)
#else
#define ASSERT(a)
#define INC(a)
#endif
typedef u64 Tchessboard[12];
typedef struct {
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
  Tchessboard *stack_move;
  uchar nextBoard;
#endif
#endif
  char promotion_piece;
  char capture;
  char from;
  char to;
  char side;
  uchar type;
  int score;
  bool used;
} Tmove;

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

enum {
  OPEN, MIDDLE, FINAL,
};

#define PAWN_CAPTUKING_BLACK 12
#define PAWN_CAPTUKING_WHITE 13
#define NO_PROMOTION -1
#define rotateBoard90(x) (ROTATE90[(x) & 0xFFFFULL] | ROTATE90[((x) >> 16) & 0xFFFFULL]<<2 | ROTATE90[((x) >> 32)	& 0xFFFFULL]<<4 | ROTATE90[((x) >> 48) & 0xFFFFULL]<<6)
#define myassert(a) if(!(a)){cout<<"\n********************************** ERROR IN "<<__FILE__<< " on line "<<__LINE__<<" **********************************\n"<<flush;exit(1);};

#endif
