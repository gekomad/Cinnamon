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
#define LUNG_FILE 300
#define MAX_RANDOM 500

#define get_random( a)	((int)(rand()%a+1.0))

/*#define popola_rr sprintf(rr,"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",\
MOB,KING_TRAPPED,KNIGHT_TRAPPED,BISHOP_TRAPPED,ROOK_TRAPPED,QUEEN_TRAPPED,CONNECTED_ROOKS,\
ROOK_ATTACK,BISHOP_ATTACK,KNIGHT_ATTACK,QUEEN_ATTACK,ROOK_BLOCKED,ROOK_7TH_RANK,OPEN_FILE,\
BLOCK_PAWNS,UNDEVELOPED,HALF_OPEN_FILE_Q,OPEN_FILE_Q,FORK_SCORE,DOUBLED_PAWNS,PAWN_7H,\
PED,KING_PROXIMITY,ISO,PASS,DIST_,DISTA,END_OPENING,QUAD,PEDAM);*/


typedef struct MTag {
  char fen[LUNG_FILE][200];
  int side[LUNG_FILE];
} Mnode;
Mnode M;
const int results_crafty_wac[LUNG_FILE] = { 198, 45, 31, -70, -384, 148, -155, -314, -510, 72, -121, -472, 9, -23,
  -303, 34, 22, -200,
  223, 68, -147, 75, -49, 4, -41, -175, 348, -69, -33, -54, -47, 10, -145, 58,
  -190, -64, -45, 382, 87, 50, -651, -40, -84, -245,
  -67, -3, -128, -53, 53, 660, 362, -282, -110, 139, 29, 51, 191, -91, -66,
  443, 188, -276, -175, -103, -9, 308, 127, -139, -192, -11,
  93, -63, -146, 326, -321, -95, 396, -187, -132, -16, 286, 52, -31, 308,
  -286, 4, 199, -181, 391, -73, 102, -44, 109, 185, 277, -233,
  -319, 114, -165, -476, 18, -85, 112, 58, 383, -24, 74, 78, -140, -59, 56,
  -461, -217, -62, 33, 84, 43, 35, -76, -63, -138, 275,
  -168, -102, 32, 319, -141, 424, 113, -122, 248, 325, -346, -159, -70, -218,
  -2, 328, 210, -227, 204, 76, -63, 97, -59, -283, 30, -63,
  -12, 182, 12, 49, 108, -433, -91, 209, 271, 16, -155, -456, -6, 186, 23,
  -107, 216, 65, 356, 217, 20, 101, 5, 301, 162, 94,
  245, -38, -395, 151, 110, 110, 371, 83, 24, -101, 155, 126, -188, 152, -373,
  -173, -216, -332, -25, -236, 99, 48, -605, -26, -44, -156,
  -94, -25, -45, 273, -61, 237, 315, -28, -245, 212, -79, 296, 627, -60, 316,
  -68, 18, -25, -375, 230, -322, -130, 177, 96, 160, 224,
  74, 33, 110, 195, -57, -45, 211, -37, -214, -1, 51, -56, -117, -7, -147,
  -10, -149, -97, -224, -199, -216, -108, -22, -10, -119, -224,
  124, 50, -36, -142, -113, 188, 143, -286, 168, -93, -72, -136, -212, -117,
  -9, -26, 102, -57, -99, -108, 94, -83, 95, 229, -28, -51,
  114, 89, -372, 207, 740, -334, 854, -353, -333, -52, 45, 281, 32, 314, 7,
  -306, 305, -58, -46, -292, -238, 34
};

typedef struct teststructt {
  char val[200];
  double eval;
} teststruct;
char rr[2001];

u64
diff_crafty (  ) {
  u64 media = 0;
  int yy;
  int t = 0;


  for ( int uu = 0; uu < LUNG_FILE; uu++ ) {
    loadfen ( M.fen[uu] );
    // print();
    yy = eval ( M.side[uu]
#ifdef FP_MODE
		, -_INFINITE, _INFINITE
#endif
#ifdef HASH_MODE
		, 0
#endif
       );
    // printf("\n%d %d",yy,results_crafty_wac[t]);
    media += ( u64 ) ( ( yy - results_crafty_wac[t] ) ) * ( ( yy - results_crafty_wac[t] ) );
    if ( yy > 0 && results_crafty_wac[t] < 0 || yy < 0 && results_crafty_wac[t] > 0 )
      media += ( u64 ) ( ( yy - results_crafty_wac[t] ) );
    //printf("\n%d",yy);
    t++;
  };

  return media;
}

