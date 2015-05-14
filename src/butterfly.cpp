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
u64 num_moves, num_movesq, num_tot_moves;
char mply, black_move;
int MAX_TIME_MILLSEC, MAX_DEPTH_TO_SEARCH, HASH_SIZE, st_force, list_id, xboard, force;
int evaluateMobility_mode, null_sem, supplementary_mill_sec;
u64 n_cut_hash;

unsigned long Index_BitScanForward;
char BITCOUNT[65536];
int Friend_king[2];
int enpas, running, ob_count, LazyEvalCuts;
int Ttime, quies_mode, max_depth, hand_do_movec, otime;
int CASTLE_DONE[2];
int ENP_POSSIBILE;
int main_ply;
int CASTLE_NOT_POSSIBLE[2];
int CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
int CASTLE_NOT_POSSIBLE_KINGSIDE[2];
int START_CASTLE_NOT_POSSIBLE[2];
int START_CASTLE_NOT_POSSIBLE_QUEENSIDE[2];
int START_CASTLE_NOT_POSSIBLE_KINGSIDE[2];
int START_CASTLE_DONE[2];
struct timeb start_time;
TmoveList gen_list;
Tchessboard chessboard;
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
EVAL_TAG EVAL;
//stack_move_tag stack_move1;
int initialply;
Tmove result_move;
#else
u64 n_perft;
u64 listcount_n;
#endif
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
char N_EVALUATION[2];
double beta_efficency1, beta_efficency_tot;
int beta_efficency_tot_count;
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
  running = 1;
  ftime ( &start1 );
  init (  );
  Tmove *mossa;
  list_id++;
  ASSERT ( list_id < MAX_PLY );
  list_id = 0;
  generateCap ( STANDARD, side );
  generateMoves ( STANDARD, side );
  listcount = get_gen_list_count ( list_id );
  u64 tot = 0;
  printf ( "\n\n\t\t\t\t----- ply %d -----", mply + 1 );
  listcount_n++;
  u64 r;
  for ( ii = 1; ii <= listcount; ii++ ) {
    list_id = 0;
    n_perft = 0;
    mossa = &gen_list[list_id][ii];
    makemove ( mossa, &r );
    ael ( change_side ( side ), mply );
    if ( mply >= MAX_DEPTH_TO_SEARCH )
      running = 0;
    takeback ( mossa );
    char y;
    char x = getFen ( get_piece_at ( ( side ), TABLOG[mossa->from] ) );
    if ( x == 'p' || x == 'P' )
      x = ' ';
    if ( mossa->capture != 12 )
      y = '*';
    else
      y = '-';
    printf ( "\n %d\t%c%s%c%s \t%llu ", ii, x, decodeBoardinv ( mossa->from, side ), y, decodeBoardinv ( mossa->to, side ), n_perft );
    fflush ( stdout );
    tot += n_perft;
  }
  ftime ( &end1 );
  TimeTaken = diff_time ( end1, start1 );
  printf ( "\n       \t\t\t\t\t\t\t%.2lf seconds", ( double ) TimeTaken / 1000 );

#ifdef DEBUG_MODE
  char dummy[1000];
  memset ( dummy, 0, sizeof ( dummy ) );
  sprintf ( dummy, "\n%llu nodes", tot );
  if ( TimeTaken ) {
    memset ( dummy, 0, sizeof ( dummy ) );
    sprintf ( dummy, "\n%llu nodes per second", tot * 1000 / TimeTaken );
  }
#endif
  printf ( "\n%llu nodes", tot );
  fflush ( stdout );
}

#else

