#include "stdafx.h"
#ifndef PERFT_MODE
#define MATCH_QUEENSIDE "O-O-O e1c1 e8c8"
#define MATCH_KINGSIDE "O-O e1g1 e8g8"
#define MATCH_QUEENSIDE_WHITE "O-O-O e1c1"
#define MATCH_KINGSIDE_WHITE "O-O e1g1"
#define MATCH_QUEENSIDE_BLACK "O-O-O e8c8"
#define MATCH_KINGSIDE_BLACK "O-O e8g8"
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
#include <signal.h>
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

void
writeWinboard ( char *msg ) {
  for ( unsigned t = 0; t < strlen ( msg ); t++ ) {
    fprintf ( stdout, "%c", msg[t] );
  };
  fprintf ( stdout, "%c", 10 );
  fflush ( stdout );
}

void
readWinboard ( char *msg ) {
  do {
    memset ( msg, 0, 255 );
    int b = 0;
    char s;
    while ( 1 ) {
      s = ( char ) getchar (  );
      if ( s == 10 ) {
	if ( force == 1 && !strcmp ( msg, "a2a3" ) )
	  memset ( msg, 0, 255 );
	break;
      }
      msg[b++] = s;
    }
  } while ( msg[0] == 0 );
}

void
use_quit ( int ) {
  if ( xboard )
    return;
  printf ( "\nuse quit to exit\n" );
  fflush ( stdout );
}

