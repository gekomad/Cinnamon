/*
    Cinnamon is a UCI chess engine
    Copyright (C) 2011-2014 Giuseppe Cannella

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

#include "Uci.h"

/*

 8| 63 62 61 60 59 58 57 56
 7| 55 54 53 52 51 50 49 48
 6| 47 46 45 44 43 42 41 40
 5| 39 38 37 36 35 34 33 32
 4| 31 30 29 28 27 26 25 24
 3| 23 22 21 20 19 18 17 16
 2| 15 14 13 12 11 10 09 08
 1| 07 06 05 04 03 02 01 00
 ...a  b  c  d  e  f  g  h

ratings
Rank Name                  Elo    +    - games score oppo. draws
   1 buzz-008_2181         385   13   13  2551   79%   146   14%
   2 beowulf_2194          368   13   13  2551   77%   146   14%
   3 gk-090-64-ja_2109     360   13   13  2552   77%   146   16%
   4 clarabit_2098         263   12   12  2542   65%   146   18%
   5 soldat_1960           197   12   12  2542   56%   146   16%
   6 smash_1925            172   12   12  2552   53%   146   12%
   7 faile-64_1976         153   11   11  2552   51%   146   20%
   8 Cinnamon 1.2a         152   11   11  2550   51%   146   30%
   9 Cinnamon 1.2b         146    5    5 33140   52%   -11   15%
  10 Cinnamon 1.1c         105   11   11  2552   44%   146   27%
  11 heracles_1973         105   12   12  2551   44%   146   16%
  12 zct_2043              -15   12   12  2542   30%   146   12%
  13 jabba-64_2041       -1196  200  179  2551    0%   146    0%
  14 gullydeckel-64_1982 -1196  200  179  2552    0%   146    0%

los
                     bu be gk cl so sm fa Ci Ci Ci he zc ja gu
buzz-008_2181           95 99100100100100100100100100100100100
beowulf_2194          4    78100100100100100100100100100100100
gk-090-64-ja_2109     0 21   100100100100100100100100100100100
clarabit_2098         0  0  0    99100100100100100100100100100
soldat_1960           0  0  0  0    99 99 99 99100100100100100
smash_1925            0  0  0  0  0    98 99 99 99 99100100100
faile-64_1976         0  0  0  0  0  1    54 86 99 99100100100
Cinnamon 1.2a         0  0  0  0  0  0 45    83 99 99100100100
Cinnamon 1.2b         0  0  0  0  0  0 13 16    99 99100100100
Cinnamon 1.1c         0  0  0  0  0  0  0  0  0    52100100100
heracles_1973         0  0  0  0  0  0  0  0  0 47   100100100
zct_2043              0  0  0  0  0  0  0  0  0  0  0    99 99
jabba-64_2041         0  0  0  0  0  0  0  0  0  0  0  0    50
gullydeckel-64_1982   0  0  0  0  0  0  0  0  0  0  0  0 49

 */

using namespace _board;