void
iterative_deeping ( int side ) {
  struct timeb start1, end1;
  int alpha = -_INFINITE;
  int beta = _INFINITE;
  LINE line2, line;
  int val;
  char pvv[200];
  Tmove move2;
  int TimeTaken;
  running = 1;
  mply = 0;
  int score;
  ftime ( &start_time );
  memset ( HistoryHeuristic, 0, sizeof ( HistoryHeuristic ) );
  memset ( KillerHeuristic, 0, sizeof ( KillerHeuristic ) );
#ifdef HASH_MODE
  memset ( hash_array[BLACK], 0, HASH_SIZE * sizeof ( Thash ) );
  memset ( hash_array[WHITE], 0, HASH_SIZE * sizeof ( Thash ) );
#endif
  ftime ( &start1 );
  while ( running ) {
    init (  );
    initialply = mply;
    memset ( &line2, 0, sizeof ( LINE ) );
    memset ( &pvv, 0, sizeof ( pvv ) );
		/******** open book ***************/
    if ( use_book ) {
      u64 key = makeZobristKey ( side );
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
	result_move.promotion_piece = -1;
	//printf ("\nopen book found %d %d", result_move.from, result_move.to);
	running = 0;
	return;
      }
    }
    ++mply;

#ifdef DEBUG_MODE
    printf ( "\n\nply: %d ...", mply );
#endif
    main_ply = mply;
    val = ael ( side, mply, alpha, beta, &line2, makeZobristKey (  ), 1 );

    if ( !running )
      return;
    memcpy ( &line, &line2, sizeof ( LINE ) );
    /*if (!line.cmove ||((val < alpha) || (val > beta))) {//Aspiration Windows
       alpha = -_INFINITE;
       beta = _INFINITE;
       mply--;
       //printf("\nAspiration Windows");
       continue;
       } else {
       alpha = val - valWINDOW;
       beta = val + valWINDOW;
       //printf("\nAspiration Windows. alpha: %d beta: %d", alpha, beta);
       } */

    memcpy ( &move2, line.argmove, sizeof ( Tmove ) );

    for ( int t = 0; t < line.cmove; t++ ) {
      char pvv_tmp[10];
      memset ( pvv_tmp, 0, sizeof ( pvv_tmp ) );
      ASSERT ( t < MAX_PLY );
      ASSERT ( t >= 0 );
      strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].from, side ) );
      if ( line.argmove[t].from >= 0 )
	strcat ( pvv_tmp, decodeBoardinv ( line.argmove[t].to, side ) );
      strcat ( pvv, pvv_tmp );
      strcat ( pvv, " " );
    };
#ifdef TEST_MODE
    if ( line.argmove[0].from == KINGSIDE ) {
      if ( side )
	strcpy ( test_found, "g1" );
      else
	strcpy ( test_found, "g8" );
    }
    else if ( line.argmove[0].from == QUEENSIDE ) {

      if ( side )
	strcpy ( test_found, "c1" );
      else
	strcpy ( test_found, "c8" );
    }
    else
      strcpy ( test_found, decodeBoardinv ( line.argmove[0].to, side ) );
#endif
    score = -_INFINITE;
    memcpy ( &result_move, &move2, sizeof ( Tmove ) );
    if ( result_move.from >= 0 && result_move.to >= 0 )
      HistoryHeuristic[result_move.from][result_move.to] += 100000;
    ftime ( &end1 );
    TimeTaken = diff_time ( end1, start1 );
    num_tot_moves += ( num_movesq + num_moves );
    printf ( "\n%d %d %d %llu", mply, result_move.score / 100, TimeTaken, num_movesq + num_moves );
    printf ( " %s", pvv );
    int nps = 0;
    if ( ( TimeTaken ) )
      nps = ( int ) ( ( num_movesq + num_moves ) * 1000 / TimeTaken );

    fflush ( stdout );
#ifdef DEBUG_MODE
    if ( n_cut ) {
      double b = beta_efficency1 / ( ( int ) n_cut ) * 100;
      printf ( "\nbeta_efficency: %.2f%% ", b );
      beta_efficency_tot += b;
      beta_efficency_tot_count++;
    }
    printf ( "\nmillsec: %d  (%d nodes per seconds) \ntot moves: %llu \ntot moves quis: %llu", TimeTaken, nps, num_tot_moves, num_movesq );
    printf ( "\nnull_move_cut: %llu", null_move_cut );
    printf ( "\nn_cut: %llu\nLazyEvalCuts: %d", n_cut, LazyEvalCuts );
    printf ( "\neval_count: %llu", eval_count );
