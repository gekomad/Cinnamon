/*
    Cinnamon UCI chess engine
    Copyright (C) Giuseppe Cannella

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
#include "../perft/Perft.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Uci *u = NULL;

using namespace _board;

extern "C" {

char *command(char *t, char *arg) {
    return u->command(t, arg);
}

unsigned perft(char *fen, int depth, int hashSize) {
    Perft *p = &Perft::getInstance();
    p->setParam(fen, depth, 1, hashSize, "", true);
    p->start();
    p->join();
    return p->getResult();
}
int isvalid(char *fen) {
    ChessBoard c;
    return c.loadFen(fen) == 2 ? 0 : 1;
}

}//extern C

int main(int argc, char **argv) {
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
    cout << "version compiled " << __DATE__ << " with emscripten - " << __VERSION__ << "\n";
    cout << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n";
    u = new Uci();
    return 0;
}
