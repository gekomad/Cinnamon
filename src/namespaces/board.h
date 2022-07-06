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
#include "../util/Bitboard.h"

using namespace constants;

class board {
private:
    board() {}
public:
    [[gnu::pure]] static u64 colors(const int pos);

    static bool isOccupied(const uchar pos, const u64 allpieces);

    [[gnu::pure]] static int getFile(const char cc);

    static bool checkInsufficientMaterial(const int nPieces, const _Tchessboard &chessboard);

    static u64 performRankFileCaptureAndShift(const int position, const u64 enemies, const u64 allpieces);

    template<uchar side>
    static u64
    __attribute__((always_inline))  getPinned(const u64 allpieces, const u64 friends, const int kingPosition, const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getPinned")
        u64 result = 0;
        ASSERT_RANGE(kingPosition, 0, 63)
        const u64 *s = LINK_SQUARE[kingPosition];
        constexpr int xside = X(side);
        u64 attacked = DIAGONAL_ANTIDIAGONAL[kingPosition] &
                       (chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside]);
        attacked |=
                RANK_FILE[kingPosition] & (chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside]);
        for (; attacked; RESET_LSB(attacked)) {
            const int pos = BITScanForward(attacked);
            const u64 b = *(s + pos) & allpieces;
#ifndef NDEBUG
            u64 x = *(s + pos) & (allpieces & NOTPOW2(kingPosition));
            assert(b == x);
#endif
            if (!(b & (b - 1))) {
                result |= b & friends;
            }
        }
        return result;
    }

    static u64 getDiagShiftAndCapture(const int position, const u64 enemies, const u64 allpieces);

    [[gnu::pure]] static bool isCastleRight_WhiteKing(const uchar RIGHT_CASTLE);

    static u64 getMobilityRook(const int position, const u64 enemies, const u64 friends);

    [[gnu::pure]] static bool isCastleRight_BlackKing(const uchar RIGHT_CASTLE);

    [[gnu::pure]]static bool isCastleRight_WhiteQueen(const uchar RIGHT_CASTLE);

    [[gnu::pure]]static bool isCastleRight_BlackQueen(const uchar RIGHT_CASTLE);

    static bool isPieceAt(const uchar pieces, const uchar pos, const _Tchessboard &chessboard);

    template<uchar side>
    __attribute__((always_inline)) static u64 getBitmap(const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getBitmap")
        return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] |
               chessboard[KNIGHT_BLACK + side] | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }

    __attribute__((always_inline))  static u64 getBitmap(const _Tchessboard &chessboard) {
        return chessboard[PAWN_BLACK] | chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] |
               chessboard[KNIGHT_BLACK] | chessboard[KING_BLACK] | chessboard[QUEEN_BLACK] | chessboard[PAWN_WHITE] |
               chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] |
               chessboard[KNIGHT_WHITE] | chessboard[KING_WHITE] | chessboard[QUEEN_WHITE];
    }

    template<uchar side>
    __attribute__((always_inline))  static int getPieceAt(const u64 bitmapPos, const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getPieceAt")
        if ((chessboard[PAWN_BLACK + side] & bitmapPos))return PAWN_BLACK + side;
        if ((chessboard[ROOK_BLACK + side] & bitmapPos))return ROOK_BLACK + side;
        if ((chessboard[BISHOP_BLACK + side] & bitmapPos))return BISHOP_BLACK + side;
        if ((chessboard[KNIGHT_BLACK + side] & bitmapPos))return KNIGHT_BLACK + side;
        if ((chessboard[KING_BLACK + side] & bitmapPos))return KING_BLACK + side;
        if ((chessboard[QUEEN_BLACK + side] & bitmapPos))return QUEEN_BLACK + side;
        return SQUARE_EMPTY;
    }

#ifndef NDEBUG

    static u64 getBitmap(const uchar side, const _Tchessboard &chessboard);

    static int getPieceAt(uchar side, const u64 bitmapPos, const _Tchessboard &chessboard);

#endif

    template<uchar side>
    __attribute__((always_inline))  static u64 getAttackers(const int position, const u64 allpieces, const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getAttackers")
        ASSERT_RANGE(position, 0, 63)
        ASSERT_RANGE(side, 0, 1)
        constexpr int xside = X(side);
        ///knight
        u64 attackers = KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + xside];

        ///king
        attackers |= NEAR_MASK1[position] & chessboard[KING_BLACK + xside];

        ///pawn
        attackers |= PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + xside];

        ///bishop queen
        u64 enemies = chessboard[BISHOP_BLACK + xside] | chessboard[QUEEN_BLACK + xside];
        u64 n = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
        for (; n; RESET_LSB(n)) {
            attackers |= POW2(BITScanForward(n));
        }
        enemies = chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside];
        n = Bitboard::getRankFile(position, allpieces) & enemies;
        for (; n; RESET_LSB(n)) {
            attackers |= POW2(BITScanForward(n));
        }

        return attackers;
    }

    static bool isAttacked(const uchar side, const int position, const u64 allpieces, const _Tchessboard &chessboard) {
        if (side == WHITE)return isAttacked < WHITE > (position, allpieces, chessboard);
        return isAttacked < BLACK > (position, allpieces, chessboard);
    }

    template<uchar side>
    __attribute__((always_inline))  static bool isAttacked(const int position, const u64 allpieces, const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("isAttacked")
        ASSERT_RANGE(position, 0, 63)
        ASSERT_RANGE(side, 0, 1)
        constexpr int xside = X(side);
        ///knight
        if (KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + xside]) {
            return true;
        }

        ///king
        if (NEAR_MASK1[position] & chessboard[KING_BLACK + xside]) {
            return true;
        }
        ///pawn
        if (PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + xside]) {
            return true;
        }

        u64 enemies = chessboard[BISHOP_BLACK + xside] | chessboard[QUEEN_BLACK + xside];
        if ((DIAGONAL_ANTIDIAGONAL[position] & enemies)) {
            ///bishop queen
            if (Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies) {
                return true;
            }
        }

        enemies = chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside];
        if (!(RANK_FILE[position] & enemies)) return false;
        if (Bitboard::getRankFile(position, allpieces) & enemies) {
            return true;
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) static bool anyAttack(u64 sq, const u64 allpieces, const _Tchessboard &chessboard) {
        for (; sq; RESET_LSB(sq)) {
            if (isAttacked<side>(BITScanForward(sq), allpieces, chessboard))return true;
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) static bool inCheck1(const _Tchessboard &chessboard) {
        return isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                getBitmap<BLACK>(chessboard) | getBitmap<WHITE>(chessboard), chessboard);
    }

    template<uchar side>
    __attribute__((always_inline)) static u64 getBitmapNoPawnsNoKing(const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getBitmapNoPawns")
        return chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
               chessboard[QUEEN_BLACK + side];
    }


    template<uchar side>
    __attribute__((always_inline))  static u64 getPiecesNoKing(const _Tchessboard &chessboard) {
        BENCH_AUTO_CLOSE("getPiecesNoKing")
        return chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] |
               chessboard[PAWN_BLACK + side] | chessboard[QUEEN_BLACK + side];
    }


};

