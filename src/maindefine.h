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

#if !defined(__MAINDEFINE_H)
#define __MAINDEFINE_H

// div 8   >> 3
// * 8   << 3
/*
63 62 61 60 59 58 57 56
55 54 53 52 51 50 49 48
47 46 45 44 43 42 41 40
39 38 37 36 35 34 33 32
31 30 29 28 27 26 25 24
23 22 21 20 19 18 17 16
15 14 13 12 11 10 09 08
07 06 05 04 03 02 01 00

Depth 	Perft(Depth) 	Total Nodes
1 		20 					20				verified
2 		400 				420				verified
3 		8902 				9322			verified
4 		197281 				206603			verified
5 		4865609 			5072212			verified
6 		119060324 			124132536		verified
7 		3195901860 			3320034396		verified
8 		84998978956 		88319013352		verified
9 		2439530234167 		2527849247519
10 		69352859712417 		71880708959936
                        
*/
//http://chessprogramming.wikispaces.com/Perft+Results
//indent -br -l1000 -nce -cdw -cli0 -cbi0 -prs -sai -saf -di1 -nbc -brs -brf -bli0  *.cpp *.h
//xboard -fcp ./butterfly
#define INITIAL_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "

//#define INITIAL_FEN "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1   "

#define debugfile "out.log"
#define _INFINITE 2147483646
#ifdef _MSC_VER
typedef unsigned __int64 u64;
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#define longlong long long unsigned
typedef longlong u64;
#endif
typedef unsigned char uchar;
#ifndef PERFT_MODE
#define OPENBOOK_FILE "book.dat"
#endif

//thanks to Beowulf :-)
#define ROOK_ATTACK 4
#define BISHOP_ATTACK 4
#define KNIGHT_ATTACK 4
#define QUEEN_ATTACK 4
#define OPEN_FILE_Q 3
#define FORK_SCORE 4
#define BONUS2BISHOP 15
#define MOB 1
#define KING_TRAPPED 2
#define KNIGHT_TRAPPED 20
#define BISHOP_TRAPPED 100
#define ROOK_TRAPPED 30
#define QUEEN_TRAPPED 4
#define CONNECTED_ROOKS 5
#define ROOK_BLOCKED 2
#define ROOK_7TH_RANK 16
#define OPEN_FILE 6
#define BLOCK_PAWNS 2
#define UNDEVELOPED 10
#define HALF_OPEN_FILE_Q 3
#define DOUBLED_PAWNS 12
#define PAWN_7H 15
#define PED_CENTRE 1
#define KING_PROXIMITY 2
#define ISO 18
#define PAWN_PUSH 4
#define SPACE 8
#define DIST_XKING 1
#define END_OPENING 1
#define ENEMY_NEAR_KING 1
#define XQUEEN_NEAR_KING 1
#define PAWN_NEAR_KING 3
#define BONUS_CASTLE 8
#define NEAR_XKING 3
#define BONUS_11 1
#define BISHOP_ON_QUEEN 1
#define ENEMIES_PAWNS_ALL 1
#define DOUBLED_ISOLATED_PAWNS 25
#define BACKWARD_PAWN 2
#define BACKWARD_OPEN_PAWN 3


#define END_OPEN (max_pieces_per_side (0)<6 || max_pieces_per_side (0)<6 )
#define DRAW   "1/2-1/2 {Draw by repetition}"
#define WIN_BLACK   "0-1 {Black mates}"
#define WIN_WHITE   "1-0 {White mates}"


#define MAX_MOVE   90
#define MAX_PLY   32


#define SQUARE_FREE   12
#define PAWN_BLACK   0
#define PAWN_WHITE   1
#define ROOK_BLACK   2
#define ROOK_WHITE   3
#define BISHOP_BLACK   4
#define BISHOP_WHITE   5
#define KNIGHT_BLACK   6
#define KNIGHT_WHITE   7
#define KING_BLACK   8
#define KING_WHITE   9
#define QUEEN_BLACK   10
#define QUEEN_WHITE   11
#define PAWN_CAPTUKING_BLACK   12
#define PAWN_CAPTUKING_WHITE   13

#define PAWN_S   0
#define ROOK_S   1
#define BISHOP_S   2
#define KNIGHT_S   3
#define RE_S   4
#define QUEEN_S   5
#define PAWN_CAPTURE_S_BLACK   6
#define PAWN_CAPTURE_S_WHITE   7
//#define BONUS_CASTLE  200

#define KINGSIDE -1
#define QUEENSIDE -2
#define CASTLE -3
#define PROMOTION -4
#define BLACK   0
#define WHITE   1
#define STANDARD  1
#define EVALUATION   102
#define ENPASSANT   101


#define VALUEPAWN 100
#define VALUEROOK 520
#define VALUEBISHOP 335
#define VALUEKNIGHT 330
#define VALUEKING _INFINITE
#define VALUEQUEEN 980
#define ATTACK_F7_F2 10
#define BOOK_EPD "book.epd"
#ifdef HASH_MODE

#define hashfEXACT  0
#define hashfALPHA 1
#define hashfBETA 2


//#define UPDATE_TXT "../open_book.txt"
#endif
#define D2 12
#define E2 11
#define D3 20
#define E3 19

#define E7 51
#define D7 52

#define E6 43
#define D6 44
#define F7 50
#define F2 10


const int PIECES_VALUE[13] = {
  VALUEPAWN, VALUEPAWN,
  VALUEROOK, VALUEROOK,
  VALUEBISHOP, VALUEBISHOP,
  VALUEKNIGHT, VALUEKNIGHT,
  VALUEKING, VALUEKING,
  VALUEQUEEN, VALUEQUEEN,
  0
};

