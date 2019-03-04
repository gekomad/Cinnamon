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

class ChessBoard: public Bitboard {
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

    static constexpr uchar RIGHT_KING_CASTLE_WHITE_MASK = 0x10;
    static constexpr uchar RIGHT_QUEEN_CASTLE_WHITE_MASK = 0x20;
    static constexpr uchar RIGHT_KING_CASTLE_BLACK_MASK = 0x40;
    static constexpr uchar RIGHT_QUEEN_CASTLE_BLACK_MASK = 0x80;
    static constexpr u64 CENTER_MASK = 0x1818000000ULL;

    static constexpr int SQUARE_FREE = 12;

    static constexpr u64 NO_ENPASSANT = 100;
    static constexpr uchar KING_SIDE_CASTLE_MOVE_MASK = 0x4;
    static constexpr uchar QUEEN_SIDE_CASTLE_MOVE_MASK = 0x8;

    void display();

    string boardToFen() const;

    string getFen();

    char decodeBoard(string);

    int loadFen(string);

    int getPieceByChar(char);

#ifdef DEBUG_MODE

    u64 getBitmap(int side);

    bool checkNPieces(std::unordered_map<int, int>);

#endif

    static u64 colors(int pos) {
        if (POW2[pos] & 0x55aa55aa55aa55aaULL)return 0x55aa55aa55aa55aaULL;
        return 0xaa55aa55aa55aa55ULL;
    }

    template<int side>
    u64 getBitmap() const {
        return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] |
            chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    template<int side>
    static u64 getBitmap(const _Tchessboard &chessboard) {
        return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] |
            chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    void setSide(bool b) {
        chessboard[SIDETOMOVE_IDX] = b;
    }

    int getSide() const {
        return chessboard[SIDETOMOVE_IDX];
    }

