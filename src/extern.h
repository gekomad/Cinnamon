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

#if !defined(EXTERN_H)
#define EXTERN_H
#ifdef DEBUG_MODE
#include "assert.h"
#endif
#include <sys/timeb.h>
extern int euristic_pruning, EvalCuts;

#ifdef TEST_MODE
extern char test_ris[20];
extern char test_trovato[20];
extern u64 num_moves_test;
#endif
extern unsigned long Index_BitScanForward;
extern int re_amico[2], st_force, MAX_DEPTH_TO_SEARCH, HASH_SIZE, supplementary_mill_sec;
extern u64 num_moves, num_moves2, num_movesq, num_tot_moves;
extern char mply, black_move;
extern int MAX_TIME_MILLSEC, winbtime, list_id, xboard, maxdep, incremental_move, force;
extern int attacco, evaluateMobility_mode, null_sem;
extern u64 n_cut_hash;
extern char *BITCOUNT;
extern int pos_king[1];
extern int pvv_da, pvv_a, path_pvv;
extern int DO_QUIES, FLG_WIN_WHITE, FLG_WIN_BLACK, FLG_DRAW;
extern int enpas, run, ob_count, fine_apertura1, mate;
extern int quies_mode, max_depth, hand_do_movec;
extern int CASTLE_DONE[2];
extern int ENP_POSSIBILE;
extern int CASTLE_NOT_POSSIBLE[2];
extern int CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
extern int CASTLE_NOT_POSSIBLE_KINGSIDE[2];
extern struct timeb start_time;
extern int max_depth_quies;
extern TmoveList gen_list;
extern Tchessboard chessboard;
extern fen_node FEN_STACK;
extern FILE *outf;
#ifndef PERFT_MODE
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
extern Teval evalNode;
extern int initialply;
extern Tmove result_move;
#else
extern u64 n_perft;
extern u64 listcount_n;
#endif

#ifndef PERFT_MODE
#ifdef DEBUG_MODE
extern double beta_efficency;
extern u64 eval_count;
extern u64 null_move_cut, collisions, n_record_hash;
extern u64 n_cut, n_cut_fp, n_cut_razor;
#endif
#endif
#endif
