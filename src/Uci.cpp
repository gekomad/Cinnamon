#ifndef TEST_MODE
#include "Uci.h"

Uci::Uci ( char *iniFile ) {
  search = new Search ( INITIAL_FEN );
  it = new IterativeDeeping ( search );
  listner (  );
}

Uci::~Uci (  ) {
  delete it;
  delete search;
}

int
Uci::moveFen ( const char *fenStr ) {
  if ( strlen ( fenStr ) != 4 && strlen ( fenStr ) != 5 ) {
    cout << fenStr;
    myassert ( 0 );
  }
  Tmove move;
  //castle//////////
  const char *MATCH_QUEENSIDE = "O-O-O e1c1 e8c8";
  const char *MATCH_QUEENSIDE_WHITE = "O-O-O e1c1";
  const char *MATCH_KINGSIDE_WHITE = "O-O e1g1";
  const char *MATCH_QUEENSIDE_BLACK = "O-O-O e8c8";
  const char *MATCH_KINGSIDE_BLACK = "O-O e8g8";

  if ( ( ( strstr ( MATCH_QUEENSIDE_WHITE, fenStr ) || strstr ( MATCH_KINGSIDE_WHITE, fenStr ) ) && search->get_piece_at ( WHITE, TABLOG[E1] ) == KING_WHITE )
       || ( ( strstr ( MATCH_QUEENSIDE_BLACK, fenStr ) || strstr ( MATCH_KINGSIDE_BLACK, fenStr ) ) && search->get_piece_at ( BLACK, TABLOG[E8] )
	    == KING_BLACK ) ) {

    if ( strstr ( MATCH_QUEENSIDE, fenStr ) ) {
      move.type = QUEEN_SIDE_CASTLE_MOVE_MASK;
      move.from = QUEEN_SIDE_CASTLE_MOVE_MASK;
    }
    else {
      move.from = KING_SIDE_CASTLE_MOVE_MASK;
      move.type = KING_SIDE_CASTLE_MOVE_MASK;
    }
    if ( strstr ( fenStr, "1" ) ) {
      move.side = WHITE;

    }
    else if ( strstr ( fenStr, "8" ) ) {
      move.side = BLACK;

    }
    else
      myassert ( 0 );
    search->makemove ( &move );
    return move.side;
  }
  int i;
  int f = 0;
  for ( i = 0; i < 64; i++ ) {
    if ( !strncmp ( BOARD[i], fenStr, 2 ) ) {
      f = 1;
      break;
    }
  }
  if ( !f ) {
    cout << fenStr;
    myassert ( 0 );
  }
  f = 0;
  int from = i;
  for ( i = 0; i < 64; i++ ) {
    if ( !strncmp ( BOARD[i], fenStr + 2, 2 ) ) {
      f = 1;
      break;
    }
  }
  if ( !f ) {
    cout << fenStr;
    myassert ( 0 );
  }
  int to = i;
  int piece_from;
  if ( ( piece_from = search->get_piece_at ( WHITE, TABLOG[from] ) ) != 12 ) {
    move.side = WHITE;
  }
  else if ( ( piece_from = search->get_piece_at ( BLACK, TABLOG[from] ) ) != 12 ) {
    move.side = BLACK;
  }
  else
    myassert ( 0 );
  move.from = from;
  move.to = to;
  if ( strlen ( fenStr ) == 4 ) {
    move.type = STANDARD_MOVE_MASK;
    if ( piece_from == PAWN_WHITE || piece_from == PAWN_BLACK ) {
      if ( get_column[from] != get_column[to] && search->get_piece_at ( move.side ^ 1, TABLOG[to] ) == 12 ) {
	move.type = ENPASSANT_MOVE_MASK;
      }
    }
  }
  else if ( strlen ( fenStr ) == 5 ) {
    move.type = PROMOTION_MOVE_MASK;
    if ( move.side == WHITE )
      move.promotion_piece = search->getInvFen ( toupper ( fenStr[4] ) );
    else
      move.promotion_piece = search->getInvFen ( fenStr[4] );
    myassert ( move.promotion_piece != -1 );
  }
  search->makemove ( &move );
  return move.side;
}