#ifdef FP_MODE
    printf ( "\nn_cut_FutilityPruning: %llu", n_cut_fp );
    printf ( "\nn_cut_razor: %llu", n_cut_razor );
#endif
#ifdef HASH_MODE
    printf ( "\nhash_record: %llu\nn_cut_hash: %llu\nhash_collisions: %llu", n_record_hash, n_cut_hash, collisions );
#endif
#endif
    mate = 0;
    /*if (abs(val) == _INFINITE) {
       mate = 1;
       if (!xboard)
       printf("\nMATE, stop search");
       running = 0;
       } */
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
#ifndef PERFT_MODE
  ASSERT ( stack_move1.next < MAX_PLY );
  ASSERT ( stack_move1.next >= 0 );
#endif
  int side = ( black_move == 1 ? 0 : 1 );
  if ( check_draw ( n_pieces ( side ) + BitCount ( chessboard[PAWN_BLACK + side] ), n_pieces ( side ^ 1 ) + BitCount ( chessboard[PAWN_BLACK + ( side ^ 1 )] ), side ) ) {
    writeWinboard ( "\nRESULT 1/2-1/2 {insufficient mating material}" );
    exit ( 0 );
#ifdef _MSC_VER
    return;
#else
    return NULL;
#endif
  }
  u64 key = makeZobristKey (  );
  char t[255];
  memset ( t, 0, sizeof ( t ) );
  hand_do_movec = 1;
  memset ( &result_move, 0, sizeof ( result_move ) );
  iterative_deeping ( side );
  if ( result_move.from == 0 && result_move.to == 0 ) {
    list_id++;
    generateMoves ( STANDARD, side );
    generateCap ( STANDARD, side );
    int listcount = get_gen_list_count ( list_id );
    if ( !listcount ) {
      if ( side )
	writeWinboard ( "\nRESULT 0-1 {Black mates}" );
      else
	writeWinboard ( "\nRESULT 1-0 {White mates}" );
      list_id--;
#ifdef _MSC_VER
      return;
#else
      return NULL;
#endif
    }
    for ( int ii = 1; ii <= listcount; ii++ ) {
      Tmove *mossa = &gen_list[list_id][ii];
      u64 key;
      makemove ( mossa, &key );
      printf ( "\n%d %d %d", mossa->from, mossa->to, mossa->capture );
      if ( !in_check ( side ) ) {
	memcpy ( &result_move, mossa, sizeof ( Tmove ) );

	if ( !chessboard[KING_BLACK + side] ) {	//search for mate

	  break;
	}
      }
      takeback ( mossa );
    }
    list_id--;
  }
  makemove ( &result_move, &key );
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
  writeWinboard ( t );
}
#endif

