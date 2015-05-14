#if !defined(EXTERN_H)
#define EXTERN_H
#ifdef DEBUG_MODE
#include "assert.h"
#endif
#include <sys/timeb.h>
extern int LazyEvalCuts;

#ifdef TEST_MODE
extern char test_ris[20];
extern char test_found[20];
extern u64 num_moves_test;
#endif

extern unsigned long Index_BitScanForward;
extern int Friend_king[2], st_force, MAX_DEPTH_TO_SEARCH, HASH_SIZE, supplementary_mill_sec;
extern char BITCOUNT[65536];
extern u64 num_moves, num_movesq, num_tot_moves;
extern char mply, black_move;
extern int MAX_TIME_MILLSEC, list_id, xboard, force;
extern int otime, evaluateMobility_mode, null_sem;
extern u64 n_cut_hash;
extern int main_ply;
extern int Ttime, enpas, running, ob_count, mate;
extern int quies_mode, max_depth, hand_do_movec;
extern int CASTLE_DONE[2];
extern int ENP_POSSIBILE;
extern int CASTLE_NOT_POSSIBLE[2];
extern int CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
extern int CASTLE_NOT_POSSIBLE_KINGSIDE[2];
extern int START_CASTLE_NOT_POSSIBLE[2];
extern int START_CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
extern int START_CASTLE_NOT_POSSIBLE_KINGSIDE[2];
extern int START_CASTLE_DONE[2];
extern struct timeb start_time;
extern TmoveList gen_list;
extern Tchessboard chessboard;

#ifndef PERFT_MODE
extern EVAL_TAG EVAL;
extern int use_book, OPENBOOK_SIZE;
extern Topenbook *openbook;
#endif
#ifdef HASH_MODE
extern Thash *hash_array[2];
#endif
#ifndef PERFT_MODE
extern int HistoryHeuristic[64][64];
extern int KillerHeuristic[MAX_PLY][64][64];
extern int main_depth;
extern TopenbookLeaf *openbook_tree;
extern int initialply;
extern Tmove result_move;
#else
extern u64 n_perft;
extern u64 listcount_n;
#endif

#ifndef PERFT_MODE
//extern stack_move_tag stack_move1;
#ifdef DEBUG_MODE
extern char N_EVALUATION[2];
extern double beta_efficency1, beta_efficency_tot;
extern int beta_efficency_tot_count;
extern u64 eval_count;
extern u64 null_move_cut, collisions, n_record_hash;
extern u64 n_cut, n_cut_fp, n_cut_razor;
#endif
#endif
#endif
