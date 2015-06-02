#include "Uci.h"
#include <unistd.h>
#include "namespaces.h"

/*
   # ENGINE                : RATING    POINTS  PLAYED    (%)
   1 Cinnamon 1.1beta14    : 2349.1     890.0    1398   63.7%
   2 Cinnamon 1.0          : 2250.9     508.0    1398   36.3%



 8| 63 62 61 60 59 58 57 56
 7| 55 54 53 52 51 50 49 48
 6| 47 46 45 44 43 42 41 40
 5| 39 38 37 36 35 34 33 32
 4| 31 30 29 28 27 26 25 24
 3| 23 22 21 20 19 18 17 16
 2| 15 14 13 12 11 10 09 08
 1| 07 06 05 04 03 02 01 00
 ...a  b  c  d  e  f  g  h


rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 Depth 	Perft
 1 		20 				verified
 2  	400 			verified
 3 		8902 			verified
 4 		197281 			verified
 5 		4865609 		verified
 6 		119060324 		verified
 7 		3195901860      verified
 8 		84998978956     verified
 9		2439530234167   verified

position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -
 Depth 	Perft
 1		48              verified
 2		2039            verified
 3		97862           verified
 4		4085603         verified
 5		193690690       verified
 6		8031647685      verified
 7      374190009323    verified

rnbqkbnr/pp1ppppp/8/2pP4/8/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 2
Depth 	Perft
 1      30              verified
 2      631             verified
 3      18825           verified
 4      437149          verified
 5      13787913        verified

 astyle --style=java  --suffix=none --close-templates --unpad-paren --align-reference=type --align-pointer=type  --delete-empty-lines  --add-brackets *.cpp *.h
 */

using namespace _board;

int
main ( int argc, char **argv ) {	//TODO comprimere con upx
  _bits::init (  );
  _random::init (  );
  cout << NAME;
  cout << " UCI (ex Butterfly) by Giuseppe Cannella" << endl;
#if UINTPTR_MAX == 0xffffffffffffffff
  cout << "64-bit";
#ifdef HAS_POPCNT
  cout << " popcnt";
#endif

#else
  cout << "32-bit";
#endif
  cout << " version compiled " << __DATE__ << " with gcc " << __VERSION__ << endl;
  cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>" << endl << endl;
#ifdef DEBUG_MODE
  cout << "DEBUG_MODE" << endl;
#endif
  char opt;
  while ( ( opt = getopt ( argc, argv, "bp:" ) ) != -1 ) {
    if ( opt == 'b' ) {
      IterativeDeeping *it = new IterativeDeeping (  );
      it->setUseBook ( false );
      it->setMaxTimeMillsec ( 40000 );
      it->run (  );
      delete it;
      _bits::_free (  );
      return 0;
    }
    else if ( opt == 'p' ) {	// perft test
      const string error = "use: -perft [-d depth] [-c nCpu] [-h hash size (Mb)] [-f \"fen position\"]";
      if ( string ( optarg ) != "erft" ) {
	cout << error << endl;
	_bits::_free (  );
	return 1;
      };
      int nCpu = 1;
      int perftDepth = 1;
      string fen;
      int PERFT_HASH_SIZE = 0;
      while ( ( opt = getopt ( argc, argv, "d:f:h:f:c:" ) ) != -1 ) {
	if ( opt == 'd' ) {	//depth
	  perftDepth = atoi ( optarg );
	}
	else if ( opt == 'c' ) {	//N cpu
	  nCpu = atoi ( optarg );
	}
	else if ( opt == 'h' ) {	//hash
	  PERFT_HASH_SIZE = atoi ( optarg );
	}
	else if ( opt == 'f' ) {	//fen
	  fen = optarg;
	}
      }
      if ( perftDepth > GenMoves::MAX_PLY || perftDepth <= 0 || nCpu > 32 || nCpu <= 0 || PERFT_HASH_SIZE > 32768 || PERFT_HASH_SIZE < 0 ) {
	cout << error << endl;
	_bits::_free (  );
	return 1;
      }

      if ( fen.empty (  ) )
	fen = STARTPOS;
      cout << argv[0] << " -perft -d " << perftDepth << " -c " << nCpu << " -h " << PERFT_HASH_SIZE << " -f \"" << fen << "\"" << endl;
      Perft *p = new Perft ( fen, perftDepth, nCpu, PERFT_HASH_SIZE );
      delete p;
      return 0;
    }
  }
  Uci *uci = new Uci (  );
  delete uci;
  _bits::_free (  );
  return 0;
}