int
main ( int argc, char **argv )
{
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
#if UINTPTR_MAX == 0xffffffffffffffff
    cout << "64-bit ";
#else
    cout << "32-bit ";
#endif
#ifdef HAS_POPCNT
    cout << "popcnt ";
#endif
#ifdef HAS_BSF
    cout << "bsf ";
#endif
    cout << "version compiled " << __DATE__ << " with gcc " << __VERSION__ << "\n";
    cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n";
#ifdef CLOP
    cout << "CLOP ENABLED\n";
#endif
#ifdef DEBUG_MODE
    cout << "DEBUG_MODE\n";
#endif
    cout << flush;
    char opt;
    const string DTM_HELP = "-dtm -f \"fen position\" [-p path] [-s scheme] [-i installed pieces]";
    const string PERFT_HELP = "-perft [-d depth] [-c nCpu] [-h hash size (mb)] [-f \"fen position\"] [-F dump file]";
    const string EPD2PGN_HELP = "-epd2pgn -f file_epd [-m max_pieces]";

    while ( ( opt = getopt ( argc, argv, "e:hd:bp:f:" ) ) != -1 )
    {
        if ( opt == 'h' )
        {
            cout << "Distance to mate: " << argv[0] << " " << DTM_HELP << "\n";
            cout << "Perft test: " << argv[0] << " " << PERFT_HELP << "\n";
            cout << "Create .pgn from .epd: " << argv[0] << " " << EPD2PGN_HELP << endl;
            return 0;
        }

        if ( opt == 'e' )
        {
            if ( string ( optarg ) != "pd2pgn" )
            {
                cout << "use: " << argv[0] << " " << EPD2PGN_HELP << endl;
                return 1;
            };

            string epdfile;

            int m = 64;

            while ( ( opt = getopt ( argc, argv, "f:m:" ) ) != -1 )
            {
                if ( opt == 'f' )  	//file
                {
                    epdfile = optarg;
                }

                if ( opt == 'm' )  	//n' pieces
                {
                    string h = optarg;
                    m = stoi ( h );
                }
            }

            ///////////////////
            ifstream inData;
            string fen;

            if ( !_file::fileExists ( epdfile ) )
            {
                cout << "error file not found  " << epdfile << endl;
                return 1;
            }

            inData.open ( epdfile );
            int count = 0;
            int n = 0;
            ostringstream os;
            os << "[Date \"" << _time::getYear (  ) << "." << _time::getMonth (  ) << "." << _time::getDay (  ) << "\"]";
            string date = os.str (  );

            while ( !inData.eof (  ) )
            {
                getline ( inData, fen );
                n = 0;

                for ( unsigned i = 0; i < fen.size (  ); i++ )
                {
                    char c = tolower ( fen[i] );

                    if ( c == ' ' )
                    {
                        break;
                    }

                    if ( c == 'b' || c == 'k' || c == 'r' || c == 'q' || c == 'p' || c == 'n' )
                    {
                        n++;
                    }
                }

                if ( n > 0 && n <= m )
                {
                    count++;
                    cout << "[Site \"" << count << " (" << n << " pieces)\"]\n";
                    cout << date << "\n";
                    cout << "[Result \"*\"]\n";
                    string fenClean, token;
                    istringstream uip ( fen, ios::in );
                    uip >> token;
                    fenClean += token + " ";
                    uip >> token;
                    fenClean += token + " ";
                    uip >> token;
                    fenClean += token + " ";
                    uip >> token;
                    fenClean += token;
                    cout << "[FEN \"" << fenClean << "\"]\n";
                    cout << "*" << "\n";
                }
            }

            cout << endl;
            return 0;
        }

        if ( opt == 'b' )
        {
            unique_ptr < IterativeDeeping > it ( new IterativeDeeping (  ) );
            it->setUseBook ( false );
            it->setMaxTimeMillsec ( 40000 );
            it->run (  );
            return 0;
        }
        else if ( opt == 'd' )  	// gtb dtm
        {
            if ( string ( optarg ) != "tm" )
            {
                cout << "use: " << argv[0] << " " << DTM_HELP << endl;
                return 1;
            };

            string fen, token;

            IterativeDeeping it;

            while ( ( opt = getopt ( argc, argv, "f:p:s:i:" ) ) != -1 )
            {
                if ( opt == 'f' )  	//fen
                {
                    fen = optarg;
                }
                else if ( opt == 'p' )  	//path
                {
                    token = optarg;
                    it.getGtb (  ).setPath ( token );
                }
                else if ( opt == 's' )  	//scheme
                {
                    token = optarg;

                    if ( !it.getGtb (  ).setScheme ( token ) )
                    {
                        cout << "set scheme error" << endl;
                        return 1;
                    }
                }
                else if ( opt == 'i' )
                {
                    token = optarg;

                    if ( !it.getGtb (  ).setInstalledPieces ( stoi ( token ) ) )
                    {
                        cout << "set installed pieces error" << endl;
                        return 1;
                    }
                }
            }

            if ( !it.getGtbAvailable (  ) )
            {
                cout << "error TB not found" << endl;
                return 1;
            }

            it.loadFen ( fen );
            it.printDtm (  );
            return 0;
        }
        else if ( opt == 'p' )  	// perft test
        {
            if ( string ( optarg ) != "erft" )
            {
                continue;
            };

            int nCpu = 0;

            int perftDepth = 0;

            string fen;

            int PERFT_HASH_SIZE = 0;

            string dumpFile;

            while ( ( opt = getopt ( argc, argv, "d:f:h:f:c:F:" ) ) != -1 )
            {
                if ( opt == 'd' )  	//depth
                {
                    perftDepth = atoi ( optarg );
                }
                else if ( opt == 'F' )  	//use dump
                {
                    dumpFile = optarg;

                    if ( dumpFile.empty (  ) )
                    {
                        cout << "use: " << argv[0] << " " << PERFT_HELP << endl;
                        return 1;
                    }
                }
                else if ( opt == 'c' )  	//N cpu
                {
                    nCpu = atoi ( optarg );
                }
                else if ( opt == 'h' )  	//hash
                {
                    PERFT_HASH_SIZE = atoi ( optarg );
                }
                else if ( opt == 'f' )  	//fen
                {
                    fen = optarg;
                }
            }

            if ( perftDepth > GenMoves::MAX_PLY || perftDepth < 0 || nCpu > 32 || nCpu < 0 || PERFT_HASH_SIZE > 32768 || PERFT_HASH_SIZE < 0 )
            {
                cout << "use: " << argv[0] << " " << PERFT_HELP << endl;
                return 1;
            }

            if ( PERFT_HASH_SIZE )
            {
                cout << "dump hash table in file every " << ( Perft::secondsToDump / 60 ) << " minutes" << endl;
            }
            unique_ptr < Perft > p ( new Perft ( fen, perftDepth, nCpu, PERFT_HASH_SIZE, dumpFile ) );
            return 0;
        }
    }

    unique_ptr < Uci > p ( new Uci (  ) );
    return 0;
}