#define _max(a,b) (a>b?a:b)
#define square_bit_occupied(x) (x==BLACK ? (chessboard[PAWN_BLACK]|chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]):(chessboard[PAWN_WHITE]|chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE]))
#define get_pieces(x) (x==BLACK ? chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]:chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE])
#define n_pieces(x) (BitCount(get_pieces(x)))

#define max_pieces_per_side(tipo) (BitCount(square_bit_occupied(tipo)))

#define in_check()(attack_square(BLACK,BitScanForward (chessboard[KING_BLACK ])) ? 1:(attack_square(WHITE,BitScanForward (chessboard[KING_WHITE])) ? 1:0))
#define R_adpt(tipo,depth) (2+((depth) > (3+((max_pieces_per_side(tipo)<3)?2:0))))
#define null_ok(depth,side)((null_sem) ? 0:(depth < 3 ?0:(max_pieces_per_side(side) < 4 ? 0:1)))

#define square_all_bit_occupied() (chessboard[PAWN_BLACK]|chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]|chessboard[PAWN_WHITE]|chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE])

#define shift32(b) (((unsigned*)&b)[1])

#define make_extension()(((chessboard[WHITE] & ORIZZONTAL_48)||(chessboard[BLACK] & ORIZZONTAL_8))? 1: 0)
#ifdef DEBUG_MODE
#define check_side(side) (assert(side==0 || side==1))
#endif
#define get_piece_at(side,tablogpos)((side==WHITE)?(\
(chessboard[PAWN_WHITE]&tablogpos)?PAWN_WHITE:(\
(chessboard[KING_WHITE]&tablogpos)?KING_WHITE:(\
(chessboard[ROOK_WHITE]&tablogpos)?ROOK_WHITE:(\
(chessboard[BISHOP_WHITE]&tablogpos)?BISHOP_WHITE:(\
(chessboard[KNIGHT_WHITE]&tablogpos)?KNIGHT_WHITE:(\
(chessboard[QUEEN_WHITE]&tablogpos)?QUEEN_WHITE:SQUARE_FREE)))))):\
(\
(chessboard[PAWN_BLACK]&tablogpos)?PAWN_BLACK:(\
(chessboard[KING_BLACK]&tablogpos)?KING_BLACK:(\
(chessboard[ROOK_BLACK]&tablogpos)?ROOK_BLACK:(\
(chessboard[BISHOP_BLACK]&tablogpos)?BISHOP_BLACK:(\
(chessboard[KNIGHT_BLACK]&tablogpos)?KNIGHT_BLACK:(\
(chessboard[QUEEN_BLACK]&tablogpos)?QUEEN_BLACK:SQUARE_FREE)))))))


#define lazy_eval_black() (BitCount(chessboard[0])*VALUEPAWN+\
BitCount(chessboard[2])*VALUEROOK+\
BitCount(chessboard[4])*VALUEBISHOP+\
BitCount(chessboard[6])*VALUEKNIGHT+\
BitCount(chessboard[10])*VALUEQUEEN)



#define lazy_eval_white() (BitCount(chessboard[1])*VALUEPAWN+\
BitCount(chessboard[3])*VALUEROOK+\
BitCount(chessboard[5])*VALUEBISHOP+\
BitCount(chessboard[7])*VALUEKNIGHT+\
BitCount(chessboard[11])*VALUEQUEEN)


#ifdef FP_MODE

#define FUTIL_MARGIN 2*VALUEPAWN + VALUEPAWN
#define EXT_FUTILY_MARGIN  VALUEROOK + VALUEBISHOP + VALUEPAWN
#define RAZOR_MARGIN  VALUEQUEEN + VALUEPAWN

#endif
#define ACTUAL_COUNT 39
#define valWINDOW 200

#define VOID_FEN "8/8/8/8/8/8/8/8/R w KQkq - 0 1"
#define TABSALTOPAWN 0xFF00000000FF00ULL
#define TABCAPTUREPAWN_RIGHT 0xFEFEFEFEFEFEFEFEULL
#define TABCAPTUREPAWN_LEFT 0x7F7F7F7F7F7F7F7FULL

#ifndef PERFT_MODE
typedef struct TopenbookTag {
  u64 key;
  char from_white, to_white, from_black, to_black;
  int eval;
} Topenbook;

struct TopenbookLeaf {
  char from_white, to_white, from_black, to_black;
  int eval;
  u64 key;
  TopenbookLeaf *l;
  TopenbookLeaf *r;
};


typedef struct EvalTag {
  char open_column[2];
  char semi_open_column[2];
  char re_attaccato[64];
  int king_attak[2];
  int king_security[2];
  u64 isolated;
  u64 attaccate[64];
  u64 attaccanti[64];
} Teval;

#endif

typedef struct TmoveTag {
  char from, to, side, capture, type, promotion_piece;
  int score;

} Tmove;

#ifdef HASH_MODE
typedef struct ThashTag {
  u64 key;
  int score;
  char depth;
  char flags;
} Thash;
#endif

typedef struct {
  char *fen[100];
  int count;
} fen_node;

typedef u64 Tchessboard[12];
typedef u64 lll[MAX_PLY];

typedef Tmove TmoveList[MAX_PLY][MAX_MOVE];

typedef struct tagLINE {
  int cmove;			// Number of moves in the line.
  Tmove argmove[MAX_PLY];	// The line.
} LINE;

#endif
