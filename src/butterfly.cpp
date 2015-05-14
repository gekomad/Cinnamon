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

#include "winboard.h"
#ifdef _MSC_VER
#include "windows.h"
#include <Windows.h>
#include "stdafx.h"
#include "LIMITS.H"
#include "math.h"
#include <conio.h>

#else
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <stdio.h>
#include <math.h>

#include "maindefine.h"
#include "utility.h"
#include "eval.h"
#include "gen.h"
#include "cap.h"
#include "utility.h"
#ifdef TEST_MODE
#include "test.h"
#endif

#include "search.h"



#include <stddef.h>
#include <stdlib.h>

#include "gen.h"

#include "zobrist.h"
#include "openbook.h"
#include "bitmap.h"
#include "checkmove.h"


int euristic_pruning;


#ifdef TEST_MODE
char test_ris[20];
char test_trovato[20];
#endif

u64 num_moves, num_moves2, num_movesq, num_tot_moves;
char mply, black_move;
int winbtime, list_id, xboard, maxdep, incremental_move;
int evaluateMobility_mode, null_sem;
u64 n_cut_hash;
char *BITCOUNT = NULL;

int pos_re[1];
int attacco, re_amico[2];
int pvv_da, pvv_a, path_pvv;
int DO_QUIES, FLG_WIN_WHITE, FLG_WIN_BLACK, FLG_DRAW;
int enpas, run, ob_count, fine_apertura1, EvalCuts;
int quies_mode, max_depth, hand_do_movec;
int CASTLE_DONE[2];
int ENP_POSSIBILE;
int CASTLE_NOT_POSSIBLE[2];
int CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
int CASTLE_NOT_POSSIBLE_KINGSIDE[2];
struct timeb start_time;
int max_depth_quies;
TmoveList gen_list;

Tchessboard chessboard;
fen_node FEN_STACK;

#ifdef HASH_MODE
Topenbook *openbook;

int use_book, OPENBOOK_SIZE;
Thash *hash_array[2];

#endif
int mate;
#ifndef PERFT_MODE
int HistoryHeuristic[64][64];
int KillerHeuristic[MAX_PLY][64][64];
int main_depth;
TopenbookLeaf *openbook_tree;
Teval evalNode;
int initialply;
Tmove result_move;
#else
u64 n_perft;
u64 listcount_n;
#endif
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
double beta_efficency;
u64 eval_count;
u64 null_move_cut, collisions, n_record_hash;
u64 n_cut, n_cut_fp, n_cut_razor;
#endif
#endif