#ifdef _MSC_VER
void
listner_winboard (  )
#else
void *
listner_winboard ( void * )
#endif
{
  signal ( SIGINT, use_quit );
  int know_command, edit, side = 1;
  char move[255];
  int FEN_SIDE = 1;
  char inputstr[255];
  int go;
  u64 key;
  move[0] = '*';
  char dummy[255];
  running = 0;
  while ( 1 ) {
    readWinboard ( inputstr );
    go = 0;
    hand_do_movec = 0;
    know_command = 0;
    if ( !strcmp ( inputstr, "quit" ) ) {
      running = 0;
#ifdef _MSC_VER
      return;
#else
      return NULL;
#endif
    }
    else if ( strstr ( inputstr, "result " ) ) {
      know_command = 1;
      black_move = 0;
      running = 0;
    }
    else if ( !strcmp ( inputstr, "hard" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "random" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "otim" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "print" ) ) {
      move[0] = '*';
      know_command = 1;
      print (  );
    }
    else if ( !strcmp ( inputstr, "edit" ) ) {
      move[0] = '*';
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "c" ) ) {
      know_command = 1;
      side = 0;
    }
    else if ( !strcmp ( inputstr, "#" ) ) {
      know_command = 1;
      edit = 1;
      loadfen ( VOID_FEN );
      side = 1;
    }
    else if ( !strcmp ( inputstr, "." ) ) {
      move[0] = '*';
      know_command = 1;
      edit = 0;
    }
    else if ( !strcmp ( inputstr, "gettime" ) ) {
      know_command = 1;
    }
    else if ( strlen ( inputstr ) == 3 && edit == 1 ) {
      know_command = 1;
      if ( !side )
	inputstr[0] = ( char ) tolower ( inputstr[0] );
      char pezzo = getFenInv[inputstr[0]];
#ifdef DEBUG_MODE
      ASSERT ( pezzo != -1 );
      ASSERT ( pezzo != 12 );
#endif
      chessboard[pezzo] = Chessboard ( pezzo ) | TABLOG[decodeBoard ( inputstr + 1 )];
    }
    else if ( !strcmp ( inputstr, "easy" ) || !strcmp ( inputstr, "draw" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "xboard" ) ) {
      writeWinboard ( "" );
      xboard = 1;
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "force" ) ) {
      force = 1;
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "?" ) ) {
      running = 0;
      know_command = 1;
      move[0] = '*';
    }
    else if ( !strcmp ( inputstr, "eval" ) ) {
      int t;
      t = eval ( !black_move
#ifdef FP_MODE
		 , -_INFINITE, _INFINITE
#endif
		 , 0 );
      if ( !FEN_SIDE )
	t = -t;
      sprintf ( inputstr, "Eval: %d", t );
      writeWinboard ( inputstr );
      move[0] = '*';
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "createbook" ) ) {
      create_open_book ( BOOK_EPD );
      know_command = 1;
#ifdef _MSC_VER
      return;
#else
      return NULL;
#endif
    }
    else if ( !strcmp ( inputstr, "white" ) ) {
      black_move = 0;
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "black" ) ) {
      black_move = 1;
      know_command = 1;
    }
    else if ( !strcmp ( inputstr, "computer" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "post" ) )
      know_command = 1;
    else if ( !strcmp ( inputstr, "new" ) ) {
      know_command = 1;
      black_move = 1;
      running = 0;
      init (  );
      move[0] = '*';
      move[1] = '\0';
#ifndef TEST_MODE
      loadfen ( INITIAL_FEN );
#endif
    }
    else if ( ( ( strstr ( MATCH_QUEENSIDE_WHITE, inputstr ) || strstr ( MATCH_KINGSIDE_WHITE, inputstr ) ) && get_piece_at ( WHITE, TABLOG[E1] ) == KING_WHITE ) || ( ( strstr ( MATCH_QUEENSIDE_BLACK, inputstr ) || strstr ( MATCH_KINGSIDE_BLACK, inputstr ) ) && get_piece_at ( BLACK, TABLOG[E8] ) == KING_BLACK ) ) {
      if ( strstr ( MATCH_QUEENSIDE, inputstr ) )
	result_move.from = QUEENSIDE;
      else
	result_move.from = KINGSIDE;
      if ( strstr ( inputstr, "1" ) )
	result_move.side = WHITE;
      else
	result_move.side = BLACK;
      result_move.type = CASTLE;

      makemove ( &result_move, &key );
      know_command = 1;
      if ( !force )
	go = 1;
    }
    else if ( strstr ( inputstr, "set " ) ) {
      know_command = 1;
      black_move = 1;
      running = 0;
      init (  );
      move[0] = '*';
      move[1] = '\0';
      char d[256];
      strcpy ( d, inputstr + 4 );
      if ( !strcmp ( d, "undo" ) ) {
	hand_do_movec = 0;
      }
      else {
	loadfen ( d );
	if ( strstr ( d, " w " ) )
	  FEN_SIDE = 1;
	else
	  FEN_SIDE = 0;
      }
      print (  );
    }
    else if ( strstr ( inputstr, "sd " ) ) {
      know_command = 1;
      MAX_DEPTH_TO_SEARCH = atoi ( inputstr + 3 );
    }
    else if ( strstr ( inputstr, "level " ) ) {
      know_command = 1;
      if ( strstr ( inputstr, "level 0 9999 9999" ) )
	supplementary_mill_sec = -1;
      else {
	supplementary_mill_sec = 1000 * atoi ( strstr ( strstr ( strstr ( inputstr, " " ) + 1, " " ) + 1, " " ) + 1 );
	supplementary_mill_sec -= ( int ) ( supplementary_mill_sec * 0.1 );
      }
    }
    else if ( strstr ( inputstr, "otim " ) ) {
      know_command = 1;
      otime = atoi ( inputstr + 5 );
    }
    else if ( strstr ( inputstr, "time " ) ) {
      know_command = 1;
      Ttime = atoi ( inputstr + 5 );
      if ( supplementary_mill_sec == -1 ) {
	MAX_TIME_MILLSEC = 10000;
      }
      else {
	MAX_TIME_MILLSEC = supplementary_mill_sec + Ttime * 10 / 40;
      }
      if ( st_force < ( int ) MAX_TIME_MILLSEC )
	MAX_TIME_MILLSEC = st_force;
      if ( otime > Ttime ) {
	MAX_TIME_MILLSEC -= ( int ) ( MAX_TIME_MILLSEC * ( ( 135.0 - Ttime * 100.0 / otime ) / 100.0 ) );
      }
    }
    else if ( strstr ( inputstr, "st " ) ) {
      know_command = 1;
      st_force = MAX_TIME_MILLSEC = atoi ( inputstr + 3 ) * 1000;
    }
    else if ( strlen ( inputstr ) == 4 && ( inputstr[1] >= 48 && inputstr[1] <= 56 && inputstr[3] >= 48 && inputstr[3] <= 56 ) ) {
      move[0] = inputstr[0];
      move[1] = inputstr[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = inputstr[2];
      move[1] = inputstr[3];
      if ( get_piece_at ( WHITE, TABLOG[result_move.from] ) != SQUARE_FREE ) {
	result_move.side = WHITE;
	black_move = 1;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
      }
      result_move.to = decodeBoard ( move );
      if ( get_piece_at ( result_move.side, TABLOG[result_move.from] ) < 2 && get_column[result_move.from] != get_column[result_move.to] && get_piece_at ( 0, TABLOG[result_move.to] ) == SQUARE_FREE && get_piece_at ( 1, TABLOG[result_move.to] ) == SQUARE_FREE ) {
	result_move.type = ENPASSANT;
      }
      else
	result_move.type = STANDARD;

      print (  );
      makemove ( &result_move, &key );

      know_command = 1;
      if ( !force )
	go = 1;
    }
    else if ( strlen ( inputstr ) == 5 ) {
      result_move.type = PROMOTION;
      result_move.promotion_piece = getFenInv[inputstr[4]];
      move[0] = inputstr[0];
      move[1] = inputstr[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = inputstr[2];
      move[1] = inputstr[3];
      move[2] = 0;
      result_move.to = decodeBoard ( move );
      if ( get_piece_at ( WHITE, TABLOG[result_move.from] ) != SQUARE_FREE ) {
	result_move.promotion_piece++;
	result_move.side = WHITE;
	black_move = 1;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
      }

      makemove ( &result_move, &key );
      print (  );
      know_command = 1;
      if ( !force )
	go = 1;
    }
    else if ( strlen ( inputstr ) == 3 && ( inputstr[1] == '7' || inputstr[1] == '2' ) ) {
      result_move.type = PROMOTION;
      result_move.promotion_piece = getFenInv[inputstr[2]];
      move[0] = inputstr[0];
      move[1] = inputstr[1];
      move[2] = 0;
      result_move.from = decodeBoard ( move );
      move[0] = inputstr[2];
      move[1] = inputstr[3];
      if ( inputstr[1] == '7' ) {
	result_move.side = WHITE;
	result_move.promotion_piece++;
	black_move = 1;
	result_move.to = result_move.from + 8;
      }
      else {
	result_move.side = BLACK;
	black_move = 0;
	result_move.to = result_move.from - 8;
      }

      makemove ( &result_move, &key );
      know_command = 1;
      if ( !force )
	go = 1;
    }
    else if ( !strcmp ( inputstr, "go" ) ) {
      go = 1;
      force = 0;
      move[1] = 0;
      know_command = 1;
    };
    if ( go && !hand_do_movec ) {
      know_command = 1;
      go = 0;
#if defined  _MSC_VER	|| defined  __GNUWIN32__
      DWORD s;
      CreateThread ( NULL, 0, ( LPTHREAD_START_ROUTINE ) hand_do_move, ( LPVOID ) NULL, 0, &s );
#else
      pthread_t thread1;
      pthread_create ( &thread1, NULL, hand_do_move, NULL );

#endif
      know_command = 1;
    }
    else if ( know_command == 0 ) {
      strcat ( strcpy ( dummy, "Error (unknown command): " ), inputstr );
      writeWinboard ( dummy );
    };
  };
}
#endif