void
start ( int argc, char *argv[] ) {
  int t;
  num_tot_moves = 0;
#ifndef PERFT_MODE
  openbook = NULL;
  //memset(&stack_move1, 0, sizeof(stack_move1));
#endif
#ifdef TEST_MODE
  num_moves_test = 0;
#endif

  MAX_TIME_MILLSEC = 5000;
  st_force = _INFINITE;
  MAX_DEPTH_TO_SEARCH = 32;
  for ( t = 0; t < 65536; t++ )
    BITCOUNT[t] = ( char ) BitCountSlow ( t );
#ifndef PERFT_MODE
  use_book = xboard = 0;
  //printf ("Load book ...");
  fflush ( stdout );
  use_book = load_open_book (  );
  /*
     if (!use_book)
     printf ("not found. Open book not used.\n"); //xboard catch error
     else
     printf ("ok\n");
   */
#endif
#ifdef HASH_MODE
  hash_array[WHITE] = hash_array[BLACK] = NULL;
#ifdef DEBUG_MODE
  beta_efficency_tot_count = 0;
  beta_efficency_tot = 0.0;
  for ( t = 0; t < OPENBOOK_SIZE; t++ )
    if ( ( openbook[t].from_black != -1 && openbook[t].to_black < 0 ) || ( openbook[t].from_white != -1 && openbook[t].to_white < 0 ) )
      printf ( "\nerror" );

#endif
  HASH_SIZE = 2097091;		//default 64 Mb
  if ( argc > 1 )
    for ( t = 1; t < argc; t++ ) {
      if ( strstr ( argv[t], "-hash32" ) )
	HASH_SIZE = 1048549;	//must be a prime number
      else if ( strstr ( argv[t], "-hash64" ) )
	HASH_SIZE = 2097091;
      else if ( strstr ( argv[t], "-hash128" ) )
	HASH_SIZE = 4194301;
      else if ( strstr ( argv[t], "-hash256" ) )
	HASH_SIZE = 8388593;
      else if ( strstr ( argv[t], "-hash512" ) )
	HASH_SIZE = 16777213;
      else if ( strstr ( argv[t], "-hash1024" ) )
	HASH_SIZE = 33554467;
    }
  hash_array[BLACK] = ( Thash * ) malloc ( HASH_SIZE * sizeof ( Thash ) );
  if ( !hash_array[BLACK] )
    myassert ( 0, "\nMemory Allocation Failure\n" );
  hash_array[WHITE] = ( Thash * ) malloc ( HASH_SIZE * sizeof ( Thash ) );
  if ( !hash_array[WHITE] )
    myassert ( 0, "\nMemory Allocation Failure\n" );
#endif
#ifndef TEST_MODE
  loadfen ( INITIAL_FEN );
#endif
}

/*
 void bench() {
 unsigned r = 0;
 for (int t = 0; t < 9999999; t++) {
 for (int i = 0; i < 14; i++) {
 for (int j = 0; j < 64; j++) {
 ;
 }
 }
 }
 printf("\n%d", r);
 }
 */
int
main ( int argc, char *argv[] ) {
  //bench();      return 0;
  start ( argc, argv );
  printf ( "\nPERFT" );
#ifdef PERFT_MODE
  printf ( " ON" );
#else
  printf ( " OFF" );
#endif
  printf ( "\nHASH" );
#ifdef HASH_MODE
  printf ( " ON (use -hash32, -hash64, -hash128, -hash512, -hash1024) default -hash32" );
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
#ifndef PERFT_MODE
  if ( use_book )
    printf ( "\nOPEN BOOK YES\n" );
  else
#endif
    printf ( "\n" );		//OPEN BOOK NO (book.dat n o t  f o u n d, to create it type 'createbook')\n");
#ifdef TEST_MODE
  test (  );
#else
#ifndef PERFT_MODE
  print (  );
#endif

#ifdef PERFT_MODE
  if ( argc < 2 ) {
    printf ( "\nmissing depth and/or FEN position, example:\nbutterfly 3 \"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\nbutterfly 3\n" );
    return 1;
  };
  loadfen ( INITIAL_FEN );
  if ( argc == 3 )
    loadfen ( argv[2] );
  print (  );
  mply = ( char ) atoi ( argv[1] ) - 1;
  if ( mply < 0 || mply > 32 ) {
    printf ( "\nerror:  0 < depth <32" );
    return 1;
  }
  printf ( "\nthink...\n" );
  do_perft (  );
#endif
#endif

#ifndef PERFT_MODE
#ifdef _MSC_VER
  DWORD winboardthread;
  HANDLE thread1 = CreateThread ( NULL, 0, ( LPTHREAD_START_ROUTINE ) listner_winboard, ( LPVOID ) NULL, 0, &winboardthread );
  WaitForSingleObject ( thread1, INFINITE );
#else
  pthread_t thread1;
  pthread_create ( &thread1, NULL, listner_winboard, NULL );
  pthread_join ( thread1, NULL );
#endif
#endif
  return 0;

}
