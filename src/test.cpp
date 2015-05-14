#include "stdafx.h"
#ifdef TEST_MODE
#include <time.h>
#include "maindefine.h"
#include "gen.h"
#include "butterfly.h"
#include "utility.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utility.h"
#include <sys/stat.h>
#include "extern.h"
#include "eval.h"
#include "zobrist.h"
#ifdef _MSC_VER
#include <windows.h>
#endif

/*example output crafty:
 material.......  -1.95
 pawns..........  -0.12
 passed pawns...  -0.27
 knights........   0.00
 bishops........   0.02
 rooks..........   0.22
 queens.........   0.00
 kings..........   0.00
 development....   0.00
 pawn races.....   0.00
 total..........  -2.10*/

void
extract_crafty_scores ( const char *PATH_LOG, double *crafty_material, double *crafty_pawns, double *crafty_passed_pawns, double *crafty_knights, double *crafty_bishop, double *crafty_rooks, double *crafty_queens, double *crafty_kings, double *crafty_development, double *crafty_pawn_race, double *crafty_total ) {
  char buf[300];
  FILE *streamLog;
  streamLog = fopen ( PATH_LOG, "r" );
  if ( !streamLog )
    myassert ( 0, "open crafty.log error" );
  while ( fgets ( buf, sizeof ( buf ), streamLog ) != NULL ) {
    if ( strstr ( buf, "material......." ) )
      *crafty_material = atof ( buf + 15 );
    if ( strstr ( buf, "pawns.........." ) )
      *crafty_pawns = atof ( buf + 15 );
    if ( strstr ( buf, "passed pawns..." ) )
      *crafty_passed_pawns = atof ( buf + 15 );
    if ( strstr ( buf, "knights........" ) )
      *crafty_knights = atof ( buf + 15 );
    if ( strstr ( buf, "bishops........" ) )
      *crafty_bishop = atof ( buf + 15 );
    if ( strstr ( buf, "rooks.........." ) )
      *crafty_rooks = atof ( buf + 15 );
    if ( strstr ( buf, "queens........." ) )
      *crafty_queens = atof ( buf + 15 );
    if ( strstr ( buf, "kings.........." ) )
      *crafty_kings = atof ( buf + 15 );
    if ( strstr ( buf, "development...." ) )
      *crafty_development = atof ( buf + 15 );
    if ( strstr ( buf, "pawn races....." ) )
      *crafty_pawn_race = atof ( buf + 15 );
    if ( strstr ( buf, "total.........." ) )
      *crafty_total = atof ( buf + 15 );
  }
  fclose ( streamLog );
}

