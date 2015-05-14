#include "Uci.h"

Uci::Uci (  ) {
  it = new IterativeDeeping (  );
  listner (  );
}

Uci::~Uci (  ) {
  delete it;
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
  int lastTime = 0;
  uciMode = false;
  while ( !stop ) {
    getline ( cin, command );
    istringstream uip ( command, ios::in );
    getToken ( uip, token );
    knowCommand = false;

    if ( token == "perft" ) {
      int perftDepth = -1;
      int nCpu = 1;
      int PERFT_HASH_SIZE = 0;
      string fen;
      getToken ( uip, token );
      while ( !uip.eof (  ) ) {
	if ( token == "depth" ) {
	  getToken ( uip, token );
	  perftDepth = atoi ( token.c_str (  ) );
	  if ( perftDepth > GenMoves::MAX_PLY || perftDepth <= 0 )
	    perftDepth = 2;
	  getToken ( uip, token );
	}
	else if ( token == "ncpu" ) {
	  getToken ( uip, token );
	  nCpu = atoi ( token.c_str (  ) );
	  if ( nCpu > 32 || nCpu <= 0 )
	    nCpu = 1;
	  getToken ( uip, token );
	}
	else if ( token == "hash_size" ) {
	  getToken ( uip, token );
	  PERFT_HASH_SIZE = atoi ( token.c_str (  ) );
	  if ( PERFT_HASH_SIZE > 32768 || PERFT_HASH_SIZE < 0 )
	    PERFT_HASH_SIZE = 64;
	  getToken ( uip, token );
	}
	else if ( token == "fen" ) {
	  uip >> token;
	  do {
	    fen += token;
	    fen += ' ';
	    uip >> token;
	  } while ( token != "ncpu" && token != "hash_size" && token != "depth" && !uip.eof (  ) );
	}
	else
	  break;
      }
      if ( perftDepth != -1 ) {
	int hashSize = it->getHashSize (  );
	it->setHashSize ( 0 );
	if ( fen.empty (  ) )
	  fen = it->getFen (  );
	cout << "perft depth " << perftDepth << " nCpu " << nCpu << " hash_size " << PERFT_HASH_SIZE << " fen " << fen << endl;
	Perft *p = new Perft ( fen, perftDepth, nCpu, PERFT_HASH_SIZE );
	delete p;
	it->setHashSize ( hashSize );
      }
      else
	cout << "use: perft depth d [nCpu n] [hash_size mb] [fen fen_string]" << endl;
      knowCommand = true;
    }
    else if ( token == "quit" ) {
      knowCommand = true;
      it->setRunning ( false );
      stop = true;
    }
    else if ( token == "ponderhit" ) {
      knowCommand = true;
      it->startClock (  );
      it->setMaxTimeMillsec ( lastTime - lastTime / 3 );
      it->setPonder ( false );
    }
    else if ( token == "display" ) {
      knowCommand = true;
      it->display (  );
    }
    else if ( token == "isready" ) {
      knowCommand = true;
      it->setRunning ( 0 );
      cout << "readyok" << endl;
    }
    else if ( token == "uci" ) {
      knowCommand = true;
      uciMode = true;
      cout << "id name " << NAME << endl;
      cout << "id author Giuseppe Cannella" << endl;
      cout << "option name Hash type spin default 64 min 1 max 16384" << endl;
      cout << "option name Nullmove type check default true" << endl;
      cout << "option name Clear Hash type button" << endl;
      cout << "option name MakeBook type button" << endl;
      if ( it->getUseBook (  ) )
	cout << "option name OwnBook type check default true" << endl;	//TODO rispondere a http://immortalchess.net/forum/showthread.php?p=346514&langid=1
      else
	cout << "option name OwnBook type check default false" << endl;
      if ( it->getPonderEnabled (  ) )
	cout << "option name Ponder type check default true" << endl;
      else
	cout << "option name Ponder type check default false" << endl;
      cout << "uciok" << endl;
    }
    else if ( token == "score" ) {
      int t = it->getScore ( it->getSide (  ), -_INFINITE, _INFINITE );
      if ( !it->getSide (  ) ) {
	t = -t;
      }
      cout << "Score: " << t << endl;
      knowCommand = true;
    }
    else if ( token == "stop" ) {
      knowCommand = true;
      it->setPonder ( false );
      it->setRunning ( 0 );
    }
    else if ( token == "ucinewgame" ) {
      it->loadFen (  );
      it->clearMovesPath (  );
      it->clearHash (  );
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
	    it->setHashSize ( atoi ( token.c_str (  ) ) );
	  }
	}
	else if ( token == "nullmove" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    knowCommand = true;
	    it->setNullMove ( token == "true" ? true : false );
	  }
	}
	else if ( token == "ownbook" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    it->setUseBook ( token == "true" ? true : false );
	    knowCommand = true;
	  }
	}
	else if ( token == "ponder" ) {
	  getToken ( uip, token );
	  if ( token == "value" ) {
	    getToken ( uip, token );
	    it->enablePonder ( token == "true" ? true : false );
	    knowCommand = true;
	  }
	}
	else if ( token == "makebook" ) {
	  bool useBook = it->getUseBook (  );
	  it->setUseBook ( false );
	  OpenBook *b = new OpenBook (  );
	  cout << "create book..." << flush;
	  if ( b->create (  ) )
	    cout << " ok" << endl;
	  delete b;
	  it->setUseBook ( useBook );
	  knowCommand = true;
	}
	else if ( token == "clear" ) {
	  getToken ( uip, token );
	  if ( token == "hash" ) {
	    knowCommand = true;
	    it->clearHash (  );
	  }
	}
      }
    }
    else if ( token == "position" ) {
      it->lock (  );
      knowCommand = true;
      it->setRepetitionMapCount ( 0 );
      getToken ( uip, token );
      _Tmove move;
      it->setFollowBook ( false );
      if ( token == "startpos" ) {
	it->clearMovesPath (  );
	it->setUseBook ( it->getUseBook (  ) );
	it->loadFen (  );
	getToken ( uip, token );
      }
      if ( token == "fen" ) {
	string fen;
	while ( token != "moves" && !uip.eof (  ) ) {
	  uip >> token;
	  fen += token;
	  fen += ' ';
	}
	it->init (  );
	it->setSide ( it->loadFen ( fen ) );
	it->pushStackMove (  );
      }
      if ( token == "moves" ) {
//                it->clearMovesPath();
	while ( !uip.eof (  ) ) {
	  uip >> token;
	  it->setSide ( !it->getMoveFromSan ( token, &move ) );
	  it->pushMovesPath ( it->decodeBoard ( token.substr ( 0, 2 ) ) );
	  it->pushMovesPath ( it->decodeBoard ( token.substr ( 2, 2 ) ) );
	  it->makemove ( &move );
	}
      }
      it->unLock (  );
    }
    else if ( token == "go" ) {
      it->setMaxDepth ( GenMoves::MAX_PLY );
      int wtime = 200000;	//5 min
      int btime = 200000;
      int winc = 0;
      int binc = 0;
      bool forceTime = false;
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
	else if ( token == "depth" ) {
	  int depth;
	  uip >> depth;
	  if ( depth > GenMoves::MAX_PLY )
	    depth = GenMoves::MAX_PLY;
	  it->setMaxDepth ( depth );
	  it->setMaxTimeMillsec ( 0x7FFFFFFF );
	  forceTime = true;
	}
	else if ( token == "movetime" ) {
	  int tim;
	  uip >> tim;
	  it->setMaxTimeMillsec ( tim );
	  forceTime = true;
	}
	else if ( token == "infinite" ) {
	  it->setMaxTimeMillsec ( 0x7FFFFFFF );
	  forceTime = true;
	}
	else if ( token == "ponder" ) {
	  it->setPonder ( true );
	}
      }
      if ( !forceTime ) {
	if ( it->getSide (  ) == WHITE ) {
	  winc -= ( int ) ( winc * 0.1 );
	  it->setMaxTimeMillsec ( winc + wtime / 40 );
	  if ( btime > wtime ) {
	    it->setMaxTimeMillsec ( it->getMaxTimeMillsec (  ) - ( int ) ( it->getMaxTimeMillsec (  ) * ( ( 135.0 - wtime * 100.0 / btime ) / 100.0 ) ) );
	  }
	}
	else {
	  binc -= ( int ) ( binc * 0.1 );
	  it->setMaxTimeMillsec ( binc + btime / 40 );
	  if ( wtime > btime ) {
	    it->setMaxTimeMillsec ( it->getMaxTimeMillsec (  ) - ( int ) ( it->getMaxTimeMillsec (  ) * ( ( 135.0 - btime * 100.0 / wtime ) / 100.0 ) ) );
	  }
	}
	lastTime = it->getMaxTimeMillsec (  );
      }
      if ( !uciMode )
	it->display (  );
      it->stop (  );
      it->start (  );
      knowCommand = true;
    }
    if ( !knowCommand ) {
      cout << "Unknown command: " << command << endl;
    };
    cout << flush;
  }
}
