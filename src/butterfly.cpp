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
#ifndef __GNUWIN32__
#include <sys/wait.h>
#endif
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

#ifdef TEST_MODE
char test_ris[20];
char test_found[20];
u64 num_moves_test;
#endif

u64 num_moves, num_moves2, num_movesq, num_tot_moves;
char mply, black_move;

int MAX_TIME_MILLSEC, MAX_DEPTH_TO_SEARCH, HASH_SIZE, st_force, list_id, xboard, force;
int evaluateMobility_mode, null_sem, supplementary_mill_sec;
u64 n_cut_hash;
char *BITCOUNT = NULL;

unsigned long Index_BitScanForward;
int Friend_king[2];
int pvv_from, pvv_to, path_pvv;
int enpas, run, ob_count, EvalCuts;
int quies_mode, max_depth, hand_do_movec;
int CASTLE_DONE[2];
int ENP_POSSIBILE;
int CASTLE_NOT_POSSIBLE[2];

int CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
int CASTLE_NOT_POSSIBLE_KINGSIDE[2];
struct timeb start_time;
FILE *outf;
TmoveList gen_list;

Tchessboard chessboard;
fen_node FEN_STACK;
#ifndef PERFT_MODE
Topenbook *openbook;
int use_book, OPENBOOK_SIZE;
#endif
#ifdef HASH_MODE
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
  int TimeTaken = 0;

  run = 1;
  //mply = 0;
  num_moves2 = 0;

  //while ( run ) {
  ftime ( &start1 );
  init (  );

  Tmove *mossa;
  list_id++;
#ifdef DEBUG_MODE
  assert ( list_id < MAX_PLY );
#endif
  list_id = 0;

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
    char x = getFen ( get_piece_at ( ( side ), TABLOG[mossa->from] ) );
    if ( x == 'p' || x == 'P' )
      x = ' ';
    if ( mossa->capture != 12 )
      y = '*';
    else
      y = '-';
    printf ( "\n %d\t%c%s%c%s \t%I64u ", ii, x, decodeBoardinv ( mossa->from, side ), y, decodeBoardinv ( mossa->to, side ), n_perft );
    tot += n_perft;
  }
  ftime ( &end1 );
  TimeTaken = diff_time ( end1, start1 );
  printf ( "\n       \t\t\t\t\t\t\t%.2lf seconds", ( double ) TimeTaken / 1000 );
#ifdef DEBUG_MODE
  //printf ( "\n%I64u nodes", tot );
  char dummy[1000];
  memset ( dummy, 0, sizeof ( dummy ) );
  sprintf ( dummy, "\n%I64u nodes", tot );

  if ( TimeTaken ) {
    memset ( dummy, 0, sizeof ( dummy ) );
    sprintf ( dummy, "\n%I64u nodes per second", tot * 1000 / TimeTaken );

  }
  //printf ( "\n%I64u nodes per second", tot * 1000 / TimeTaken );
#else
  printf ( "\n%I64u nodes", tot );
#endif

  while ( 1 );
}