/*
void
test_eval_diff_crafty ()
{
  //lim ((butterfly.eval(wac) - crafty.eval(wac))^2) = 0
  //0->300
  while (1)
    {
      FILE *stream;

      int inc = 1;		//get_random(100);
      char file[] = "WAC.EPD";
      char line2[2001];

      u64 migliore;
      u64 media;
      stream = fopen (file, "r");
      myassert (stream, "test error file not found");
      int t = 0;
      while (fgets (line2, sizeof (line2), stream) != NULL)
	{
	  strcpy (M.fen[t], line2);
	  if (strstr (line2, " w "))
	    M.side[t] = 1;
	  else
	    M.side[t] = 0;
	  t++;
	}
      fclose (stream);
      int X = 1;
//  u64 migiore_migliore = 0;
      //int stop = 0;
      popola_rr;
      while (1)
	{
	  migliore = diff_crafty ();
	  printf ("\n%I64u\t%s", migliore, rr);
	  for (int ww = 0; ww < ACTUAL_COUNT; ww++)
	    {
	      int do1 = 1;
	      actual[ww] += inc;
	      popola_actual;
	      media = diff_crafty ();
	      if (media < migliore)
		{
		  popola_rr;
		  printf ("\n%I64u\t%s", media, rr);
		  fflush (stdout);
		  migliore = media;
		  do1 = 1;
		}
	      else
		{
		  fprintf (stderr, ".");
		  actual[ww] -= inc;
		  popola_actual;
		  do1 = 2;
		};
	      while (do1 == 1)
		{

		  actual[ww] += inc;
		  popola_actual;
		  media = diff_crafty ();
		  if (media < migliore)
		    {
		      popola_rr;
		      printf ("\n%I64u\t%s", media, rr);
		      fflush (stdout);
		      migliore = media;
		    }
		  else
		    {
		      fprintf (stderr, ".");
		      actual[ww] -= inc;
		      popola_actual;
		      do1 = 0;
		    }
		};
	      while (do1 == 2)
		{
		  if (actual[ww] - inc < 1)
		    break;
		  actual[ww] -= inc;
		  popola_actual;
		  media = diff_crafty ();
		  if (media < migliore)
		    {
		      popola_rr;
		      //printf (".");
		      printf ("\n%I64u\t%s", media, rr);
		      fflush (stdout);
		      migliore = media;
		      do1 = 2;
		    }
		  else
		    {
		      fprintf (stderr, ".");
		      do1 = 0;
		      actual[ww] += inc;
		      popola_actual;
		    }
		}

	      if (X > 1)
		inc = get_random (10);
	      else
		inc = 1;
	    }			//for

	  printf ("\nloop %d", X++);
	}

    }
}
*/
void
test_wac (  ) {
  printf ( "\n ****** START TEST WAC *******" );
  int count = 0;
  int trovati = 0;
  FILE *stream;
  char line[2001];

  stream = fopen ( "wac.epd", "r" );
  if ( !stream )
    stream = fopen ( "../wac.epd", "r" );
  myassert ( stream, "test error file not found" );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    printf ( "\n%s", line );
    printf ( "\n%s", line );
    loadfen ( line );
    count++;
    print (  );
    do_move (  );

    if ( strstr ( test_ris, test_trovato ) ) {
      trovati++;
      printf ( "\nOK" );
    }
    else {
      printf ( "\nKO" );
    }
    if ( mate ) {
      printf ( " MATE" );
    }
    printf ( " RESULT: (%s %s) %s %d/%d", test_trovato, test_ris, line, trovati, count );

  }
  fclose ( stream );
  printf ( "\n ****** END TEST MATE *******" );
}


void
test_mate (  ) {
  printf ( "\n ****** START TEST MATE *******" );
  int count = 0;
  int trovati = 0;
  FILE *stream;
  //mate.epd generated by http://www.frayn.net/beowulf/matetest.zip

  char line[2001];
  stream = fopen ( "mate.epd", "r" );
  if ( !stream )
    stream = fopen ( "../mate.epd", "r" );
  myassert ( stream, "test error file not found" );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    printf ( "\n%s", line );
    init (  );
    loadfen ( line );
    count++;
    print (  );
    do_move (  );
    if ( mate )
      trovati++;
  }
  fclose ( stream );
  printf ( "\n***** Found: %d/%d mates", trovati, count );
  printf ( "\n ****** END TEST MATE *******" );
}


#ifdef HASH_MODE
void
zobr (  ) {
  const int tt = 90111;
  struct timeb start, end;
  int t1;

  u64 ii = 0;

  ftime ( &start );
  ii = 0;
  for ( t1 = 0; t1 < tt; t1++ )
    for ( int t = 0; t < 64; t++ )
      for ( int i = 1; i < 12; i++ ) {
	ii += shift32 ( zobrist_key[t][i] );
      }

  ftime ( &end );
  printf ( "\nshift time:  %d %I64d", diff_time ( end, start ), ii );
  ftime ( &start );
  ii = 0;
  for ( t1 = 0; t1 < tt; t1++ )
    for ( int t = 0; t < 64; t++ )
      for ( int i = 1; i < 12; i++ )
	ii += zobrist_key[t][i] >> 32;

  ftime ( &end );
  printf ( "\n<< time:  %d %I64d", diff_time ( end, start ), ii );
}
#endif
void
test (  ) {

  struct timeb start, end;

  srand ( ( unsigned ) time ( NULL ) );

  ftime ( &start );

#ifdef HASH_MODE
//      zobr();
#endif


  //test_eval_diff_crafty();  
  //test_mate ();
  test_wac (  );
  ftime ( &end );
  printf ( "\ntime:  %d num_tot_moves %I64u", diff_time ( end, start ), num_tot_moves );

/*#ifdef _MSC_VER
  Sleep (_INFINITE);
#endif
*/
}

#endif
