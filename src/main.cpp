/*
em++  -std=c++11  ChessBoard.cpp  Eval.cpp  GenMoves.cpp  Hash.cpp  IterativeDeeping.cpp  main.cpp  OpenBook.cpp  Search.cpp  SearchManager.cpp   Uci.cpp util/String.cpp util/IniFile.cpp util/Bits.cpp -w -s EXPORTED_FUNCTIONS="['_main','_perft','_command']" -s NO_EXIT_RUNTIME=1 -o cinnamon2.0.js -O3 --memory-init-file 0

com = Module.cwrap('command', 'string', ['string','string'])
com("setMaxTimeMillsec","3000")
com("position","r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -")
com("go","")


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
#include "perft/Perft.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

 */
Uci *u = NULL;
Perft *p = NULL;
ChessBoard *c = NULL;
using namespace _board;

extern "C" {

char *command(char *t, char *arg) {
    return u->command(t, arg);
}

unsigned perft(char *fen, int depth) {
    return 1;
}
int isvalid(char *fen) {
    if (c->loadFen(fen) == -1)return 0;
    return 1;
}

}//extern C

int main(int argc, char **argv) {
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
    cout << "version compiled " << __DATE__ << " with emscripten - " << __VERSION__ << "\n";
    cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n";
    //p = new Perft();
//    c = new ChessBoard();
	u = new Uci();
//    if (c->loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") == -1)cout <<"KO\n";
//    else cout <<"OK\n";

//    int nMoves = perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3);
//    cout << "N tasks: " << nMoves << endl;


//    
  //  cout << u->command("go", "") << endl;
    return 0;
}

