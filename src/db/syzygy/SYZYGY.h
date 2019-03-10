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

#pragma once

#include "../../namespaces/def.h"
#include "../../ChessBoard.h"
#include "../../util/Singleton.h"
#include "tbprobe.h"

#include <map>

using namespace std;


class SYZYGY: public Singleton<SYZYGY> {
    friend class Singleton<SYZYGY>;

public:

    ~SYZYGY();

    bool getAvailable() const;

    string getPath() const;

    bool setPath(const string &path);

    void restart();


    int getDtm(const _Tchessboard &c, const bool turn);

//    string getBestmove(const _Tchessboard &c, const bool turn);

private:


    const map<string, string> mapBoardPos = {
        {"h1", "h8"},
        {"h2", "h7"},
        {"h3", "h6"},
        {"h4", "h5"},
        {"h5", "h4"},
        {"h6", "h3"},
        {"h7", "h2"},
        {"h8", "h1"},

        {"g1", "g8"},
        {"g2", "g7"},
        {"g3", "g6"},
        {"g4", "g5"},
        {"g5", "g4"},
        {"g6", "g3"},
        {"g7", "g2"},
        {"g8", "g1"},

        {"f1", "f8"},
        {"f2", "f7"},
        {"f3", "f6"},
        {"f4", "f5"},
        {"f5", "f4"},
        {"f6", "f3"},
        {"f7", "f2"},
        {"f8", "f1"},

        {"b1", "b8"},
        {"b2", "b7"},
        {"b3", "b6"},
        {"b4", "b5"},
        {"b5", "b4"},
        {"b6", "b3"},
        {"b7", "b2"},
        {"b8", "b1"},

        {"a1", "a8"},
        {"a2", "a7"},
        {"a3", "a6"},
        {"a4", "a5"},
        {"a5", "a4"},
        {"a6", "a3"},
        {"a7", "a2"},
        {"a8", "a1"},

        {"c1", "c8"},
        {"c2", "c7"},
        {"c3", "c6"},
        {"c4", "c5"},
        {"c5", "c4"},
        {"c6", "c3"},
        {"c7", "c2"},
        {"c8", "c1"},


        {"d1", "d8"},
        {"d2", "d7"},
        {"d3", "d6"},
        {"d4", "d5"},
        {"d5", "d4"},
        {"d6", "d3"},
        {"d7", "d2"},
        {"d8", "d1"},

        {"e1", "e8"},
        {"e2", "e7"},
        {"e3", "e6"},
        {"e4", "e5"},
        {"e5", "e4"},
        {"e6", "e3"},
        {"e7", "e2"},
        {"e8", "e1"}};

    string decodePos(string &s);

    SYZYGY();

    int search(const _Tchessboard &c, const bool turn, unsigned *results);

    string path = "";

    string pickMove(const unsigned *results, const unsigned wdl);

    int rank(int s) { return ((s) >> 3); }

    int file(int s) { return ((s) & 0x07); }

    u64 decode(u64 d);
};

