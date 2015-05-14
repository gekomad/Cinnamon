#if !defined(__MAINDEFINE_H)
#define __MAINDEFINE_H

/*
 8|63 62 61 60 59 58 57 56
 7|55 54 53 52 51 50 49 48
 6|47 46 45 44 43 42 41 40
 5|39 38 37 36 35 34 33 32
 4|31 30 29 28 27 26 25 24
 3|23 22 21 20 19 18 17 16
 2|15 14 13 12 11 10 09 08
 1|07 06 05 04 03 02 01 00
 a  b  c  d  e  f  g  h

 Depth 			  Perft
 1 					20 		verified
 2 				   400 		verified
 3 				  8902 		verified
 4 				197281 		verified
 5 			   4865609 		verified
 6 			 119060324 		verified
 7 			3195901860 		verified
 8 		   84998978956 		verified
 9 		 2439530234167		not verified (2439530234532)
 10 	69352859712417

 */

#define INITIAL_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#ifdef _MSC_VER
//_mm_popcnt_u64 al posto di popcount only intelSSE4
#ifndef HAS_64BITS
#define BITScanForward( bits ) (_BitScanForward(&Index_BitScanForward,(unsigned long)bits)?Index_BitScanForward:(_BitScanForward(&Index_BitScanForward,bits>>32)?Index_BitScanForward+32:0))
#else
//#define BitCount(bits) ((unsigned)__popcnt64(bits))
#define BITScanForward( bits ) (_BitScanForward64(&Index_BitScanForward,bits)?Index_BitScanForward:0)
#endif
#endif
#define lazy_eval(SIDE) (SIDE? lazy_eval_white ()-lazy_eval_black ():lazy_eval_black () - lazy_eval_white ())

#define _INFINITE 2147483646
#ifdef _MSC_VER
typedef unsigned __int64 u64;

#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
typedef long long unsigned u64;
#endif
typedef unsigned char uchar;

#ifndef PERFT_MODE
#define OPENBOOK_FILE "book.dat"
#endif

#define PAWN_ATTACK_KING 25
#define ROOK_ATTACK_KING 19
#define BISHOP_ATTACK_KING 20
#define KNIGHT_ATTACK_KING        19
#define QUEEN_ATTACK_KING 18
#define OPEN_FILE_Q 3
#define FORK_SCORE 8
#define BONUS2BISHOP 15
#define MOB 2
#define KING_TRAPPED              28
#define KNIGHT_TRAPPED 10
#define BISHOP_TRAPPED_DIAG 20
#define BISHOP_TRAPPED 10
#define ROOK_TRAPPED 10
#define QUEEN_TRAPPED 20
#define CONNECTED_ROOKS 5
#define ROOK_BLOCKED 20
#define ROOK_7TH_RANK 16
#define OPEN_FILE 10
#define BLOCK_PAWNS 2
#define UNDEVELOPED 10
#define HALF_OPEN_FILE_Q 3
#define DOUBLED_PAWNS 6
#define PAWN_IN_RACE             91
#define PAWN_7H                   49
#define PED_CENTRE 1
#define FRIEND_NEAR_KING 1
#define ENEMY_NEAR_KING 2
#define ISO 5
#define PAWN_PUSH 4
#define SPACE 1
#define END_OPENING 5
#define KING_ATTACKED 10
#define PAWN_NEAR_KING 2
#define BONUS_CASTLE 15
#define NEAR_XKING 3
#define BONUS_11 1
#define BISHOP_ON_QUEEN 1
#define ENEMIES_PAWNS_ALL 1
#define DOUBLED_ISOLATED_PAWNS 10
#define BACKWARD_PAWN 2
#define BACKWARD_OPEN_PAWN 3

#define MAX_MOVE   90
#define MAX_PLY   64

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

#define KINGSIDE -1
#define QUEENSIDE -2
#define CASTLE -3
#define PROMOTION -4
#define BLACK   0
#define WHITE   1
#define STANDARD  1
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

#endif

#define D3 20
#define E3 19
#define E6 43
#define D6 44
#define F7 50
#define F2 10

#define C1 5
#define F1 2
#define C8 58
#define F8 61

#define E1 3
#define E8 59
const int PIECES_VALUE[13] = { VALUEPAWN, VALUEPAWN, VALUEROOK, VALUEROOK, VALUEBISHOP, VALUEBISHOP,

  VALUEKNIGHT, VALUEKNIGHT, VALUEKING, VALUEKING, VALUEQUEEN, VALUEQUEEN, 0
};

#define diff_time(a,b) ((int) (1000 * (((struct timeb)a).time - ((struct timeb)b).time) + (((struct timeb)a).millitm - ((struct timeb)b).millitm)))
#define _max(a,b) (a>b?a:b)
#define square_bit_occupied(x) (x==BLACK ? (chessboard[PAWN_BLACK]|chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]):(chessboard[PAWN_WHITE]|chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE]))
#define get_pieces(x) (x==BLACK ? chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]:chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE])
#define n_pieces(x) (BitCount(get_pieces((x))))

#define in_check(side)((side)==BLACK ?attack_square(BLACK,BITScanForward (chessboard[KING_BLACK ])) :(attack_square(WHITE,BITScanForward (chessboard[KING_WHITE])) ))
#define R_adpt(tipo,depth) (2+(depth > (3+((n_pieces(tipo)<3)?2:0))))
#define null_ok(depth,side,n_pieces_side)(null_sem ? 0:(depth < 3 ?0:(n_pieces_side < 4 ? 0:1)))

