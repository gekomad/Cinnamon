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


#include "stdafx.h"
#ifndef PERFT_MODE


#define MATCH_QUEENSIDE "O-O-O e1c1 e8c8"
#define MATCH_KINGSIDE "O-O e1g1 e8g8"

#include "maindefine.h"

#include "gen.h"
#include "butterfly.h"
#include "search.h"
#include "test.h"


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "extern.h"
#include "maindefine.h"
#include "eval.h"
#include "winboard.h"
#include "search.h"

#ifdef _MSC_VER
#include <windows.h>
#include <conio.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif
#include <stdlib.h>

#include "gen.h"
#include "utility.h"
#include "zobrist.h"





int know_command, edit, side = 1;

char move[255];
char *dummy;


void
writeWinboard ( char *msg ) {
  //if ( strlen ( msg ) != 0 && msg[1] != 'O' )
  // msg = lowercase ( msg );
  for ( unsigned t = 0; t < strlen ( msg ); t++ ) {
    fprintf ( stdout, "%c", msg[t] );
  };
  fprintf ( stdout, "%c", 10 );
  fflush ( stdout );
  //debug (msg);
};

void
readWinboard ( char *tt ) {

  memset ( tt, 0, 255 );
  int b = 0;
  char s;
  while ( 1 ) {
    s = ( char ) getchar (  );
    if ( s == 10 )
      break;
    tt[b++] = s;
  }
  //debug (tt);
};