#else
/*
void
do_move ( int side ) {	
struct timeb start1,end1;
int alpha = -_INFINITE;
int beta = _INFINITE;
LINE line;  
int val;  
int TimeTaken;
run = 1;
mply = 1;
int score;  
ftime ( &start_time ); 
init (  ); 
while ( run ) {
ftime ( &start1 );
initialply = mply;


++mply;

printf ( "\nply: %d ................................................", mply );

list_id=0;
gen_list[list_id][0].score=0;
num_moves2 = 0;
generateCap ( STANDARD, side );
generateMoves ( STANDARD, side );
int listcount = gen_list[list_id][0].score;
Tmove *mossa;
if ( listcount ) {
int ii;		
for ( ii = 1; ii <= listcount; ii++ ) {
mossa = &gen_list[list_id][ii];				
makemove ( mossa );
alpha = -_INFINITE;
beta = _INFINITE;
//print();			
val = -ael ( side^1, mply, -beta,-alpha, &line );
printf("\n%s %s %d",decodeBoardinv ( mossa->from,  mossa->side ),decodeBoardinv (  mossa->to,  mossa->side ),val);
takeback ( mossa );

run = still_time (  );
if(!run){printf("\nnnnnnnnnnn");break;}
score = -_INFINITE;
memcpy ( &result_move, &mossa, sizeof ( Tmove ) );
}  
}
ftime (&end1);
TimeTaken = diff_time (end1, start1);
printf("\ntime %d",TimeTaken);
}

}
*/
void
do_move ( int side ) {
  struct timeb start1, end1;
  int alpha = -_INFINITE;
  int beta = _INFINITE;
  LINE line;
  int val;
  char pvv[200];
  Tmove move2;
  int TimeTaken;
  run = 1;
  mply = 0;
  int score;
  num_moves2 = 0;
  ftime ( &start_time );
  memset ( HistoryHeuristic, 0, sizeof ( HistoryHeuristic ) );
  memset ( KillerHeuristic, 0, sizeof ( KillerHeuristic ) );
#ifdef HASH_MODE
  memset ( hash_array[BLACK], 0, HASH_SIZE * sizeof ( Thash ) );
  memset ( hash_array[WHITE], 0, HASH_SIZE * sizeof ( Thash ) );
#endif
  ftime ( &start1 );
  while ( run ) {
    init (  );
    initialply = mply;
    memset ( &line, 0, sizeof ( line ) );
    memset ( &pvv, 0, sizeof ( pvv ) );
		/******** open book ***************/
    if ( use_book ) {
      u64 key = makeZobristKey (  );
      int t;
      if ( ( t = search_openbook ( key, side ) ) != -1 ) {
	ob_count++;
	if ( side ) {
	  result_move.from = openbook[t].from_white;
	  result_move.to = openbook[t].to_white;
	}
	else {
	  result_move.from = openbook[t].from_black;
	  result_move.to = openbook[t].to_black;
	}
	result_move.type = STANDARD;	//TODO
	result_move.side = ( char ) side;
	//result_move.type=openbook[t].eval;
	//printf ("\nopen book found %d %d", result_move.from, result_move.to);
	run = 0;
	return;
      }
    }
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
      pvv_from = line.argmove[0].from;
      pvv_to = line.argmove[0].to;
      for ( int t = 0; t < line.cmove; t++ ) {
	char pvv_tmp[10];
	memset ( pvv_tmp, 0, sizeof ( pvv_tmp ) );
	strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].from, side ) );
	if ( line.argmove[t].from >= 0 )
	  strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].to, side ) );
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
      if ( line.argmove[0].from == KINGSIDE ) {
	if ( side )
	  strcpy ( test_found, "g1" );
	strcpy ( test_found, "g8" );
      }
      else if ( line.argmove[0].from == QUEENSIDE ) {
	if ( side )
	  strcpy ( test_found, "c1" );
	strcpy ( test_found, "c8" );
      }
      else
	strcpy ( test_found, decodeBoardinv ( line.argmove[0].to, side ) );
