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

#include <iostream>
#include <string.h>
#include <sstream>
#include "namespaces/String.h"
#include "namespaces/bits.h"
#include <unordered_map>
#include "namespaces/random.h"
#include "namespaces/board.h"
#include <climits>
#include "util/logger.h"
#include "util/Bitboard.h"

#ifdef BENCH_MODE

#include "util/bench/Times.h"

#endif

using namespace _logger;
using namespace constants;
using namespace _def;

class ChessBoard {
public:
    ChessBoard();

    string decodeBoardinv(const _Tmove *move, const uchar side, const bool verbose = false);

    virtual ~ChessBoard();

    const string getFen() const;

    int loadFen(const string &);

    void clearChessboard();

    void setSide(const bool b) {
        sideToMove = b;
    }

    void setChess960(bool c) { chess960 = c; }

    bool isChess960() const {
        return chess960;
    }

    void display() const;

    string boardToFen() const;

    uchar rightCastle;

    uchar enPassant;
    uchar sideToMove;

    void print(const _Tmove *move);

    _Tchessboard chessboard;
protected:


    uchar startPosWhiteKing;
    uchar startPosWhiteRookKingSide;
    uchar startPosWhiteRookQueenSide;

    uchar startPosBlackKing;
    uchar startPosBlackRookKingSide;
    uchar startPosBlackRookQueenSide;


    string MATCH_QUEENSIDE;
    string MATCH_QUEENSIDE_WHITE;
    string MATCH_KINGSIDE_WHITE;
    string MATCH_QUEENSIDE_BLACK;
    string MATCH_KINGSIDE_BLACK;

    int movesCount = 1;
    bool chess960 = false;

    void makeZobristKey();


#ifndef NDEBUG

    void updateZobristKey(int piece, int position) {
        ASSERT_RANGE(position, 0, 63)
        ASSERT_RANGE(piece, 0, 15)
        chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position];
    }

#else
#define updateZobristKey(piece, position) (chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position])

#endif
private:
    string fenString;
    char whiteRookKingSideCastle;
    char whiteRookQueenSideCastle;
    char blackRookKingSideCastle;
    char blackRookQueenSideCastle;

    int loadFen();
};