void
test_eval_crafty ( const char *testfile ) {
  /*
     CREATE TABLE `eval` (
     `id` int(11) NOT NULL,
     `fen` text NOT NULL,
     `butterfly_material` int(11) NOT NULL,
     `butterfly_pawns` int(11) NOT NULL,
     `butterfly_passed_pawns` int(11) NOT NULL,
     `butterfly_knights` int(11) NOT NULL,
     `butterfly_bishop` int(11) NOT NULL,
     `butterfly_rooks` int(11) NOT NULL,
     `butterfly_queens` int(11) NOT NULL,
     `butterfly_kings` int(11) NOT NULL,
     `butterfly_development` int(11) NOT NULL,
     `butterfly_pawn_race` int(11) NOT NULL,
     `butterfly_total` int(11) NOT NULL,
     `crafty_material` double NOT NULL,
     `crafty_pawns` double NOT NULL,
     `crafty_passed_pawns` double NOT NULL,
     `crafty_knights` double NOT NULL,
     `crafty_bishop` double NOT NULL,
     `crafty_rooks` double NOT NULL,
     `crafty_queens` double NOT NULL,
     `crafty_kings` double NOT NULL,
     `crafty_development` double NOT NULL,
     `crafty_pawn_race` double NOT NULL,
     `crafty_total` double NOT NULL,
     PRIMARY KEY (`id`)
     ) ENGINE=MyISAM DEFAULT CHARSET=latin1';
   */
  double crafty_material;
  double crafty_pawns;
  double crafty_passed_pawns;
  double crafty_knights;
  double crafty_bishop;
  double crafty_rooks;
  double crafty_queens;
  double crafty_kings;
  double crafty_development;
  double crafty_pawn_race;
  double crafty_total;
  int butterfly_material;
  int butterfly_pawns;
  int butterfly_passed_pawns;
  int butterfly_knights;
  int butterfly_bishop;
  int butterfly_rooks;
  int butterfly_queens;
  int butterfly_kings;
  int butterfly_development;
  int butterfly_pawn_race;
  int butterfly_total;
  FILE *stream;
  FILE *evalcsv;
  //const char* PATH_LOG = "/tmp/ramdisk/crafty.log";const char* PATH_CRAFTY = "/tmp/ramdisk/crafty";
  const char *PATH_LOG = "/home/geko/crafty.log";	//to speed up uses RAMDISK to store this file
  const char *PATH_CRAFTY = "/home/geko/chess/crafty-23.4/crafty";

  //const char* PATH_LOG="r:\\crafty.log";const char* PATH_CRAFTY="r:\\crafty-23.4-win32.exe";
  printf ( "\n ****** START test_eval_crafty %s  *******", testfile );
  printf ( "\nTo speed up put crafty.log in RAMDISK:\n\ton linux: mkdir /tmp/ramdisk; chmod 777 /tmp/ramdisk;sudo mount -t tmpfs -o size=5M tmpfs /tmp/ramdisk/" );

  printf ( "\n\ton windows: http://www.mydigitallife.info/free-ramdisk-for-windows-vista-xp-2000-and-2003-server" );

  printf ( "\n\nLoad eval.csv in mysql: mysqlimport --local -u root -p butterfly --fields-terminated-by='|' eval.csv" );

  printf ( "\nand calculates Mean Square Error:"
	   "\nSELECT fen, butterfly_bishop, crafty_bishop *100, POW( butterfly_bishop - crafty_bishop *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
	   "\nSELECT fen, butterfly_kings, crafty_kings *100, POW( butterfly_kings - crafty_kings *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
	   "\nSELECT fen, butterfly_knights, crafty_knights *100, POW( butterfly_knights - crafty_knights *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
	   "\nSELECT fen, butterfly_material, crafty_material *100, POW( butterfly_material - crafty_material *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
	   "\nSELECT fen, butterfly_passed_pawns, crafty_passed_pawns *100, POW( butterfly_passed_pawns - crafty_passed_pawns *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_pawns, crafty_pawns *100, POW( butterfly_pawns - crafty_pawns *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_pawn_race, crafty_pawn_race *100, POW( butterfly_pawn_race - crafty_pawn_race *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_queens, crafty_queens *100, POW( butterfly_queens - crafty_queens *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_rooks, crafty_rooks *100, POW( butterfly_rooks - crafty_rooks *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_total, crafty_total *100, POW( butterfly_total - crafty_total *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT sum(pow(butterfly_total-crafty_total*100,2))/(select count(1) from eval )from eval\n" "\ngenerating eval.csv..." );

  fflush ( stdout );
  struct timeb start, end;
  ftime ( &start );
  int count = 0;
  num_tot_moves = 0;
  char line[2001];
  char buf[2001];
  int side;
  evalcsv = fopen ( "eval.csv", "w" );
  if ( !evalcsv )
    myassert ( 0, "error eval.csv" );
  strcpy ( line, testfile );
  stream = fopen ( line, "r" );
  if ( !stream ) {
    memset ( line, 0, sizeof ( line ) );
    strcpy ( line, "../" );
    strcat ( line, testfile );
    stream = fopen ( line, "r" );
  }
  if ( !stream )
    myassert ( 0, "error file not found" );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    if ( line[strlen ( line ) - 1] = '\n' )
      line[strlen ( line ) - 1] = 0;
    if ( line[strlen ( line ) - 1] = '\r' )
      line[strlen ( line ) - 1] = 0;
    //printf("\n|%s|",line);fflush(stdout);
    side = loadfen ( line, 0 );
    count++;
    eval ( side,
#ifdef FP_MODE
	   -_INFINITE, _INFINITE,
#endif
	   0, &butterfly_material, &butterfly_pawns, &butterfly_passed_pawns, &butterfly_knights, &butterfly_bishop, &butterfly_rooks, &butterfly_queens, &butterfly_kings, &butterfly_development, &butterfly_pawn_race, &butterfly_total );
    sprintf ( buf, "%s log=off ponder=off setboard \"%s\" score quit>%s", PATH_CRAFTY, line, PATH_LOG );
    //printf("\n|%s|",buf);fflush(stdout);
    int dummy = system ( buf );
    extract_crafty_scores ( PATH_LOG, &crafty_material, &crafty_pawns, &crafty_passed_pawns, &crafty_knights, &crafty_bishop, &crafty_rooks, &crafty_queens, &crafty_kings, &crafty_development, &crafty_pawn_race, &crafty_total );
    fprintf ( evalcsv, "%d|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f\n", count, line, butterfly_material, butterfly_pawns, butterfly_passed_pawns, butterfly_knights, butterfly_bishop, butterfly_rooks, butterfly_queens, butterfly_kings, butterfly_development, butterfly_pawn_race, butterfly_total, crafty_material, crafty_pawns, crafty_passed_pawns, crafty_knights, crafty_bishop, crafty_rooks, crafty_queens, crafty_kings, crafty_development, crafty_pawn_race, crafty_total );
  }
  fclose ( stream );
  fclose ( evalcsv );
  ftime ( &end );
  printf ( "\nEND test, time:  %d total", diff_time ( end, start ) );
  fflush ( stdout );
}

