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

#include "SYZYGY.h"

#ifdef JS_MODE
bool SYZYGY::getAvailable() const{return false;}

string SYZYGY::getPath() const{return "";}

bool SYZYGY::setPath(const string &path){return false;}

void SYZYGY::restart(){}

int SYZYGY::getDtm() const{return INT_MAX;}

string SYZYGY::getBestmove(){return false;}
#else

SYZYGY::SYZYGY() {

}

SYZYGY::~SYZYGY() {

}

bool SYZYGY::getAvailable() const { return false; }

string SYZYGY::getPath() const { return ""; }

bool SYZYGY::setPath(const string &path) {
    SYZYGY::path = path;
    tb_init_syzygy(path.c_str());
    return TB_LARGEST;
}

void SYZYGY::restart() { }

int SYZYGY::getDtm(const _Tchessboard &c, const bool turn) {
    unsigned results[TB_MAX_MOVES];

    int res = search(c, turn, results);
    if (res != TB_RESULT_FAILED) {
        return TB_GET_DTZ(res);
    }
    return INT_MAX;
}

string SYZYGY::decodePos(string &s) {
    auto a1 = mapBoardPos.find(s.substr(0, 2))->second;
    auto a2 = mapBoardPos.find(s.substr(2, 4))->second;
    return a1 + a2;
}

string SYZYGY::getBestmove(const _Tchessboard &c, const bool turn) {
    unsigned results[TB_MAX_MOVES];
    int res = search(c, turn, results);
    if (res == TB_RESULT_FAILED) {
        return "";
    }

    string bestmove = pickMove(results, TB_WIN);
    if (!bestmove.empty())return decodePos(bestmove);

    bestmove = pickMove(results, TB_CURSED_WIN);
    if (!bestmove.empty())return decodePos(bestmove);

    bestmove = pickMove(results, TB_DRAW);
    if (!bestmove.empty())return decodePos(bestmove);

    bestmove = pickMove(results, TB_BLESSED_LOSS);
    if (!bestmove.empty())return decodePos(bestmove);

    bestmove = pickMove(results, TB_LOSS);

    return decodePos(bestmove);
}

int SYZYGY::search(const _Tchessboard &c, const bool turn, unsigned *results) {
    u64 white = decode(ChessBoard::getBitmap<WHITE>(c));
    u64 black = decode(ChessBoard::getBitmap<BLACK>(c));
    u64 kings = decode(c[KING_BLACK] | c[KING_WHITE]);
    u64 queens = decode(c[QUEEN_BLACK] | c[QUEEN_WHITE]);
    u64 rooks = decode(c[ROOK_BLACK] | c[ROOK_WHITE]);
    u64 bishops = decode(c[BISHOP_BLACK] | c[BISHOP_WHITE]);
    u64 knights = decode(c[KNIGHT_BLACK] | c[KNIGHT_WHITE]);
    u64 pawns = decode(c[PAWN_BLACK] | c[PAWN_WHITE]);
    unsigned rule50 = 0;//TODO
    unsigned castling = 0;//TODO
    u64 ep = 0;//TODO

    return tb_probe_root(white, black, kings,
                         queens, rooks, bishops, knights, pawns,
                         rule50, castling, ep, turn, results);
}

string SYZYGY::pickMove(const unsigned *results, const unsigned wdl) {
    for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {
        if (TB_GET_WDL(results[i]) != wdl)
            continue;

        unsigned from = TB_GET_FROM(results[i]);
        unsigned to = TB_GET_TO(results[i]);
        unsigned r = rank(from);
        unsigned f = file(from);

        unsigned r1 = rank(to);
        unsigned f1 = file(to);

        return ChessBoard::getCell(f, r).append(ChessBoard::getCell(f1, r1));

    }

    return "";
}

u64 SYZYGY::decode(u64 c) {

    static constexpr array<int, 64> _decode = {
        7, 6, 5, 4, 3, 2, 1, 0,
        15, 14, 13, 12, 11, 10, 9, 8,
        23, 22, 21, 20, 19, 18, 17, 16,
        31, 30, 29, 28, 27, 26, 25, 24,
        39, 38, 37, 36, 35, 34, 33, 32,
        47, 46, 45, 44, 43, 42, 41, 40,
        55, 54, 53, 52, 51, 50, 49, 48,
        63, 62, 61, 60, 59, 58, 57, 56};

    u64 res = 0;
    for (; c; RESET_LSB(c)) {
        int position = BITScanForward(c);
        res |= POW2[_decode[position]];
    }
    return res;
}


#endif