#define square_all_bit_occupied() (chessboard[PAWN_BLACK]|chessboard[ROOK_BLACK]|chessboard[BISHOP_BLACK]|chessboard[KNIGHT_BLACK]|chessboard[KING_BLACK]|chessboard[QUEEN_BLACK]|chessboard[PAWN_WHITE]|chessboard[ROOK_WHITE]|chessboard[BISHOP_WHITE]|chessboard[KNIGHT_WHITE]|chessboard[KING_WHITE]|chessboard[QUEEN_WHITE])

//#define make_extension()(((chessboard[WHITE] & ORIZZONTAL_48)||(chessboard[BLACK] & ORIZZONTAL_8))? 1: 0)
#ifdef DEBUG_MODE
#define check_side(side) (assert(side==0 || side==1))
#define ASSERT(a) assert(a)
#else
#define get_gen_list(list_id,i) (&gen_list[list_id][i])
#define get_gen_list_count(list_id) (gen_list[list_id][0].score)
#define ASSERT(a)
#define check_side(side)
#define tablog(x) TABLOG[x]
#define Chessboard(x) chessboard[x]
#define change_side(side) (side^1)
#define get_piece_at(side,tablogpos)((side==WHITE)?(\
(chessboard[PAWN_WHITE] & tablogpos) ? PAWN_WHITE : (\
(chessboard[KING_WHITE] & tablogpos) ? KING_WHITE : (\
(chessboard[ROOK_WHITE] & tablogpos) ? ROOK_WHITE : (\
(chessboard[BISHOP_WHITE] & tablogpos) ? BISHOP_WHITE : (\
(chessboard[KNIGHT_WHITE] & tablogpos) ? KNIGHT_WHITE : (\
(chessboard[QUEEN_WHITE] & tablogpos) ? QUEEN_WHITE : SQUARE_FREE)))))):\
  (\
   (chessboard[PAWN_BLACK] & tablogpos) ? PAWN_BLACK : (\
							(chessboard[KING_BLACK] & tablogpos) ?KING_BLACK: ((chessboard		[ROOK_BLACK] &\
		tablogpos) ?\
ROOK_BLACK\
: (\
		(chessboard\
				[BISHOP_BLACK]\
				& tablogpos) ?\
		BISHOP_BLACK\
		: (\
				(chessboard\
						[KNIGHT_BLACK]\
						& tablogpos)\
				?\
				KNIGHT_BLACK\
				: (\
						(chessboard\
								[QUEEN_BLACK]\
								&\
								tablogpos)\
						?\
						QUEEN_BLACK\
						:\
						SQUARE_FREE)))))))

#endif

#define lazy_eval_black() (BitCount(chessboard[0])*VALUEPAWN+\
  BitCount (chessboard[2]) * VALUEROOK +  \
BitCount (chessboard[4]) * VALUEBISHOP +  \
BitCount (chessboard[6]) * VALUEKNIGHT +  \
BitCount (chessboard[10]) * VALUEQUEEN)

#define lazy_eval_white() (BitCount(chessboard[1])*VALUEPAWN+\
  BitCount (chessboard[3]) * VALUEROOK +  \
BitCount (chessboard[5]) * VALUEBISHOP +  \
BitCount (chessboard[7]) * VALUEKNIGHT +  \
BitCount (chessboard[11]) * VALUEQUEEN)

#ifdef FP_MODE
#define FUTIL_MARGIN (2*VALUEPAWN )
#define EXT_FUTILY_MARGIN  (VALUEROOK)
#define RAZOR_MARGIN  (VALUEQUEEN + VALUEPAWN)
#endif
#define valWINDOW 50
#define VOID_FEN "8/8/8/8/8/8/8/8/R w KQkq - 0 1"
#define TABJUMPPAWN 0xFF00000000FF00ULL
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

typedef struct {
  u64 ALLPIECES;
  unsigned short pos_king[2];
  u64 all_pieces[2];
  u64 pawns[2];
  u64 queens[2];
  u64 rooks[2];
  char open_column[2];
  char semi_open_column[2];
  char king_attacked[64];
  char END_OPEN;
  int king_security_distance[2];
  u64 isolated[2];
  u64 attacked[64];
  u64 attackers[64];
#ifdef TEST_MODE
  int passed_pawn_score[2];
  int race_pawn_score[2];
#endif
} EVAL_TAG;
#endif

typedef struct {
  u64 board[MAX_PLY];
  int next;
} stack_move_tag;

#ifdef HASH_MODE
typedef struct ThashTag {
  u64 key;
  int score;
  char depth;
  char flags;
} Thash;

#endif
typedef struct {
  char *fen[1000];
  int count;
} fen_node;

typedef u64 Tchessboard[12];
typedef struct TmoveTag {
#ifdef DEBUG_MODE
  Tchessboard board;
#endif
  char from, to, side, capture, type, promotion_piece;
  int score;
} Tmove;
typedef Tmove TmoveList[MAX_PLY][MAX_MOVE];

typedef struct tagLINE {
  int cmove;
  Tmove argmove[MAX_PLY];
} LINE;

#endif
