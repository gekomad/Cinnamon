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

#include "gtb/gtb-probe.h"
#include "namespaces/def.h"
#include "ChessBoard.h"
#include "util/Singleton.h"

class GTB : public Singleton<GTB> {
    friend class Singleton<GTB>;

public:

    ~GTB();

    bool getAvailable() const;

    int getCache() const;

    string getPath() const;

    string getSchema() const;

    bool setCacheSize(const int mb);

    bool setPath(const string &path);

    bool setScheme(const string &s);

    void restart();

    bool setProbeDepth(const int d);

    bool setInstalledPieces(const int n);

    bool isInstalledPieces(const int p) const;

    int getProbeDepth() const;

    int getDtm(const int side, const bool doPrint, const _Tchessboard &chessboard, const uchar rightCastle, const int depth) const;

private:
    GTB();

    const int DECODE_PIECE[13] = {tb_PAWN, tb_PAWN, tb_ROOK, tb_ROOK, tb_BISHOP, tb_BISHOP, tb_KNIGHT, tb_KNIGHT, tb_KING, tb_KING, tb_QUEEN, tb_QUEEN, tb_NOPIECE};

    const int DECODE_POSITION[64] = {tb_H1, tb_G1, tb_F1, tb_E1, tb_D1, tb_C1, tb_B1, tb_A1, tb_H2, tb_G2, tb_F2, tb_E2, tb_D2, tb_C2, tb_B2, tb_A2, tb_H3, tb_G3, tb_F3, tb_E3, tb_D3, tb_C3, tb_B3, tb_A3, tb_H4, tb_G4, tb_F4, tb_E4, tb_D4, tb_C4, tb_B4, tb_A4, tb_H5, tb_G5, tb_F5, tb_E5, tb_D5, tb_C5, tb_B5, tb_A5, tb_H6, tb_G6, tb_F6, tb_E6, tb_D6, tb_C6, tb_B6, tb_A6, tb_H7, tb_G7, tb_F7, tb_E7, tb_D7, tb_C7, tb_B7, tb_A7, tb_H8, tb_G8, tb_F8, tb_E8, tb_D8, tb_C8, tb_B8, tb_A8,};

    int extractDtm(const unsigned stm1, const bool doPrint, const int tb_available1, const unsigned info1, const unsigned pliestomate1) const;

    void print(const unsigned stm1, const unsigned info1, const unsigned pliestomate1) const;

    bool load();

    const int verbosity = 0;
    int cacheSize = 32;        //mb
    const char **paths = nullptr;
    string path = "gtb/gtb4";
    int scheme = tb_CP4;
    int probeDepth = 0;
    bool installedPieces[33];    // 3,4,5
    const int wdl_fraction = 96;

};