    template<int side>
    u64 getBitmapNoPawns() const {
        return chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
            chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    template<int side>
    int getPieceAt(const u64 bitmapPos) const {
        return ((chessboard[PAWN_BLACK + side] & bitmapPos) ? PAWN_BLACK + side : ((chessboard[ROOK_BLACK + side] &
            bitmapPos) ? ROOK_BLACK + side
                       : ((chessboard[
                BISHOP_BLACK +
                    side] &
                bitmapPos) ?
                          BISHOP_BLACK + side
                           : ((chessboard[
                    KNIGHT_BLACK +
                        side] &
                    bitmapPos)
                              ?
                              KNIGHT_BLACK +
                                  side
                              : ((chessboard[
                        QUEEN_BLACK +
                            side] &
                        bitmapPos)
                                 ?
                                 QUEEN_BLACK +
                                     side
                                 : ((chessboard[
                            KING_BLACK +
                                side] &
                            bitmapPos)
                                    ?
                                    KING_BLACK +
                                        side
                                    : SQUARE_FREE))))));
    }

    static string getCell(const int file, const int rank) {
        return BOARD[FILE_AT[file] * 8 + rank];
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
        u64 posKingBit[2];
        //u64 pinned[2]; anche x regina?
        int kingSecurity[2];
        uchar posKing[2];

    } _Tboard;

    static constexpr u64 A7bit = 0x80000000000000ULL;
    static constexpr u64 B7bit = 0x40000000000000ULL;
    static constexpr u64 C6bit = 0x200000000000ULL;
    static constexpr u64 H7bit = 0x1000000000000ULL;
    static constexpr u64 G7bit = 0x2000000000000ULL;
    static constexpr u64 F6bit = 0x40000000000ULL;
    static constexpr u64 A8bit = 0x8000000000000000ULL;
    static constexpr u64 H8bit = 0x100000000000000ULL;
    static constexpr u64 A2bit = 0x8000ULL;
    static constexpr u64 B2bit = 0x4000ULL;
    static constexpr u64 H2bit = 0x100ULL;
    static constexpr u64 G2bit = 0x200ULL;
    static constexpr u64 A1bit = 0x80ULL;
    static constexpr u64 H1bit = 0x1ULL;
    static constexpr u64 F1G1bit[2] = {0x600000000000000ULL, 0x6ULL};
    static constexpr u64 H1H2G1bit[2] = {0x301000000000000ULL, 0x103ULL};
    static constexpr u64 C1B1bit[2] = {0x6000000000000000ULL, 0x60ULL};
    static constexpr u64 A1A2B1bit[2] = {0xc080000000000000ULL, 0x80c0ULL};

    static constexpr u64 C6A6bit = 0xa00000000000ULL;
    static constexpr u64 F6H6bit = 0x50000000000ULL;
    static constexpr u64 A7C7bit = 0xa0000000000000ULL;
    static constexpr u64 H7G7bit = 0x3000000000000ULL;
    static constexpr u64 C3A3bit = 0xa00000ULL;
    static constexpr u64 F3H3bit = 0x50000ULL;
    static constexpr u64 A2C2bit = 0xa000ULL;
    static constexpr u64 H2G2bit = 0x300ULL;

    static constexpr int H1 = 0;
    static constexpr int H2 = 8;
    static constexpr int H3 = 16;
    static constexpr int H4 = 24;
    static constexpr int H5 = 32;
    static constexpr int H6 = 40;
    static constexpr int H7 = 48;
    static constexpr int H8 = 56;

    static constexpr int G1 = 1;
    static constexpr int G2 = 9;
    static constexpr int G3 = 17;
    static constexpr int G4 = 25;
    static constexpr int G5 = 33;
    static constexpr int G6 = 41;
    static constexpr int G7 = 49;
    static constexpr int G8 = 57;

    static constexpr int F1 = 2;
    static constexpr int F2 = 10;
    static constexpr int F3 = 18;
    static constexpr int F4 = 26;
    static constexpr int F5 = 34;
    static constexpr int F6 = 42;
    static constexpr int F7 = 50;
    static constexpr int F8 = 58;

    static constexpr int B1 = 6;
    static constexpr int B2 = 14;
    static constexpr int B3 = 22;
    static constexpr int B4 = 30;
    static constexpr int B5 = 38;
    static constexpr int B6 = 46;
    static constexpr int B7 = 54;
    static constexpr int B8 = 62;

    static constexpr int A1 = 7;
    static constexpr int A2 = 15;
    static constexpr int A3 = 23;
    static constexpr int A4 = 31;
    static constexpr int A5 = 39;
    static constexpr int A6 = 47;
    static constexpr int A7 = 55;
    static constexpr int A8 = 63;

    static constexpr int C1 = 5;
    static constexpr int C2 = 13;
    static constexpr int C3 = 21;
    static constexpr int C4 = 29;
    static constexpr int C5 = 37;
    static constexpr int C6 = 45;
    static constexpr int C7 = 53;
    static constexpr int C8 = 61;


    static constexpr int D1 = 4;
    static constexpr int D2 = 12;
    static constexpr int D3 = 20;
    static constexpr int D4 = 28;
    static constexpr int D5 = 36;
    static constexpr int D6 = 44;
    static constexpr int D7 = 52;
    static constexpr int D8 = 60;


    static constexpr int E1 = 3;
    static constexpr int E2 = 11;
    static constexpr int E3 = 19;
    static constexpr int E4 = 27;
    static constexpr int E5 = 35;
    static constexpr int E6 = 43;
    static constexpr int E7 = 51;
    static constexpr int E8 = 59;


    static constexpr u64 BLACK_SQUARES = 0x55AA55AA55AA55AAULL;
    static constexpr u64 WHITE_SQUARES = 0xAA55AA55AA55AA55ULL;

    _Tboard structureEval;

    void makeZobristKey();

    template<int side>
    int getNpiecesNoPawnNoKing() const {
        return bitCount(
            chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
                chessboard[QUEEN_BLACK + side]);
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