#ifdef _MSC_VER
void
listner_winboard (  )
#else
void *
listner_winboard ( void *uuua )
#endif
{

  char tt[255];
  int go;


  move[0] = '*';
  dummy = ( char * ) calloc ( 1, 255 );
  run = 0;

  while ( 1 ) {
    readWinboard ( tt );
    go = 0;
    hand_do_movec = 0;
    know_command = 0;
    if ( !strcmp ( tt, "quit" ) ) {
      run = 0;
      dispose (  );
      free ( dummy );
      exit ( 0 );
    }
    else if ( !strcmp ( tt, "result" ) ) {
      know_command = 1;

      black_move = 0;
      run = 0;
    }
    else if ( !strcmp ( tt, "hard" ) )
      know_command = 1;
    else if ( !strcmp ( tt, "otim" ) )
      know_command = 1;
    else if ( !strcmp ( tt, "print" ) ) {
      move[0] = '*';
      know_command = 1;
      print (  );
    }
    else if ( !strcmp ( tt, "edit" ) ) {
      move[0] = '*';
      know_command = 1;
    }
    else if ( !strcmp ( tt, "c" ) ) {
      know_command = 1;
      side = 0;
    }
    else if ( !strcmp ( tt, "#" ) ) {
      know_command = 1;
      edit = 1;
      loadfen ( VOID_FEN );
      side = 1;
    }
    else if ( !strcmp ( tt, "." ) ) {
      move[0] = '*';
      know_command = 1;
      edit = 0;
    }
    else if ( !strcmp ( tt, "gettime" ) ) {

      know_command = 1;
      struct timeb t_current;
      ftime ( &t_current );
      printf ( "\nt_current.time %I64u", t_current.time );
      //printf("\nt_current.millitm %I64u",t_current.millitm);
      printf ( "\nstart_time.time %I64u", start_time.time );

      int i = ( int ) ( 1000 * ( t_current.time - start_time.time ) );
      printf ( "\ndiff %d", i );
      printf ( "\nMAX_TIME_MILLSEC %d", MAX_TIME_MILLSEC );

      fflush ( stdout );
    }
    else if ( strlen ( tt ) == 3 && edit == 1 ) {
      know_command = 1;
      if ( !side )
	tt[0] = ( char ) tolower ( tt[0] );
      char pezzo = getFenInv ( tt[0] );
#ifdef DEBUG_MODE
      assert ( pezzo != -1 );
      assert ( pezzo != 12 );
#endif
      chessboard[pezzo] = chessboard[pezzo] | TABLOG[decodeBoard ( tt + 1 )];
    }
    else if ( !strcmp ( tt, "easy" ) || !strcmp ( tt, "draw" ) )
      know_command = 1;
    else if ( !strcmp ( tt, "xboard" ) ) {
      writeWinboard ( "" );
      xboard = 1;
      know_command = 1;
    }
    else if ( !strcmp ( tt, "force" ) ) {
      force = 1;
      know_command = 1;
    }
    else if ( !strcmp ( tt, "?" ) ) {
      run = 0;
      know_command = 1;
      move[0] = '*';
    }

    else if ( !strcmp ( tt, "eval" ) ) {


      int t;
      t = eval ( !black_move
#ifdef FP_MODE
		 , _INFINITE, 0
#endif
		 , 0 );
      sprintf ( tt, "Eval: %d", t );
      writeWinboard ( tt );
      move[0] = '*';
      know_command = 1;


      writeWinboard ( tt );

      know_command = 1;

    }

    else if ( !strcmp ( tt, "createbook" ) ) {
      create_open_book ( BOOK_EPD );
      know_command = 1;
    }

    else if ( !strcmp ( tt, "white" ) ) {
      black_move = 0;
      know_command = 1;
    }

    else if ( !strcmp ( tt, "black" ) ) {
      black_move = 1;
      know_command = 1;
    }
    else if ( !strcmp ( tt, "computer" ) )
      know_command = 1;

    else if ( !strcmp ( tt, "post" ) )
      know_command = 1;
    else if ( !strcmp ( tt, "new" ) ) {

      know_command = 1;
      black_move = 1;
      run = 0;
      FLG_WIN_WHITE = 0;
      FLG_WIN_BLACK = 0;
      FLG_DRAW = 0;
      init (  );
      move[0] = '*';
      move[1] = '\0';
#ifndef TEST_MODE
      loadfen ( INITIAL_FEN );
#endif
    }
    else if ( strlen ( tt ) > 2 && ( strstr ( MATCH_QUEENSIDE, tt ) || strstr ( MATCH_KINGSIDE, tt ) ) ) {
      if ( strstr ( MATCH_QUEENSIDE, tt ) )
	result_move.from = QUEENSIDE;
      else
	result_move.from = KINGSIDE;
      if ( strstr ( tt, "1" ) )
	result_move.side = WHITE;
      else
	result_move.side = BLACK;

      result_move.type = CASTLE;
      makemove ( &result_move );
      print (  );
      know_command = 1;
      if ( !force )
	go = 1;

    }
    else if ( strstr ( tt, "set " ) ) {

      know_command = 1;
      black_move = 1;
      run = 0;
      FLG_WIN_WHITE = 0;
      FLG_WIN_BLACK = 0;
      FLG_DRAW = 0;
      init (  );
      move[0] = '*';
      move[1] = '\0';
      char d[256];
      strcpy ( d, tt + 4 );
      if ( !strcmp ( d, "undo" ) ) {
	pop_fen (  );
	hand_do_movec = 0;
      }

      else {
	free_fen_stack (  );
	loadfen ( d );
	push_fen (  );
      }
      print (  );
    }
    else if ( strstr ( tt, "sd " ) ) {
      know_command = 1;
      MAX_DEPTH_TO_SEARCH = atoi ( tt + 3 );
    }
    else if ( strstr ( tt, "level " ) ) {
      know_command = 1;
      if ( strstr ( tt, "level 0 9999 9999" ) )
	supplementary_mill_sec = -1;
      else {
	supplementary_mill_sec = 1000 * atoi ( strstr ( strstr ( strstr ( tt, " " ) + 1, " " ) + 1, " " ) + 1 );
	supplementary_mill_sec -= ( int ) ( supplementary_mill_sec * 0.1 );
      }
    }
    else if ( strstr ( tt, "time " ) ) {
      know_command = 1;
      if ( supplementary_mill_sec == -1 ) {
	MAX_TIME_MILLSEC = 10000;
      }
      else {
	//MAX_TIME_MILLSEC = MAX_TIME_MILLSEC < supplementary_mill_sec + atoi ( tt + 5 ) * 10 / 30 ? MAX_TIME_MILLSEC : supplementary_mill_sec + atoi ( tt + 5 ) * 10 / 30;   
	MAX_TIME_MILLSEC = supplementary_mill_sec + atoi ( tt + 5 ) * 10 / 30;
      }
      if ( st_force < ( int ) MAX_TIME_MILLSEC )
	MAX_TIME_MILLSEC = st_force;


    }
    else if ( strstr ( tt, "st " ) ) {
      know_command = 1;
      st_force = MAX_TIME_MILLSEC = atoi ( tt + 3 ) * 1000;
    }
    else if ( strlen ( tt ) == 4 && ( tt[1] >= 48 && tt[1] <= 56 && tt[3] >= 48 && tt[3] <= 56 )
       ) {
      move[0] = tt[0];
      move[1] = tt[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = tt[2];
      move[1] = tt[3];
      if ( get_piece_at ( WHITE, TABLOG[result_move.from] ) != SQUARE_FREE ) {
	result_move.side = WHITE;
	black_move = 1;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
      }
      result_move.to = decodeBoard ( move );
      if ( !force && get_piece_at ( result_move.side, TABLOG[result_move.from] ) < 2 && get_column[result_move.from] != get_column[result_move.to] && get_piece_at ( 0, TABLOG[result_move.to] ) == SQUARE_FREE && get_piece_at ( 1, TABLOG[result_move.to] ) == SQUARE_FREE )
	result_move.type = ENPASSANT;

      else
	result_move.type = STANDARD;
      makemove ( &result_move );
      print (  );
      push_fen (  );
      know_command = 1;
      if ( !force )
	go = 1;

    }
    else if ( strlen ( tt ) == 5 ) {
      result_move.type = PROMOTION;
      result_move.promotion_piece = getFenInv ( tt[4] );
      move[0] = tt[0];
      move[1] = tt[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = tt[2];
      move[1] = tt[3];
      move[2] = 0;
      result_move.to = decodeBoard ( move );
      if ( tt[3] == '8' ) {
	result_move.side = WHITE;
	black_move = 1;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
      }

      makemove ( &result_move );
      print (  );
      push_fen (  );
      know_command = 1;
      if ( !force )
	go = 1;

    }
    else if ( strlen ( tt ) == 3 && ( tt[1] == '7' || tt[1] == '2' ) ) {
      result_move.type = PROMOTION;
      result_move.promotion_piece = getFenInv ( tt[2] );
      move[0] = tt[0];
      move[1] = tt[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = tt[2];
      move[1] = tt[3];
      if ( tt[1] == '7' ) {
	result_move.side = WHITE;
	black_move = 1;
	result_move.to = result_move.from + 8;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
	result_move.to = result_move.from - 8;
      }

      makemove ( &result_move );
      print (  );
      push_fen (  );
      know_command = 1;
      if ( !force )
	go = 1;

    }
    else if ( !strcmp ( tt, "go" ) ) {
      go = 1;
      force = 0;
      move[1] = 0;
      know_command = 1;
    };
    //if (!hand_do_movec && go == 1 && strlen (move) > 0 || move[0] != '*')
    if ( go && !hand_do_movec ) {

      know_command = 1;
      go = 0;


#if defined  _MSC_VER	|| defined  __GNUWIN32__
      DWORD s;
      CreateThread ( NULL, 0, ( LPTHREAD_START_ROUTINE ) hand_do_move, ( LPVOID ) NULL, 0, &s );
      while ( !hand_do_movec )
	Sleep ( 400 );
#else
      pthread_t thread1;
      int iret1;
      char *message1 = "Thread 1";
      iret1 = pthread_create ( &thread1, NULL, hand_do_move, ( void * ) &message1 );
      while ( !hand_do_movec )
	usleep ( 30000 );
#endif

      know_command = 1;

    }
    else if ( know_command == 0 ) {
      strcat ( strcpy ( dummy, "Error (unknown command): " ), tt );
      writeWinboard ( dummy );
    };
  };

};


#endif
	//#endif
