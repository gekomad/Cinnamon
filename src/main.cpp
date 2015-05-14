#include <iostream>
#include "maindefine.h"
#include "Perft.h"
#include "Uci.h"
#include "GenMoves.h"
#include "TuningCrafty.h"
#include <unistd.h>

void
dispose (  ) {
  free ( bits::ROTATE_LEFT );
  free ( bits::ROTATE_RIGHT );
}

void
start ( string exe ) {
  exe = file::extractFileName ( exe );
  cout << NAME;
#if UINTPTR_MAX == 0xffffffffffffffff
  cout << " (64 bits)";
#else
  cout << " (32 bits)";
#endif
  cout << " UCI by Giuseppe Cannella" << endl;
  cout << "Last compiled " << __DATE__ << " " << __TIME__ << " with gcc " << __VERSION__ << endl;
  cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << endl;
#ifndef TUNE_CRAFTY_MODE
  cout << "use '" << exe << " -perft [-d depth] [-c nCpu] [-h hash size (Mb)] [-f fen position]' to run perft test" << endl << endl;
#else
  cout << "Tune crafty mode" << endl;
#endif
  srand ( time ( NULL ) );
#ifdef DEBUG_MODE
  cout << "DEBUG_MODE\n";
#endif
#ifdef NO_FP_MODE
  cout << "NO FP_MODE\n";
#endif
#ifdef NO_HASH_MODE
  cout << "NO HASH_MODE\n";
#endif
  bits::initBit (  );
}

#ifdef TUNE_CRAFTY_MODE
int
main ( int argc, char **argv ) {

  char opt;
  string crafty, epd;
  while ( ( opt = getopt ( argc, argv, "c:e:" ) ) != -1 ) {
    switch ( opt ) {
    case 'c':			//crafty path
      crafty = optarg;
      break;
    case 'e':			//epd file path
      epd = optarg;
      break;
    }
  }

  if ( crafty.empty (  ) || epd.empty (  ) ) {
    cout << "use: " << argv[0] << " -c crafty_exe -e epd_file" << endl;
    return 1;
  }
  start ( argv[0] );
  TuningCrafty t ( crafty );
  t.evalCraftyTuning ( epd );
  dispose (  );
  return 0;
}
#else

int
main ( int argc, char **argv ) {
  start ( argv[0] );
  char opt;
  int nCpu = 1;
  int perftDepth = 1;
  string fen;
  int PERFT_HASH_SIZE = 0;
  bool perftMode = false;
  vector < string > benchmarkPos = {
  "r1bq1rk1/1p2bppp/p1n1pn2/6B1/2BP4/P1N2N2/1P3PPP/R2Q1RK1 w - -",
      "1rbq1rk1/1p2npbp/p1np2p1/2p1p3/2P5/2NPN1P1/PP2PPBP/R1BQ1RK1 w - -", "rnbq1rk1/4bppp/p2p4/1p1np3/4P3/2N1BN2/PPP1QPPP/R3K2R w KQ -", "r1b1k2r/1pq2ppp/p1n1pn2/2b5/8/1BN1PN2/PP2QPPP/R1B2RK1 w kq -", "r1bq1rk1/pp1nnppp/4p3/8/3N4/2N1P3/PPQ2PPP/R3KB1R w KQ -", "rn1q1rk1/pb2bppp/5n2/2pp4/8/2N2NP1/PP2PPBP/R1BQR1K1 w - -", "rn2kb1r/1pqn1ppp/p2pb3/6P1/4Pp2/1NN5/PPP1B2P/R1BQK2R w KQkq -", "rnbqk2r/1p2ppb1/p2p3p/4n1p1/3NP3/2N3B1/PPP1BPPP/R2QK2R w KQkq -", "r1b2rk1/ppq2pbp/2pp1np1/P3n3/4P3/2N1BN1P/1PP1BPP1/R2QK2R w KQ -", "r2q1rk1/pb1n1ppp/2pbpn2/1p6/3P4/P1NBPN2/1P3PPP/R1BQ1RK1 w - -", "rnbqr1k1/pp3ppp/2p2n2/4p3/2PPP3/P1P2N2/2Q3PP/R1B1KB1R w KQ -", "r1b1k2r/pppp1ppp/4p3/8/1nPP4/5NP1/PP2PP1P/R3KB1R w KQkq -", "rn1qkb1r/1b3ppp/p4n2/1p2p3/P2N4/3BP3/1PQ2PPP/RNB2RK1 w kq -", "r2q1rk1/pp1n1pp1/2pb1n1p/3p4/3P2bB/2NBPN2/PPQ2PPP/R3K2R w KQ -", "r2qk2r/p3bppp/2p2n2/2pp4/8/1P2PQ1P/PB1P1PP1/RN2K2R w KQkq -", "rn1q1rk1/pb3ppp/1p1b1n2/2pP4/2B5/P1N2N2/1PQ2PPP/R1B1K2R w KQ -"};

  while ( ( opt = getopt ( argc, argv, "p:d:p:h:f:c:n:b" ) ) != -1 ) {
    if ( opt == 'b' ) {
      Search *search = new Search (  );
      IterativeDeeping *it = new IterativeDeeping ( search );
      search->setMaxTimeMillsec ( 2000 );
      for ( std::vector < string >::iterator ite = benchmarkPos.begin (  ); ite != benchmarkPos.end (  ); ++ite ) {
	cout << *ite << endl << flush;
	search->loadFen ( *ite );
	it->run (  );
      }
      delete search;
      delete it;
      dispose (  );
      return 0;

    }
    if ( opt == 'p' ) {
      if ( string ( optarg ) == "erft" ) {
#ifdef NO_HASH_MODE
	cout << "recompile with -DHASH_MODE";
	myassert ( 0 );
#endif
	perftMode = true;
      }
      else {
	return 1;
      }
    }
    if ( opt == 'd' ) {		//depth
      perftDepth = atoi ( optarg );
    }
    if ( opt == 'c' ) {		//N cpu
      nCpu = atoi ( optarg );
    }
    if ( opt == 'h' ) {		//hash
      PERFT_HASH_SIZE = atoi ( optarg );
    }
    if ( opt == 'f' ) {		//fen
      fen = optarg;
    }
  }
  if ( perftMode ) {
    if ( perftDepth > MAX_PLY || perftDepth <= 0 ) {
      cout << "error in -d parameter" << endl;
      return ( 1 );
    }
    if ( nCpu > 32 || nCpu <= 0 ) {
      cout << "error in -c parameter" << endl;
      return ( 1 );
    }
    if ( PERFT_HASH_SIZE > 32768 || PERFT_HASH_SIZE < 0 ) {
      cout << "error in -h parameter" << endl;
      return ( 1 );
    }
    if ( fen.empty (  ) )
      fen = START_FEN;
    Perft *p = new Perft ( fen, perftDepth, nCpu, PERFT_HASH_SIZE );
    delete p;
  }
  else {
    Uci *uci = new Uci (  );
    delete uci;
  }
  dispose (  );
  return 0;
}

#endif