void
Uci::listner (  ) {
  string command;
  int know_command;
  string token;
  while ( 1 ) {
    getline ( cin, command );
    istringstream uip ( command, ios::in );
    uip >> token;
    know_command = 0;
    if ( token == "quit" ) {
      search->setRunning ( 0 );
      sleep ( 0.5 );
      return;
    }
    else if ( token == "display" ) {
      know_command = 1;
      search->print (  );
    }
    else if ( token == "isready" ) {
      know_command = 1;
      cout << "readyok\n" << flush;
    }
    else if ( token == "uci" ) {
      know_command = 1;
      search->setUci (  );
      cout << "id name butterfly 0.5\n";
      cout << "id author Giuseppe Cannella\n";
      cout << "uciok\n" << flush;

    }
    else if ( token == "score" ) {
      int t;
      t = search->getScore ( !search->isBlackMove (  )
#ifdef FP_MODE
			     , -_INFINITE, _INFINITE
#endif
	 );
      if ( search->isBlackMove (  ) )
	t = -t;
      cout << "Score: " << t << endl << flush;

      know_command = 1;
    }
    else if ( token == "stop" ) {
      search->setRunning ( 0 );
      know_command = 1;
    }
    else if ( token == "ucinewgame" ) {
      search->loadFen ( INITIAL_FEN );
      search->setBlackMove ( false );
      know_command = 1;
    }
    else if ( token == "position" ) {
      know_command = 1;
      uip >> token;
      if ( token == "startpos" ) {
	search->loadFen ( INITIAL_FEN );
	search->setBlackMove ( false );
	uip >> token;
      }
      if ( token == "fen" ) {
	string fen;
	while ( token != "moves" && !uip.eof (  ) ) {
	  uip >> token;
	  fen += token;
	  fen += ' ';
	}
	search->init (  );
	search->setBlackMove ( search->loadFen ( ( char * ) fen.c_str (  ) ) == BLACK ? true : false );
      }
      if ( token == "moves" ) {
	while ( !uip.eof (  ) ) {
	  uip >> token;
	  search->setBlackMove ( moveFen ( token.c_str (  ) ) == BLACK ? false : true );

	}
      }
    }
    else if ( token == "go" ) {
      int wtime = 200000;	//5 min
      int btime = 200000;
      int winc = 0;
      int binc = 0;
      while ( !uip.eof (  ) ) {
	uip >> token;
	if ( token == "wtime" )
	  uip >> wtime;
	else if ( token == "btime" )
	  uip >> btime;
	else if ( token == "winc" )
	  uip >> winc;
	else if ( token == "binc" )
	  uip >> binc;
	else if ( token == "infinite" ) {
	  search->setMaxTimeMillsec ( _INFINITE );
	}
      }
      if ( search->getMaxTimeMillsec (  ) != _INFINITE ) {
	if ( search->isBlackMove (  ) == false ) {
	  winc -= ( int ) ( winc * 0.1 );
	  search->setMaxTimeMillsec ( winc + wtime / 40 );
	  if ( btime > wtime ) {
	    search->setMaxTimeMillsec ( search->getMaxTimeMillsec (  ) - ( int ) ( search->getMaxTimeMillsec (  ) * ( ( 135.0 - wtime * 100.0 / btime )
														      / 100.0 ) ) );
	  }
	}
	else {
	  binc -= ( int ) ( binc * 0.1 );
	  search->setMaxTimeMillsec ( binc + btime / 40 );
	  if ( wtime > btime ) {
	    search->setMaxTimeMillsec ( search->getMaxTimeMillsec (  ) - ( int ) ( search->getMaxTimeMillsec (  ) * ( ( 135.0 - btime * 100.0 / wtime )
														      / 100.0 ) ) );
	  }
	}
      }
      if ( !search->getUci (  ) )
	search->print ( "==========" );
      it->start (  );
      know_command = 1;
    }
    if ( !know_command ) {
      cout << "Unknown command: " << command << endl << flush;
    };
  }
}
#endif