#ifdef PERFT_MODE
void
do_perft (  ) {
  int ii, listcount, side;
  side = ( black_move == 1 ? 0 : 1 );
  struct timeb start1, end1;
  max_depth_quies = 0;
  int TimeTaken = 0;
  side = ( black_move == 1 ? 0 : 1 );

  run = 1;
  mply = 0;
  num_moves2 = 0;
  init (  );
  while ( run ) {
    ftime ( &start1 );
    init (  );

    Tmove *mossa;
    list_id++;
#ifdef DEBUG_MODE
    assert ( list_id < MAX_PLY );
#endif
    list_id = 0;
    re_amico[side] = BitScanForward ( chessboard[KING_BLACK + side] );
    re_amico[side ^ 1] = BitScanForward ( chessboard[KING_BLACK + ( side ^ 1 )] );
    generateCap ( STANDARD, side );
    generateMoves ( STANDARD, side );

    listcount = gen_list[list_id][0].score;
    u64 tot = 0;

#ifdef DEBUG_MODE
    controlloRipetizioni ( gen_list[list_id], listcount );
#endif
    printf ( "\n\n\t\t\t\t----- ply %d -----", mply + 1 );
    listcount_n++;


    for ( ii = 1; ii <= listcount; ii++ ) {
      list_id = 0;
      n_perft = 0;
      num_moves2 = 0;
      mossa = &gen_list[list_id][ii];
      makemove ( mossa );
      ael ( side ^ 1, mply );
      if ( mply >= MAX_DEPTH_TO_SEARCH )
	run = 0;
      num_moves2 += n_perft;
      takeback ( mossa );
      char y;
      char x = getFen ( get_piece_at ( ( side ), TABLOG[mossa->da] ) );
      if ( x == 'p' || x == 'P' )
	x = ' ';
      if ( mossa->capture != 12 )
	y = '*';
      else
	y = '-';
      printf ( "\n %d\t%c%s%c%s \t%I64u ", ii, x, decodeBoardinv ( mossa->da, side ), y, decodeBoardinv ( mossa->a, side ), n_perft );
      tot += n_perft;
    }
    ftime ( &end1 );
    TimeTaken = diff_time ( end1, start1 );
    printf ( "\n       \t\t\t\t\t\t\t%.2lf seconds", ( double ) TimeTaken / 1000 );
#ifdef DEBUG_MODE
    printf ( "\n%I64u nodes", tot );
    if ( TimeTaken )
      printf ( "\n%I64u nodes per second", tot * 1000 / TimeTaken );
#else
    printf ( "\n%I64u nodes", tot );
#endif

    ++mply;
  }
}
#else
void
do_move (  ) {
  int side;
  struct timeb start1, end1;
  int alpha = -_INFINITE;
  int beta = _INFINITE;
  LINE line;
  max_depth_quies = 0;
  int val;
  char pvv[200];
  Tmove move2;
  int TimeTaken;
  side = ( black_move == 1 ? 0 : 1 );
  run = 1;
  mply = 1;
  //ftime (&start_time);
  int score;
  num_moves2 = 0;
  ftime ( &start_time );
  init (  );
  while ( run ) {
    ftime ( &start1 );
    initialply = mply;
    memset ( &line, 0, sizeof ( line ) );
    memset ( &pvv, 0, sizeof ( pvv ) );
	/******** open book ***************/
#ifdef HASH_MODE
    /*if(use_book){
       u64 key =makeZobristKey();
       int t;
       if ((t=search_openbook(key,side))!=-1){
       ob_count++;
       if (side){result_move.da=openbook[t].da_white;result_move.a=openbook[t].a_white;}else
       {result_move.da=openbook[t].da_black;result_move.a=openbook[t].a_black;}
       printf("\nopen book found %d %d",result_move.da,result_move.a);
       run=0;
       return ;
       }
       } */
#endif
    //if (!retry)
    ++mply;
#ifdef DEBUG_MODE
    printf ( "\nply: %d ...", mply );
#endif
    line.cmove = 0;
    val = ael ( side, mply, alpha, beta, &line );
    run = still_time (  );
    if ( !line.cmove ) {
      run = 0;
    }
    if ( run ) {
      /*if ((val < alpha) || (val > beta) ) {//Aspiration Windows
         alpha = -_INFINITE;
         beta = _INFINITE;
         continue;
         }else {
         alpha = val - valWINDOW;
         beta = val + valWINDOW;
         printf("\nAspiration Windows. alpha: %d beta: %d",alpha,beta );
         } */
      memcpy ( &move2, line.argmove, sizeof ( Tmove ) );
      if ( !line.cmove )
	continue;
#ifdef DEBUG_MODE
      int errore = 0;
#endif
      pvv_da = line.argmove[0].da;
      pvv_a = line.argmove[0].a;
      for ( int t = 0; t < line.cmove; t++ ) {
	char pvv_tmp[10];
	memset ( pvv_tmp, 0, sizeof ( pvv_tmp ) );
	strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].da, side ) );
	if ( line.argmove[t].da >= 0 )
	  strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].a, side ) );
#ifdef DEBUG_MODE
	if ( strstr ( pvv, pvv_tmp ) )
	  errore = 1;
