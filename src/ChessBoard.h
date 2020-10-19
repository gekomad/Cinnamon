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
#include "util/String.h"
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

class ChessBoard : public Bitboard {
public:
    ChessBoard();

    virtual ~ChessBoard();

    const string getFen() const;

    int loadFen(const string &);

    const _Tchessboard &getChessboard() const {
        return chessboard;
    }

    void setSide(const bool b) {
        chessboard[SIDETOMOVE_IDX] = b;
    }

protected:
#ifdef BENCH_MODE
    Times *times = &Times::getInstance();
#endif
    _Tchessboard chessboard;
    int startPosWhiteKing = 999;
    int startPosWhiteRookKingSide = 999;
    int startPosWhiteRookQueenSide = 999;

    int startPosBlackKing = 999;
    int startPosBlackRookKingSide = 999;
    int startPosBlackRookQueenSide = 999;

    _Tboard structureEval;

    void makeZobristKey();

#ifdef DEBUG_MODE

    void updateZobristKey(int piece, int position) {
        ASSERT_RANGE(position, 0, 63);
        ASSERT_RANGE(piece, 0, 14);
        chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position];
    }

#else
#define updateZobristKey(piece, position) (chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position])

#endif
private:
    string fenString;

    int loadFen();
};