#endif
      if ( mply == MAX_DEPTH_TO_SEARCH )
	run = 0;
      score = -_INFINITE;
      memcpy ( &result_move, &move2, sizeof ( Tmove ) );
      /* for ( int a = 0; a < 64; a++ )
         for ( int b = 0; b < 64; b++ )
         if ( HistoryHeuristic[result_move.from][result_move.to] >= 100000 )//TODO ??
         HistoryHeuristic[result_move.from][result_move.to] -= 100000;
         HistoryHeuristic[result_move.from][result_move.to] += 100000; */
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
      printf ( " null_move_cut: %I64d\n", null_move_cut );
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
	if ( !xboard )
	  printf ( "\nMATE, stop search" );
	run = 0;
      }
      int tipo = result_move.type;
      if ( result_move.from < 0 )
	tipo = CASTLE;

      // SIDE = side;
      /*if(inCheck(result_move.from,result_move.to,tipo)){
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
  int side = ( black_move == 1 ? 0 : 1 );
  do_move ( side );
  if ( result_move.from == 0 && result_move.to == 0 ) {
    list_id++;
    generateMoves ( STANDARD, side );
    generateCap ( STANDARD, side );
    int listcount = gen_list[list_id][0].score;
    if ( !listcount ) {
      printf ( "MATE!!" );
#ifdef _MSC_VER
      return;
#else
      return NULL;
#endif
    }
    int ii;
    for ( ii = 1; ii <= listcount; ii++ ) {
      Tmove *mossa = &gen_list[list_id][ii];
      makemove ( mossa );
      if ( !in_check (  ) ) {
	memcpy ( &result_move, mossa, sizeof ( Tmove ) );
	takeback ( mossa );
	break;
      }
      takeback ( mossa );
    }
  }
  list_id--;
  makemove ( &result_move );
  print (  );
  push_fen (  );
  strcpy ( t, "\nmove " );
  strcat ( t, decodeBoardinv ( result_move.from, result_move.side ) );
  if ( result_move.type != CASTLE ) {
    strcat ( t, decodeBoardinv ( result_move.to, result_move.side ) );
    if ( result_move.promotion_piece != -1 )
      t[strlen ( t )] = ( char ) tolower ( getFen ( result_move.promotion_piece ) );
  }


#ifdef DEBUG_MODE
  printf ( "da %d a %d\n", result_move.from, result_move.to );
#endif
  //#ifdef _MSC_VER
  writeWinboard ( t );
  //#endif
}
#endif
void
dispose (  ) {
#ifdef HASH_MODE
  free ( hash_array[BLACK] );
  free ( hash_array[WHITE] );
  free ( openbook );
#endif
  free ( BITCOUNT );
  int i;
  for ( i = 0; i < FEN_STACK.count; i++ )
    free ( FEN_STACK.fen[i] );

}

void
start ( int argc, char *argv[] ) {
  int t;
  num_tot_moves = 0;
#ifndef PERFT_MODE
  openbook = NULL;
#endif
#ifdef TEST_MODE
  num_moves_test = 0;
#endif
  CASTLE_DONE[0] = 0;
  CASTLE_DONE[1] = 0;
  CASTLE_NOT_POSSIBLE[0] = 0;
  CASTLE_NOT_POSSIBLE[1] = 0;
  CASTLE_NOT_POSSIBLE_QUEENSIDE[0] = 0;
  CASTLE_NOT_POSSIBLE_QUEENSIDE[1] = 0;
  CASTLE_NOT_POSSIBLE_KINGSIDE[0] = 0;
  CASTLE_NOT_POSSIBLE_KINGSIDE[1] = 0;

  BITCOUNT = ( char * ) malloc ( 65536 * sizeof ( char ) );
  MAX_TIME_MILLSEC = 5000;
  st_force = _INFINITE;
  MAX_DEPTH_TO_SEARCH = 32;
  for ( t = 0; t < 65536; t++ )
    BITCOUNT[t] = ( char ) BitCountSlow ( t );
#ifndef PERFT_MODE
  use_book = xboard = 0;
  if ( 0 )
    printf ( "Load book ..." );
  fflush ( stdout );
  use_book = load_open_book (  );
  if ( 0 ) {
    if ( !use_book )
      printf ( "not found. Open book not used.\n" );	//xboard catch error
    else
      printf ( "ok\n" );
  }
#endif
#ifdef HASH_MODE
  hash_array[WHITE] = hash_array[BLACK] = NULL;

#ifdef DEBUG_MODE
  for ( t = 0; t < OPENBOOK_SIZE; t++ ) {
    if ( openbook[t].from_black != -1 )
      if ( openbook[t].to_black < 0 )
	printf ( "\nerror" );;
    if ( openbook[t].from_white != -1 )
      if ( openbook[t].to_white < 0 )
	printf ( "\nerror" );;
  }
#endif
  HASH_SIZE = 2097091;		//default 64 Mb
  if ( argc > 1 )
    for ( t = 1; t < argc; t++ ) {
      if ( strstr ( argv[t], "-hash32" ) )
	HASH_SIZE = 1048549;	//must be a prime numbers
      else if ( strstr ( argv[t], "-hash64" ) )
	HASH_SIZE = 2097091;
      else if ( strstr ( argv[t], "-hash128" ) )
	HASH_SIZE = 4194301;
      else if ( strstr ( argv[t], "-hash256" ) )
	HASH_SIZE = 8388593;
      else if ( strstr ( argv[t], "-hash512" ) )
	HASH_SIZE = 16777213;

    }
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
main ( int argc, char *argv[] ) {
  start ( argc, argv );

  //0x5d20d128d3ff0f40ULL
  //int a=BITScanForward2(0x5d20d128d3ff0f40ULL);
  //int b=BITScanForward(0x5d20d128d3ff0f40ULL);
  /* for(int i=0;i<13;i++)
     for(int i1=0;i1<64;i1++){
     int a=non_iterative_popcount(zobrist_key[i][i1]);
     int b=BitCount(zobrist_key[i][i1]);
     assert(a==b);
     printf("\n%I64u %d %d",zobrist_key[i][i1],a,b);
     } 
   */
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
#ifndef PERFT_MODE
  print (  );
#endif
  push_fen (  );
#ifdef PERFT_MODE
  if ( argc < 2 ) {
    printf ( "\nmissing depth and FEN, example 3 \"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"" );
    return 1;
  };
  loadfen ( INITIAL_FEN );
  if ( argc == 3 )
    loadfen ( argv[2] );
  print (  );
  mply = ( char ) atoi ( argv[1] ) - 1;
  printf ( "\nthink...\n" );
  do_perft (  );
#endif
#endif
#ifndef PERFT_MODE
#ifndef TEST_MODE
  while ( 1 ) {
#if defined  _MSC_VER	|| defined  __GNUWIN32__
    Sleep ( _INFINITE );
#else
    usleep ( _INFINITE );
#endif
  }
#endif
#endif
  return 0;
}