#endif
	strcat ( pvv, pvv_tmp );
	strcat ( pvv, " " );
#ifdef DEBUG_MODE
	//if(errore){printf("\nErrore ripetizione %d\n",run);fflush(stdout);};
#endif
      };

#ifdef TEST_MODE
      strcpy ( test_trovato, decodeBoardinv ( line.argmove[0].a, side ) );
#endif
      if ( mply == MAX_DEPTH_TO_SEARCH )
	run = 0;
      score = -_INFINITE;
      memcpy ( &result_move, &move2, sizeof ( Tmove ) );
      for ( int a = 0; a < 64; a++ )
	for ( int b = 0; b < 64; b++ )
	  if ( HistoryHeuristic[result_move.da][result_move.a] >= 100000 )
	    HistoryHeuristic[result_move.da][result_move.a] -= 100000;
      HistoryHeuristic[result_move.da][result_move.a] += 100000;
      ftime ( &end1 );
      TimeTaken = diff_time ( end1, start1 );
#ifdef DEBUG_MODE
      if ( n_cut )
	printf ( "\nbeta_efficency: %2f ", beta_efficency / ( ( int ) n_cut ) * 100 );
#endif
      num_tot_moves += ( num_movesq + num_moves );
      printf ( "\n%d %d %d %d", mply, result_move.score / 100, TimeTaken, num_movesq + num_moves );
      printf ( " %s", pvv );

#ifdef DEBUG_MODE
      if ( n_cut )
	printf ( "\nbeta_efficency: %f ", beta_efficency / ( ( int ) n_cut ) * 100 );
#endif
      int nps = 0;
      if ( ( TimeTaken ) )
	nps = ( int ) ( ( num_movesq + num_moves2 ) * 1000 / TimeTaken );
#ifdef DEBUG_MODE

      printf ( "\nmillsec: %d  (%d nodes per seconds) tot moves: %I64u tot moves quis: %I64u", TimeTaken, nps, num_moves2, num_movesq );
      /*
         printf(" tot enpassant: %d\n",enpassant_count);
         printf(" tot check: %d\n",check_count);
         printf(" tot capture: %d\n",capture_count);
         printf(" tot castle: %d\n",castle_count);
         printf(" tot checkmate: %d\n",checkmate_count); */
#endif
      fflush ( stdout );
#ifdef DEBUG_MODE
      printf ( "null_move_cut: %I64d\n", null_move_cut );
      printf ( "n_cut: %I64d EvalCuts: %d\n", n_cut, EvalCuts );
#ifdef FP_MODE
      printf ( "n_cut_FutilityPruning: %I64d\n", n_cut_fp );
      printf ( "n_cut_razor: %I64d\n", n_cut_razor );
#endif
#ifdef HASH_MODE
      printf ( "record: %I64d n_cut_hash: %I64u collisions: %I64d\n", n_record_hash, n_cut_hash, collisions );
#endif
#ifdef DEBUG_MODE
      printf ( "eval_count %I64u\n", eval_count );
#endif
#endif
      mate = 0;
      if ( abs ( val ) == _INFINITE ) {
	mate = 1;
	printf ( "\nMATE, stop search" );
	run = 0;
      }
      int tipo = result_move.tipo;
      if ( result_move.da < 0 )
	tipo = CASTLE;

      // SIDE = side;
      /*if(inCheck(result_move.da,result_move.a,tipo)){
         run=1;
         MATE_MODE=1;
         }  */
      if ( black_move && result_move.score == _INFINITE )
	break;
      if ( !black_move && result_move.score == -_INFINITE )
	break;
    }
  }
}

