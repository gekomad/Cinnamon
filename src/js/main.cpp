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

#include "../perft/Perft.h"
#include "Uci.h"

Uci *u = nullptr;

using namespace constants;

extern "C" {

char *command(const char *t, const char *arg) {
    return u->command(t, arg);
}

unsigned perft(const char *fen, const int depth, const int hashSize, const bool chess960) {
    Perft *p = &Perft::getInstance();
    p->setParam(fen, depth, 1, hashSize, "", chess960);
    p->start();
    p->join();
    return p->getResult();
}
int isvalid(const char *fen) {
    ChessBoard c;
    return c.loadFen(fen) == 2 ? 0 : 1;
}

} // extern C

int main(int argc, char **argv) {
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
    cout << "version compiled " << __DATE__ << " with emscripten - " << __VERSION__ << endl;
    cout << "License GPLv3+: GNU GPL version 3 or later <https://www.gnu.org/licenses/gpl-3.0.html>" << endl << endl;
    u = new Uci();
    return 0;
}
