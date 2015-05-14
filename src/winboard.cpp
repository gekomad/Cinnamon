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
  if ( strlen ( msg ) != 0 && msg[1] != 'O' )
    msg = lowercase ( msg );
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
#ifdef HASH_MODE
      free ( hash_array[BLACK] );
      free ( hash_array[WHITE] );


      free ( openbook );
#endif
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
    else if ( !strcmp ( tt, "easy" ) || !strcmp ( tt, "force" )
	      || !strcmp ( tt, "draw" ) )
      know_command = 1;
    else if ( !strcmp ( tt, "xboard" ) ) {
      writeWinboard ( "" );
      xboard = 1;
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
      move[0] = '*';
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
    else if ( strlen ( tt ) > 2 && ( strstr ( MATCH_QUEENSIDE, tt )
				     || strstr ( MATCH_KINGSIDE, tt ) ) ) {
      if ( strstr ( MATCH_QUEENSIDE, tt ) )
	result_move.da = QUEENSIDE;
      else
	result_move.da = KINGSIDE;

      result_move.side = black_move;

      result_move.tipo = CASTLE;
      makemove ( &result_move );
      print (  );
      know_command = 1;
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
    else if ( ( strlen ( tt ) == 3 || strlen ( tt ) == 4 || strlen ( tt ) == 5 )
	      && ( tt[1] >= 48 && tt[1] <= 56 && tt[3] >= 48 && tt[3] <= 56 ) ) {

      move[0] = tt[0];
      move[1] = tt[1];
      move[2] = 0;
      result_move.da = decodeBoard ( move );
      move[0] = tt[2];
      move[1] = tt[3];
      if ( get_piece_at ( WHITE, TABLOG[result_move.da] ) != SQUARE_FREE ) {
	result_move.side = WHITE;
	black_move = 1;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
      }
      result_move.a = decodeBoard ( move );
      if ( get_piece_at ( result_move.side, TABLOG[result_move.da] ) < 2 && get_column[result_move.da] != get_column[result_move.a]
	   && get_piece_at ( 0, TABLOG[result_move.a] ) == SQUARE_FREE && get_piece_at ( 1, TABLOG[result_move.a] ) == SQUARE_FREE )
	result_move.tipo = ENPASSANT;
      else
	result_move.tipo = STANDARD;
      makemove ( &result_move );
      print (  );
      push_fen (  );
      know_command = 1;
      go = 1;

    }

    else if ( !strcmp ( tt, "go" ) ) {
      go = 1;
      move[0] = '*';
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
	Sleep ( 300 );
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

/*
void
sendmove (char *res)
{
  move[0] = '\0';
  know_command = 1;
  strcat (strcpy (dummy, "move "), res);
  writeWinboard (dummy);
  if (FLG_DRAW)
    writeWinboard (DRAW);
  else if (FLG_WIN_WHITE)
    writeWinboard (WIN_WHITE);
  else if (FLG_WIN_BLACK)
    writeWinboard (WIN_BLACK);
}
*/
#endif
//#endif