void
extract_end_book ( const char *testfile ) {
  int count = 0;
  int foundit = 0;
  FILE *stream;
  char line[2001];
  int side;
  strcpy ( line, testfile );
  stream = fopen ( line, "r" );
  if ( !stream ) {
    memset ( line, 0, sizeof ( line ) );
    strcpy ( line, "../" );
    strcat ( line, testfile );
    stream = fopen ( line, "r" );
  }
  if ( !stream )
    myassert ( 0, "test error file not found" );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    loadfen ( line, 0 );
    //KBK
    if ( n_pieces ( WHITE ) == 2 && BitCount ( chessboard[PAWN_WHITE] ) == 0 && BitCount ( chessboard[BISHOP_WHITE] ) == 1 && n_pieces ( BLACK ) == 1 && BitCount ( chessboard[PAWN_BLACK] ) == 0 )
      //if( BitCount(chessboard[PAWN_BLACK])<3 && BitCount(chessboard[PAWN_WHITE]<3) )//&& n_pieces(WHITE)+n_pieces(BLACK)<8 )
      //if(strlen(line)<55)
      printf ( "%s", line );
  }
  fclose ( stream );
}

void
test_epd ( char *testfile ) {
  printf ( "\n ****** START TEST %s max time %d sec *******", testfile, MAX_TIME_MILLSEC / 1000 );
  struct timeb start, end;
  ftime ( &start );
  int count = 0;
  int foundit = 0;
  num_tot_moves = 0;
  FILE *stream;
  char line[2001];
  int side;
  strcpy ( line, testfile );
  stream = fopen ( line, "r" );
  if ( !stream ) {
    memset ( line, 0, sizeof ( line ) );
    strcpy ( line, "../" );
    strcat ( line, testfile );
    stream = fopen ( line, "r" );
  }
  if ( !stream )
    myassert ( 0, "test error file not found" );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    //strcpy (line,"r2qkb1r/pp3ppp/2bppn2/8/2PQP3/2N2N2/PP3PPP/R1B1K2R w KQkq - bm O-O; id sbd.093");
    printf ( "\n%s", line );
    side = loadfen ( line );
    count++;
    print (  );
    iterative_deeping ( side );
    if ( strstr ( test_ris, test_found ) ) {
      foundit++;
      printf ( "\nOK" );
    }
    else {
      printf ( "\nKO" );
    }
    if ( mate ) {
      printf ( " MATE" );
    }
    printf ( " RESULT: (%s %s) %s \nfound %d/%d", test_found, test_ris, line, foundit, count );
  }
  fclose ( stream );
  ftime ( &end );
  printf ( "\ntime:  %d total nodes per whole test %llu", diff_time ( end, start ), num_tot_moves );
  printf ( "\n ****** END TEST  %s found %d/%d *******", testfile, foundit, count );
#ifdef DEBUG_MODE
  printf ( "beta_efficency_tot %.2f%%", beta_efficency_tot / beta_efficency_tot_count );
#endif
  fflush ( stdout );
}

void
test (  ) {
  MAX_TIME_MILLSEC = 5000;
  const char *EPD_FILE = "/home/geko/chess/book.epd";
  //test_eval_crafty(EPD_FILE);return ;
  //extract_end_book(EPD_FILE);
  test_epd ( "wac.epd" );

  test_epd ( "sbd.epd" );
  test_epd ( "kaufman.epd" );
  test_epd ( "zugzwang.epd" );
  test_epd ( "bk.epd" );
  test_epd ( "mate.epd" );	//generated by http://www.frayn.net/beowulf/matetest.zip
  test_epd ( "arasan12.epd" );
}

#endif
