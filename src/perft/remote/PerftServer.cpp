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

#include "PerftServer.h"

void PerftServer::receive(string msg) {
    String a(msg);

    String fen, dumpFile;
    int depth, hashsize;

    stringstream ss(a);

    ss >> fen;

    fen = fen.replace(1, ' ');
    ss >> depth;
    ss >> hashsize;
    ss >> dumpFile;
    dumpFile = dumpFile.replace(1, ' ');
    cout << "fen: " << fen << "\n";
    cout << "depth: " << depth << "\n";
    cout << "hashsize: " << hashsize << "\n";
    cout << "dumpFile: " << dumpFile << endl;


};