#endif
#ifndef PERFT_MODE
#ifdef _MSC_VER
void
hand_do_move (  )
#else
void *
hand_do_move ( void *dummy )
#endif
{
  char t[255];
  memset ( t, 0, sizeof ( t ) );
  hand_do_movec = 1;
  do_move (  );
#ifdef HASH_MODE
#ifdef DEBUG_MODE
  if ( result_move.capture == 0 )
    printf ( "\nresult: da: %d a: %d score: %d n_cut_hash %d", result_move.da, result_move.a, result_move.score, n_cut_hash );
  else
    printf ( "\nopen book result_move: da: %d a: %d ", result_move.da, result_move.a );
#endif
#endif
  //memcpy (chessboard, chessboard2, sizeof (Tchessboard));
  makemove ( &result_move );
  print (  );
  push_fen (  );
  strcpy ( t, "\nmove " );
  strcat ( t, decodeBoardinv ( result_move.da, result_move.side ) );
  if ( result_move.da >= 0 )
    strcat ( t, decodeBoardinv ( result_move.a, result_move.side ) );
#ifdef DEBUG_MODE
  printf ( "%d %d\n", result_move.da, result_move.a );
#endif
#ifdef _MSC_VER
  writeWinboard ( t );
#endif
}
#endif

void
start (  ) {
  int t;
  num_tot_moves = 0;
  BITCOUNT = ( char * ) malloc ( 65536 * sizeof ( char ) );
  for ( t = 0; t < 65536; t++ )
    BITCOUNT[t] = ( char ) BitCountSlow ( t );

#ifdef HASH_MODE
  hash_array[WHITE] = hash_array[BLACK] = NULL;
  use_book = xboard = 0;
  use_book = load_open_book (  );
  //if (!use_book)
  //printf ("not found. Open book not used.\n");
#ifdef DEBUG_MODE
  for ( t = 0; t < OPENBOOK_SIZE; t++ ) {
    if ( openbook[t].da_black != -1 )
      if ( openbook[t].a_black < 0 )
	printf ( "\nerrore" );;
    if ( openbook[t].da_white != -1 )
      if ( openbook[t].a_white < 0 )
	printf ( "\nerrore" );;
  }
#endif
  hash_array[BLACK] = ( Thash * ) malloc ( HASH_SIZE * sizeof ( Thash ) );
  myassert ( hash_array[BLACK], "\nMemory Allocation Failure\n" );
  hash_array[WHITE] = ( Thash * ) malloc ( HASH_SIZE * sizeof ( Thash ) );
  myassert ( hash_array[WHITE], "\nMemory Allocation Failure\n" );

#endif
#ifndef TEST_MODE
  loadfen ( INITIAL_FEN );
#endif

#ifndef PERFT_MODE
#ifdef _MSC_VER
  DWORD winboardthread;
  CreateThread ( NULL, 0, ( LPTHREAD_START_ROUTINE ) listner_winboard, ( LPVOID ) NULL, 0, &winboardthread );
#else
  pthread_t thread1;
  int iret1;
  char *message1 = "Thread 1";
  iret1 = pthread_create ( &thread1, NULL, listner_winboard, ( void * ) &message1 );
#endif
#endif

}

int
main (  ) {
  start (  );
  FEN_STACK.count = 0;
  printf ( "\nPERFT" );
#ifdef PERFT_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nHASH" );
#ifdef HASH_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nNULL MOVES" );
#ifdef NULL_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nFUTILITY PRUNING" );
#ifdef FP_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nDEBUG" );
#ifdef DEBUG_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\n64 bit" );
#ifdef HAS_64BITS
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nTEST" );
#ifdef TEST_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif

#ifndef TEST_MODE
  printf ( "\n" );
#endif
#ifdef TEST_MODE
  test (  );
#else

  print (  );
  push_fen (  );
#ifdef PERFT_MODE
  do_perft (  );
#endif
#endif
#ifndef PERFT_MODE
#ifndef TEST_MODE
#ifdef _MSC_VER

  while ( 1 )
    Sleep ( _INFINITE );
#else
  while ( 1 ) {
    usleep ( 99999999 );
  }
#endif
#endif
#endif
  return 0;
}
