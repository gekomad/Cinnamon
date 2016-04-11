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
#include "namespaces/def.h"
#include <unordered_map>
#include "namespaces/random.h"
#include <climits>
#include "util/logger.h"
#include "util/Bitboard.h"

using namespace _logger;
using namespace _board;
using namespace _def;

class ChessBoard : public Bitboard {
public:

#define PAWN_BLACK 0
#define PAWN_WHITE 1
#define ROOK_BLACK 2
#define ROOK_WHITE 3
#define BISHOP_BLACK 4
#define BISHOP_WHITE 5
#define KNIGHT_BLACK 6
#define KNIGHT_WHITE 7
#define KING_BLACK 8
#define KING_WHITE 9
#define QUEEN_BLACK 10
#define QUEEN_WHITE 11

#define RIGHT_CASTLE_IDX 12
#define ENPASSANT_IDX 13
#define SIDETOMOVE_IDX 14
#define ZOBRISTKEY_IDX 15

    ChessBoard();

    virtual ~ChessBoard();

    static string decodeBoardinv(const uchar type, const int a, const int side) {
        if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
            return "e1c1";
        }
        if (type & KING_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
            return "e1g1";
        }
        if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
            return "e8c8";
        }
        if (type & KING_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
            return "e8g8";
        }
        ASSERT(!(type & 0xC));
        if (a >= 0 && a < 64) {
            return BOARD[a];
        }
        _assert(0);
    }

    static const uchar RIGHT_KING_CASTLE_WHITE_MASK = 0x10;
    static const uchar RIGHT_QUEEN_CASTLE_WHITE_MASK = 0x20;
    static const uchar RIGHT_KING_CASTLE_BLACK_MASK = 0x40;
    static const uchar RIGHT_QUEEN_CASTLE_BLACK_MASK = 0x80;
    static const u64 CENTER_MASK = 0x1818000000ULL;

    static const int SQUARE_FREE = 12;

    static const u64 NO_ENPASSANT = 100;
    static const uchar KING_SIDE_CASTLE_MOVE_MASK = 0x4;
    static const uchar QUEEN_SIDE_CASTLE_MOVE_MASK = 0x8;

    void display() const;

    string boardToFen() const;

    string getFen();

    char decodeBoard(string);

    int loadFen(string);

    int getPieceByChar(char);

#ifdef DEBUG_MODE

    u64 getBitmap(int side);

    bool checkNPieces(std::unordered_map<int, int>);

#endif

    template<int side>
    u64 getBitmap() const {
        return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    void setSide(bool b) {
        chessboard[SIDETOMOVE_IDX] = b;
    }

    int getSide() const {
        return chessboard[SIDETOMOVE_IDX];
    }

    template<int side>
    u64 getBitmapNoPawns() const {
        return chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    template<int side>
    int getPieceAt(const u64 bitmapPos) const {
        return ((chessboard[PAWN_BLACK + side] & bitmapPos) ? PAWN_BLACK + side : ((chessboard[ROOK_BLACK + side] & bitmapPos) ? ROOK_BLACK + side : ((chessboard[BISHOP_BLACK + side] & bitmapPos) ? BISHOP_BLACK + side : ((chessboard[KNIGHT_BLACK + side] & bitmapPos) ? KNIGHT_BLACK + side : ((chessboard[QUEEN_BLACK + side] & bitmapPos) ? QUEEN_BLACK + side : ((chessboard[KING_BLACK + side] & bitmapPos) ? KING_BLACK + side : SQUARE_FREE))))));
    }

protected:

    _Tchessboard chessboard;

    typedef struct {
        u64 allPieces;
        u64 kingAttackers[2];
        u64 allPiecesSide[2];
        u64 openFile;
        u64 semiOpenFile[2];
        u64 isolated[2];
        u64 allPiecesNoPawns[2];
        int kingSecurityDistance[2];
        uchar posKing[2];
    } _Tboard;

    static const u64 A7bit = 0x80000000000000ULL;
    static const u64 B7bit = 0x40000000000000ULL;
    static const u64 C6bit = 0x200000000000ULL;
    static const u64 H7bit = 0x1000000000000ULL;
    static const u64 G7bit = 0x2000000000000ULL;
    static const u64 F6bit = 0x40000000000ULL;
    static const u64 A8bit = 0x8000000000000000ULL;
    static const u64 H8bit = 0x100000000000000ULL;
    static const u64 A2bit = 0x8000ULL;
    static const u64 B2bit = 0x4000ULL;
    static const u64 H2bit = 0x100ULL;
    static const u64 G2bit = 0x200ULL;
    static const u64 A1bit = 0x80ULL;
    static const u64 H1bit = 0x1ULL;
    static const u64 F1G1bit = 0x6ULL;
    static const u64 H1H2G1bit = 0x103ULL;
    static const u64 C1B1bit = 0x60ULL;
    static const u64 A1A2B1bit = 0x80c0ULL;
    static const u64 F8G8bit = 0x600000000000000ULL;
    static const u64 H8H7G8bit = 0x301000000000000ULL;
    static const u64 C8B8bit = 0x6000000000000000ULL;
    static const u64 A8A7B8bit = 0xc080000000000000ULL;
    static const u64 C6A6bit = 0xa00000000000ULL;
    static const u64 F6H6bit = 0x50000000000ULL;
    static const u64 A7C7bit = 0xa0000000000000ULL;
    static const u64 H7G7bit = 0x3000000000000ULL;
    static const u64 C3A3bit = 0xa00000ULL;
    static const u64 F3H3bit = 0x50000ULL;
    static const u64 A2C2bit = 0xa000ULL;
    static const u64 H2G2bit = 0x300ULL;

    static const int E1 = 3;
    static const int E8 = 59;
    static const int C1 = 5;
    static const int F1 = 2;
    static const int C8 = 61;
    static const int F8 = 58;
    static const int D8 = 60;
    static const int A8 = 63;
    static const int H8 = 56;
    static const int G8 = 57;
    static const u64 BLACK_SQUARES = 0x55AA55AA55AA55AAULL;
    static const u64 WHITE_SQUARES = 0xAA55AA55AA55AA55ULL;

    _Tboard structureEval;

    void makeZobristKey();

    template<int side>
    int getNpiecesNoPawnNoKing() const {
        return bitCount(chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] | chessboard[QUEEN_BLACK + side]);
    }

#ifdef DEBUG_MODE

    void updateZobristKey(int piece, int position) {
        ASSERT_RANGE(position, 0, 63);
        ASSERT_RANGE(piece, 0, 14);
        chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position];
    }

    int getPieceAt(int side, u64 bitmapPos);

#else
#define updateZobristKey(piece, position) (chessboard[ZOBRISTKEY_IDX] ^= _random::RANDOM_KEY[piece][position])

#endif
private:
    string fenString;

    int loadFen();
};

