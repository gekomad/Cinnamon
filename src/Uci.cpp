#ifndef TUNE_CRAFTY_MODE
#include "Uci.h"
#include <string>
Uci::Uci (  ) {
  search = new Search (  );
  it = new IterativeDeeping ( search );
  listner (  );
}

void
Uci::setPonder ( bool b ) {
  it->setPonder ( b );
}

Uci::~Uci (  ) {
  it->stop (  );
  delete it;
  delete search;
}

int
Uci::getMove ( const string fenStr, _Tmove * move ) {
  memset ( move, 0, sizeof ( _Tmove ) );
  static const string MATCH_QUEENSIDE = "O-O-O e1c1 e8c8";
  static const string MATCH_QUEENSIDE_WHITE = "O-O-O e1c1";
  static const string MATCH_KINGSIDE_WHITE = "O-O e1g1";
  static const string MATCH_QUEENSIDE_BLACK = "O-O-O e8c8";
  static const string MATCH_KINGSIDE_BLACK = "O-O e8g8";

  if ( ( ( MATCH_QUEENSIDE_WHITE.find ( fenStr ) != string::npos || MATCH_KINGSIDE_WHITE.find ( fenStr ) != string::npos ) && search->getPieceAtWhite ( TABLOG[E1] ) == KING_WHITE )
       || ( ( MATCH_QUEENSIDE_BLACK.find ( fenStr ) != string::npos || MATCH_KINGSIDE_BLACK.find ( fenStr ) != string::npos )
	    && search->getPieceAtBlack ( TABLOG[E8] ) == KING_BLACK )
     ) {
    if ( MATCH_QUEENSIDE.find ( fenStr ) != string::npos ) {
      move->type = QUEEN_SIDE_CASTLE_MOVE_MASK;
      move->from = QUEEN_SIDE_CASTLE_MOVE_MASK;
    }
    else {
      move->from = KING_SIDE_CASTLE_MOVE_MASK;
      move->type = KING_SIDE_CASTLE_MOVE_MASK;
    }
    if ( fenStr.find ( "1" ) != string::npos ) {
      move->side = WHITE;

    }
    else if ( fenStr.find ( "8" ) != string::npos ) {
      move->side = BLACK;

    }
    else
      myassert ( 0 );
    move->from = -1;
    move->capturedPiece = SQUARE_FREE;
    return move->side;
  }
  int i;
  int f = 0;
  for ( i = 0; i < 64; i++ ) {
    if ( !fenStr.compare ( 0, 2, BOARD[i] ) ) {
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
    if ( !fenStr.compare ( 2, 2, BOARD[i] ) ) {
      f = 1;
      break;
    }
  }
  if ( !f ) {
    cout << fenStr;
    myassert ( 0 );
  }
  int to = i;
  int pieceFrom;
  if ( ( pieceFrom = search->getPieceAtWhite ( TABLOG[from] ) ) != 12 ) {
    move->side = WHITE;
  }
  else if ( ( pieceFrom = search->getPieceAtBlack ( TABLOG[from] ) ) != 12 ) {
    move->side = BLACK;
  }
  else
    myassert ( 0 );
  move->from = from;
  move->to = to;
  if ( fenStr.length (  ) == 4 ) {
    move->type = STANDARD_MOVE_MASK;
    if ( pieceFrom == PAWN_WHITE || pieceFrom == PAWN_BLACK ) {
      if ( getColumn[from] != getColumn[to] && ( ( move->side ^ 1 ) == WHITE ? search->getPieceAtWhite ( TABLOG[to] ) : search->getPieceAtBlack ( TABLOG[to] ) ) == SQUARE_FREE ) {
	move->type = ENPASSANT_MOVE_MASK;
      }
    }
  }
  else if ( fenStr.length (  ) == 5 ) {
    move->type = PROMOTION_MOVE_MASK;
    if ( move->side == WHITE )
      move->promotionPiece = getInvFen[toupper ( fenStr.at ( 4 ) )];
    else
      move->promotionPiece = getInvFen[( uchar ) fenStr.at ( 4 )];
    myassert ( move->promotionPiece != 0xFF );
  }
  if ( move->side == WHITE ) {
    move->capturedPiece = search->getPieceAtBlack ( TABLOG[move->to] );
    move->pieceFrom = search->getPieceAtWhite ( TABLOG[move->from] );
  }
  else {
    move->capturedPiece = search->getPieceAtWhite ( TABLOG[move->to] );
    move->pieceFrom = search->getPieceAtBlack ( TABLOG[move->from] );
  }
  return move->side;
}

void
Uci::getToken ( istringstream & uip, string & token ) {
  token.clear (  );
  uip >> token;

  for ( unsigned i = 0; i < token.size (  ); i++ )
    token[i] = tolower ( token[i] );
}

void
Uci::listner (  ) {
  string command;
  bool knowCommand;
  string token;
  bool stop = false;

  while ( !stop ) {
    getline ( cin, command );
    istringstream uip ( command, ios::in );
    getToken ( uip, token );
    knowCommand = false;

    if ( token == "quit" ) {
      knowCommand = true;
      search->setRunning ( 0 );
      stop = true;
    }
    else if ( token == "ponderhit" ) {	//TODO
      knowCommand = true;
      search->setMaxTimeMillsec ( 0 );
      setPonder ( false );
    }
    else if ( token == "display" ) {
      knowCommand = true;
      search->display (  );

    }
    else if ( token == "isready" ) {
      knowCommand = true;
      cout << "readyok" << endl << flush;

    }
    else if ( token == "uci" ) {
      knowCommand = true;
      search->setUci ( true );
      cout << "id name " << NAME << endl;
      cout << "id author Giuseppe Cannella" << endl;
#ifndef NO_HASH_MODE
      cout << "option name Hash type spin default 64 min 1 max 16384" << endl;
#endif
      cout << "option name Nullmove type check default true" << endl;
      cout << "option name Clear Hash type button" << endl;
      //cout << "option name Ponder type check default false"<<endl;//TODO
      cout << "uciok" << endl << flush;

    }
    else if ( token == "score" ) {
      int t;
      t = search->getScore ( search->getSide (  ), -_INFINITE, _INFINITE );
      if ( !search->getSide (  ) )
	t = -t;
      cout << "Score: " << t << endl << flush;

      knowCommand = true;

    }
    else if ( token == "stop" ) {

      knowCommand = true;
      setPonder ( false );
      search->setRunning ( 0 );
    }
    else if ( token == "ucinewgame" ) {
      search->loadFen (  );
//            search->resetRepetitionCount();
//            search->pushStackMove(search->makeZobristKey(),false);
      knowCommand = true;

    }
    else if ( token == "setoption" ) {
      getToken ( uip, token );
      if ( token == "name" ) {
	getToken ( uip, token );
	if ( token == "hash" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    knowCommand = true;
	    search->setHashSize ( atoi ( token.c_str (  ) ) );
	  }
	}
	else if ( token == "nullmove" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    knowCommand = true;
	    search->setNullMove ( token == "true" ? true : false );
	  }
	}
	else if ( token == "ponder" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    if ( token == "true" )
	      it->enablePonder ( true );
	    else
	      it->enablePonder ( false );
	    knowCommand = true;
	  }
	}
	else if ( token == "clear" ) {
	  getToken ( uip, token );
	  if ( token == "hash" ) {
	    knowCommand = true;
	    search->clearHash (  );
	  }
	}
      }

    }
    else if ( token == "position" ) {
      knowCommand = true;
//            search->resetRepetitionCount();
      getToken ( uip, token );
      _Tmove move;
      if ( token == "startpos" ) {
	search->loadFen (  );
	getToken ( uip, token );
      }

      if ( token == "fen" ) {
	string fen;
	while ( token != "moves" && !uip.eof (  ) ) {
	  uip >> token;
	  fen += token;
	  fen += ' ';
	}
	search->init (  );
	search->setSide ( search->loadFen ( fen ) );
//                search->pushStackMove(search->makeZobristKey(),false);

      }
      if ( token == "moves" ) {
	while ( !uip.eof (  ) ) {
	  uip >> token;
	  search->setSide ( !getMove ( token, &move ) );
	  //   if(move.pieceFrom==WHITE||move.pieceFrom==BLACK||move.capturedPiece!=SQUARE_FREE)
	  //       search->resetRepetitionCount();
	  u64 dummy;
	  search->makemove ( &move, &dummy );
	  // search->pushStackMove(search->makeZobristKey(),false);
	}
      }
    }
    else if ( token == "go" ) {
      int wtime = 200000;	//5 min
      int btime = 200000;
      int winc = 0;
      int binc = 0;
      setPonder ( false );
      search->setMaxTimeMillsec ( 0 );
      while ( !uip.eof (  ) ) {
	getToken ( uip, token );
	if ( token == "wtime" )
	  uip >> wtime;
	else if ( token == "btime" )
	  uip >> btime;
	else if ( token == "winc" )
	  uip >> winc;
	else if ( token == "binc" )
	  uip >> binc;
	else if ( token == "infinite" )
	  search->setMaxTimeMillsec ( 0x7FFFFFFF );
	else if ( token == "ponder" ) {
	  search->setMaxTimeMillsec ( 0x7FFFFFFF );
	  setPonder ( true );
	}
      }
      if ( search->getMaxTimeMillsec (  ) != 0x7FFFFFFF ) {
	if ( search->getSide (  ) == WHITE ) {
	  winc -= ( int ) ( winc * 0.1 );
	  search->setMaxTimeMillsec ( winc + wtime / 40 );
	  if ( btime > wtime ) {
	    search->setMaxTimeMillsec ( search->getMaxTimeMillsec (  ) - ( int ) ( search->getMaxTimeMillsec (  ) * ( ( 135.0 - wtime * 100.0 / btime ) / 100.0 ) ) );
	  }
	}
	else {
	  binc -= ( int ) ( binc * 0.1 );
	  search->setMaxTimeMillsec ( binc + btime / 40 );
	  if ( wtime > btime ) {
	    search->setMaxTimeMillsec ( search->getMaxTimeMillsec (  ) - ( int ) ( search->getMaxTimeMillsec (  ) * ( ( 135.0 - btime * 100.0 / wtime ) / 100.0 ) ) );
	  }
	}
      }
      if ( !search->getUci (  ) )
	search->display (  );
      it->start (  );
      knowCommand = true;
    }
    if ( !knowCommand ) {
      cout << "Unknown command: " << command << endl << flush;
    };

  }
}
#endif
