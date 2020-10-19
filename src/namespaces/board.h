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


// only pure functions

#pragma once

#include "constants.h"
#include "../util/bench/Times.h"
#include "../util/Bitboard.h"

using namespace constants;

class board {
public:
    static u64 colors(const int pos);

    static bool isOccupied(const uchar pos, const u64 allpieces);

    static int getFile(const char cc);

    static void display(const _Tchessboard &chessboard);

    static bool checkInsufficientMaterial(const int nPieces, const _Tchessboard &chessboard);

    static u64 performRankFileCaptureAndShift(const int position, const u64 enemies, const u64 allpieces);

    static void print(const _Tmove *move, const _Tchessboard &chessboard);

    static int getSide(const _Tchessboard &chessboard);

    template<int side>
    static u64
    getPinned(const u64 allpieces, const u64 friends, const int kingPosition, const _Tchessboard &chessboard) {
        BENCH(Times::getInstance().start("pinTime"))
        u64 result = 0;
        ASSERT_RANGE(kingPosition, 0, 63);
        const u64 *s = LINK_SQUARE[kingPosition];
        constexpr int xside = side ^1;
        u64 attacked = DIAGONAL_ANTIDIAGONAL[kingPosition] &
                       (chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside]);
        attacked |=
                RANK_FILE[kingPosition] & (chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside]);
        for (; attacked; RESET_LSB(attacked)) {
            const int pos = BITScanForward(attacked);
            const u64 b = *(s + pos) & allpieces;
#ifdef DEBUG_MODE
            u64 x = *(s + pos) & (allpieces & NOTPOW2[kingPosition]);
            ASSERT(b == x);
#endif
            if (!(b & (b - 1))) {
                result |= b & friends;
            }
        }
        BENCH(Times::getInstance().stop("pinTime"))
        return result;
    }

    static u64 getDiagShiftAndCapture(const int position, const u64 enemies, const u64 allpieces);

    static int getDiagShiftCount(const int position, const u64 allpieces);

    static bool isCastleRight_WhiteKing(const _Tchessboard &chessboard);

    static u64 getMobilityQueen(const int position, const u64 enemies, const u64 allpieces);

    static u64 getMobilityRook(const int position, const u64 enemies, const u64 friends);

    static bool isCastleRight_BlackKing(const _Tchessboard &chessboard);

    static bool isCastleRight_WhiteQueen(const _Tchessboard &chessboard);

    static bool isCastleRight_BlackQueen(const _Tchessboard &chessboard);

    static char decodeBoard(string a);

    static string getCell(const int file, const int rank);

    static bool isPieceAt(const uchar pieces, const uchar pos, const _Tchessboard &chessboard);

    static string decodeBoardinv(const uchar type, const int a, const int side);

    template<int side>
    static u64 getBitmap(const _Tchessboard &chessboard) {
        return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] |
               chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    static string boardToFen(const _Tchessboard &chessboard);

    template<int side>
    static int getPieceAt(const u64 bitmapPos, const _Tchessboard &chessboard) {
        if ((chessboard[PAWN_BLACK + side] & bitmapPos))return PAWN_BLACK + side;
        if ((chessboard[ROOK_BLACK + side] & bitmapPos))return ROOK_BLACK + side;
        if ((chessboard[BISHOP_BLACK + side] & bitmapPos))return BISHOP_BLACK + side;
        if ((chessboard[KNIGHT_BLACK + side] & bitmapPos))return KNIGHT_BLACK + side;
        if ((chessboard[KING_BLACK + side] & bitmapPos))return KING_BLACK + side;
        if ((chessboard[QUEEN_BLACK + side] & bitmapPos))return QUEEN_BLACK + side;
        return SQUARE_EMPTY;
    }

#ifdef DEBUG_MODE

    static u64 getBitmap(const int side, const _Tchessboard &chessboard);

    static int getPieceAt(int side, const u64 bitmapPos, const _Tchessboard &chessboard);

#endif

    template<int side, bool exitOnFirst>
    static u64 getAttackers(const int position, const u64 allpieces, const _Tchessboard &chessboard) {
        BENCH(Times::getInstance().start("getAttackers"))
        ASSERT_RANGE(position, 0, 63);
        ASSERT_RANGE(side, 0, 1);

        ///knight
        u64 attackers = KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers) {
            BENCH(Times::getInstance().stop("getAttackers"))
            return attackers;
        }
        ///king
        attackers |= NEAR_MASK1[position] & chessboard[KING_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers) {
            BENCH(Times::getInstance().stop("getAttackers"))
            return attackers;
        }
        ///pawn
        attackers |= PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers) {
            BENCH(Times::getInstance().stop("getAttackers"))
            return attackers;
        }
        ///bishop queen
        u64 enemies = chessboard[BISHOP_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        u64 nuovo = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
        for (; nuovo; RESET_LSB(nuovo)) {
            const int bound = BITScanForward(nuovo);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers) {
                BENCH(Times::getInstance().stop("getAttackers"))
                return attackers;
            }
        }
        enemies = chessboard[ROOK_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        nuovo = Bitboard::getRankFile(position, allpieces) & enemies;
        for (; nuovo; RESET_LSB(nuovo)) {
            const int bound = BITScanForward(nuovo);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers) {
                BENCH(Times::getInstance().stop("getAttackers"))
                return attackers;
            }
        }
        BENCH(Times::getInstance().stop("getAttackers"))
        return attackers;
    }

    template<int side>
    static bool isAttacked(const int position, const u64 allpieces, const _Tchessboard &chessboard) {
        return getAttackers<side, true>(position, allpieces, chessboard);
    }

    template<int side>
    static bool anyAttack(u64 sq, const u64 allpieces, const _Tchessboard &chessboard) {
        for (; sq; RESET_LSB(sq)) {
            const int position = BITScanForward(sq);
            if (getAttackers<side, true>(position, allpieces, chessboard))return true;
        }
        return false;
    }

    template<int side>
    static bool inCheck1(const _Tchessboard &chessboard) {
        return isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                getBitmap<BLACK>(chessboard) | getBitmap<WHITE>(chessboard), chessboard);
    }

    template<int side>
    static u64 getBitmapNoPawns(const _Tchessboard &chessboard) {
        BENCH(Times::getInstance().start("getBitmapNoPawns"))
        auto a = chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
                 chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
        BENCH(Times::getInstance().stop("getBitmapNoPawns"))
        return a;
    }


    template<int side>
    static u64 getPiecesNoKing(const _Tchessboard &chessboard) {
        return chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
               chessboard[PAWN_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    template<int side>
    static int getNpiecesNoPawnNoKing(const _Tchessboard &chessboard) {
        return bitCount(
                chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
                chessboard[QUEEN_BLACK + side]);
    }

    static string moveToString(const _Tmove *move);

};

