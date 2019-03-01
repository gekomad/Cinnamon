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

#include "GenMoves.h"
#include <fstream>
#include <string.h>
#include <iomanip>

using namespace _board;

class Eval: public GenMoves {

public:

    Eval();

    virtual ~Eval();

    short getScore(const u64 key, const int side, const int N_PIECE, const int alpha, const int beta, const bool trace);

    template<int side>
    int lazyEval() {
        return lazyEvalSide<side>() - lazyEvalSide<side ^ 1>();
    }

#ifdef DEBUG_MODE
    unsigned lazyEvalCuts;
#endif
protected:
    STATIC_CONST int FUTIL_MARGIN = 154;
    STATIC_CONST int EXT_FUTILY_MARGIN = 392;
    STATIC_CONST int RAZOR_MARGIN = 1071;
    STATIC_CONST int ATTACK_KING = 30;
    STATIC_CONST int BISHOP_ON_QUEEN = 2;
    STATIC_CONST int BACKWARD_PAWN = 2;
    STATIC_CONST int NO_PAWNS = 15;
    STATIC_CONST int DOUBLED_ISOLATED_PAWNS = 14;
    STATIC_CONST int DOUBLED_PAWNS = 5;
    STATIC_CONST int ENEMIES_PAWNS_ALL = 8;
    STATIC_CONST int PAWN_IN_7TH = 32;
    STATIC_CONST int PAWN_CENTER = 15;
    STATIC_CONST int PAWN_IN_8TH = 114;
    STATIC_CONST int PAWN_ISOLATED = 3;
    STATIC_CONST int PAWN_NEAR_KING = 2;
    STATIC_CONST int PAWN_BLOCKED = 5;
    STATIC_CONST int UNPROTECTED_PAWNS = 5;
    STATIC_CONST int ENEMY_NEAR_KING = 2;
    STATIC_CONST int FRIEND_NEAR_KING = 1;
    STATIC_CONST int BISHOP_NEAR_KING = 10;
    STATIC_CONST int HALF_OPEN_FILE_Q = 3;
    STATIC_CONST int KNIGHT_TRAPPED = 5;
    STATIC_CONST int END_OPENING = 6;
    STATIC_CONST int BONUS2BISHOP = 18;
    STATIC_CONST int BISHOP_PAWN_ON_SAME_COLOR = 5;
    STATIC_CONST int CONNECTED_ROOKS = 7;
    STATIC_CONST int OPEN_FILE = 10;
    STATIC_CONST int OPEN_FILE_Q = 3;
    STATIC_CONST int ROOK_7TH_RANK = 10;
    STATIC_CONST int ROOK_BLOCKED = 13;
    STATIC_CONST int ROOK_TRAPPED = 6;
    STATIC_CONST int UNDEVELOPED_KNIGHT = 4;
    STATIC_CONST int UNDEVELOPED_BISHOP = 4;
#ifdef DEBUG_MODE
    typedef struct {
        double BAD_BISHOP[2];
        double MOB_BISHOP[2];
        double UNDEVELOPED_BISHOP[2];
        double OPEN_DIAG_BISHOP[2];
        double BONUS2BISHOP[2];
        double BISHOP_PAWN_ON_SAME_COLOR[2];

        double ATTACK_KING_PAWN[2];
        double PAWN_CENTER[2];
        double PAWN_7H[2];
        double PAWN_IN_8TH[2];
        double PAWN_BLOCKED[2];
        double UNPROTECTED_PAWNS[2];
        double PAWN_ISOLATED[2];
        double DOUBLED_PAWNS[2];
        double DOUBLED_ISOLATED_PAWNS[2];
        double BACKWARD_PAWN[2];
        double FORK_SCORE[2];
        double PAWN_PASSED[2];
        double ENEMIES_PAWNS_ALL[2];
        double NO_PAWNS[2];

        double KING_SECURITY_BISHOP[2];
        double KING_SECURITY_QUEEN[2];
        double KING_SECURITY_KNIGHT[2];
        double KING_SECURITY_ROOK[2];
        double DISTANCE_KING[2];
        double END_OPENING_KING[2];
        double PAWN_NEAR_KING[2];
        double MOB_KING[2];

        double MOB_QUEEN[2];
        double OPEN_FILE_Q[2];
        double BISHOP_ON_QUEEN[2];
        double HALF_OPEN_FILE_Q[2];

        double UNDEVELOPED_KNIGHT[2];
        double KNIGHT_TRAPPED[2];
        double MOB_KNIGHT[2];


        double ROOK_7TH_RANK[2];
        double ROOK_TRAPPED[2];
        double MOB_ROOK[2];
        double ROOK_BLOCKED[2];
        double ROOK_OPEN_FILE[2];
        double CONNECTED_ROOKS[2];
    } _TSCORE_DEBUG;
    _TSCORE_DEBUG SCORE_DEBUG[2];
#endif

private:
    static constexpr int hashSize = 65536;
    static constexpr u64 keyMask = 0xffffffffffff0000ULL;
    static constexpr u64 valueMask = 0xffffULL;
    static constexpr short noHashValue = (short) 0xffff;

    static constexpr char MG = 0;
    static constexpr char EG = 1;

    static u64 *evalHash;

    inline void storeHashValue(const u64 key, const short value);

    inline short getHashValue(const u64 key) const;

    typedef struct {
        double pawns[2];
        double bishop[2];
        double queens[2];
        double rooks[2];
        double knights[2];
        double kings[2];
    } _Tresult;

#ifdef DEBUG_MODE
    int evaluationCount[2];

    void p() {
        cout << "|\t      \t      \t|";
    }

    void p(double d1, double d2) {
        std::cout << std::fixed;
        std::cout << std::setprecision(2);
        std::cout << "\t";
        std::cout << d1 / 100.0;
        std::cout << "\t";
        std::cout << d2 / 100.0;
        std::cout << "\t|";
    }
#endif


    void getScores(_Tresult res[2]) {
        {
            const auto x1 = evaluatePawn<BLACK>();
            res[MG].pawns[BLACK] = x1.first;
            res[EG].pawns[BLACK] = x1.second;

            const auto x2 = evaluatePawn<WHITE>();
            res[MG].pawns[WHITE] = x2.first;
            res[EG].pawns[WHITE] = x2.second;
        }

        {
            const auto x1 = evaluateBishop<BLACK>(structureEval.allPiecesSide[WHITE]);
            res[MG].bishop[BLACK] = x1.first;
            res[EG].bishop[BLACK] = x1.second;

            const auto x2 = evaluateBishop<WHITE>(structureEval.allPiecesSide[BLACK]);
            res[MG].bishop[WHITE] = x2.first;
            res[EG].bishop[WHITE] = x2.second;
        }

        {
            const auto x1 = evaluateQueen<BLACK>(structureEval.allPiecesSide[WHITE]);
            res[MG].queens[BLACK] = x1.first;
            res[EG].queens[BLACK] = x1.second;

            const auto x2 = evaluateQueen<WHITE>(structureEval.allPiecesSide[BLACK]);
            res[MG].queens[WHITE] = x2.first;
            res[EG].queens[WHITE] = x2.second;
        }

        {
            const auto x1 = evaluateRook<BLACK>(chessboard[KING_BLACK],
                                                structureEval.allPiecesSide[WHITE],
                                                structureEval.allPiecesSide[BLACK]);
            res[MG].rooks[BLACK] = x1.first;
            res[EG].rooks[BLACK] = x1.second;

            const auto x2 = evaluateRook<WHITE>(chessboard[KING_WHITE],
                                                structureEval.allPiecesSide[BLACK],
                                                structureEval.allPiecesSide[WHITE]);
            res[MG].rooks[WHITE] = x2.first;
            res[EG].rooks[WHITE] = x2.second;
        }

        {
            const auto x1 = evaluateKnight<BLACK>(chessboard[WHITE], ~structureEval.allPiecesSide[BLACK]);
            res[MG].knights[BLACK] = x1.first;
            res[EG].knights[BLACK] = x1.second;

            const auto x2 = evaluateKnight<WHITE>(chessboard[BLACK], ~structureEval.allPiecesSide[WHITE]);
            res[MG].knights[WHITE] = x2.first;
            res[EG].knights[WHITE] = x2.second;

        }

        {
            const auto x1 = evaluateKing(BLACK, ~structureEval.allPiecesSide[BLACK]);
            res[MG].kings[BLACK] = x1.first;
            res[EG].kings[BLACK] = x1.second;

            const auto x2 = evaluateKing(WHITE, ~structureEval.allPiecesSide[WHITE]);
            res[MG].kings[WHITE] = x2.first;
            res[EG].kings[WHITE] = x2.second;
        }
    }

    template<int side>
    void openFile();

    template<int side>
    pair<short, short> evaluatePawn();

    template<int side>
    pair<short, short> evaluateBishop(const u64);

    template<int side>
    pair<short, short> evaluateQueen(const u64 enemies);

    template<int side>
    pair<short, short> evaluateKnight(const u64, const u64);

    template<int side>
    pair<short, short> evaluateRook(const u64, u64 enemies, u64 friends);

    pair<short, short> evaluateKing(int side, u64 squares);

    template<int side>
    int lazyEvalSide() {
        return bitCount(chessboard[PAWN_BLACK + side]) * VALUEPAWN +
            bitCount(chessboard[ROOK_BLACK + side]) * VALUEROOK +
            bitCount(chessboard[BISHOP_BLACK + side]) * VALUEBISHOP +
            bitCount(chessboard[KNIGHT_BLACK + side]) * VALUEKNIGHT +
            bitCount(chessboard[QUEEN_BLACK + side]) * VALUEQUEEN;
    }

    void generateLinkRook();
};

namespace _eval {

    constexpr int BISHOP_OUTPOST[2][64] = {
        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 3, 3, 3, 3, 1, 0,
         0, 3, 5, 5, 5, 5, 3, 0,
         0, 1, 2, 2, 2, 2, 1, 0,
         0, 0, 1, 1, 1, 1, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 1, 1, 1, 1, 0, 0,
         0, 1, 2, 2, 2, 2, 1, 0,
         0, 3, 5, 5, 5, 5, 3, 0,
         0, 1, 3, 3, 3, 3, 1, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0}
    };

    constexpr int KNIGHT_OUTPOST[2][64] = {
        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 4, 4, 4, 4, 1, 0,
         0, 2, 6, 8, 8, 6, 2, 0,
         0, 1, 4, 4, 4, 4, 1, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0},

        {0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 1, 4, 4, 4, 4, 1, 0,
         0, 2, 6, 8, 8, 6, 2, 0,
         0, 1, 4, 4, 4, 4, 1, 0,
         0, 0, 0, 0, 0, 0, 0, 0,
         0, 0, 0, 0, 0, 0, 0, 0}
    };
    static constexpr int
        MOB_QUEEN[2][29] = {{0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                            {-15, -11, -7, 0, 2, 5, 5, 10, 11, 12, 15, 18, 28, 30, 32, 34, 37, 42, 43, 45, 46, 47, 48,
                             50, 52, 52, 53, 55, 57}};

    static constexpr int MOB_ROOK[2][15] = {{-1, 0, 1, 4, 5, 6, 7, 9, 12, 14, 19, 22, 23, 24, 25},
                                            {-12, -9, 3, 4, 9, 10, 15, 21, 29, 31, 40, 45, 50, 51, 52}
    };

    static constexpr int MOB_KNIGHT[9] = {-8,
                                          -4,
                                          7,
                                          10,
                                          15,
                                          20,
                                          30,
                                          35,
                                          40};

    static constexpr int MOB_BISHOP[2][14] = {{-8, -7, 2, 8, 9, 10, 15, 20, 28, 30, 40, 45, 50, 50},
                                              {-20, -10, -4, 0, 5, 10, 15, 20, 28, 30, 40, 45, 50, 50}};

    static constexpr int MOB_KING[2][9] = {{1, 2, 2, 1, 0, 0, 0, 0, 0},
                                           {-25, 15, 5, 7, 15, 20, 25, 26, 30}};

    static constexpr int MOB_CASTLE[2][3] = {{-50, 30, 50},
                                             {-1, 10, 10}};

    static constexpr int BONUS_ATTACK_KING[18] = {-1, 2, 8, 64, 128, 512, 512, 512, 512, 512, 512, 512, 512, 512, 512,
                                                  512, 512, 512};

    static constexpr u64 PAWN_PROTECTED_MASK[2][64] =
        {{0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL, 0x20000ULL, 0x50000ULL,
          0xa0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL, 0x2000000ULL, 0x5000000ULL,
          0xa000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL, 0x200000000ULL,
          0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL, 0xa000000000ULL,
          0x4000000000ULL, 0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL, 0x280000000000ULL,
          0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL, 0x2000000000000ULL, 0x5000000000000ULL,
          0xa000000000000ULL, 0x14000000000000ULL, 0x28000000000000ULL, 0x50000000000000ULL, 0xa0000000000000ULL,
          0x40000000000000ULL, 0xFF000000000000ULL, 0xFF000000000000ULL, 0xFF000000000000ULL, 0xFF000000000000ULL,
          0xFF000000000000ULL, 0xFF000000000000ULL, 0xFF000000000000ULL, 0xFF000000000000ULL, 0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0, 0xFF00ULL, 0xFF00ULL, 0xFF00ULL, 0xFF00ULL, 0xFF00ULL, 0xFF00ULL, 0xFF00ULL,
          0xFF00ULL, 0x200ULL, 0x500ULL, 0xa00ULL, 0x1400ULL, 0x2800ULL, 0x5000ULL, 0xa000ULL, 0x4000ULL, 0x20000ULL,
          0x50000ULL, 0xa0000ULL, 0x140000ULL, 0x280000ULL, 0x500000ULL, 0xa00000ULL, 0x400000ULL, 0x2000000ULL,
          0x5000000ULL, 0xa000000ULL, 0x14000000ULL, 0x28000000ULL, 0x50000000ULL, 0xa0000000ULL, 0x40000000ULL,
          0x200000000ULL, 0x500000000ULL, 0xa00000000ULL, 0x1400000000ULL, 0x2800000000ULL, 0x5000000000ULL,
          0xa000000000ULL, 0x4000000000ULL, 0x20000000000ULL, 0x50000000000ULL, 0xa0000000000ULL, 0x140000000000ULL,
          0x280000000000ULL, 0x500000000000ULL, 0xa00000000000ULL, 0x400000000000ULL, 0xFF00000000000000ULL,
          0xFF00000000000000ULL, 0xFF00000000000000ULL, 0xFF00000000000000ULL, 0xFF00000000000000ULL,
          0xFF00000000000000ULL, 0xFF00000000000000ULL, 0xFF00000000000000ULL}};

    static constexpr u64 PAWN_BACKWARD_MASK[2][64] =
        {{0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 131584ULL, 328960ULL, 657920ULL, 1315840ULL, 2631680ULL,
          5263360ULL, 10526720ULL, 4210688ULL, 33685504ULL, 84213760ULL, 168427520ULL, 336855040ULL, 673710080ULL,
          1347420160ULL, 2694840320ULL, 1077936128ULL, 8623489024ULL, 21558722560ULL, 43117445120ULL, 86234890240ULL,
          172469780480ULL, 344939560960ULL, 689879121920ULL, 275951648768ULL, 2207613190144ULL, 5519032975360ULL,
          11038065950720ULL, 22076131901440ULL, 44152263802880ULL, 88304527605760ULL, 176609055211520ULL,
          70643622084608ULL, 565148976676864ULL, 1412872441692160ULL, 2825744883384320ULL, 5651489766768640ULL,
          11302979533537280ULL, 22605959067074560ULL, 45211918134149120ULL, 18084767253659648ULL, 562949953421312ULL,
          1407374883553280ULL, 2814749767106560ULL, 5629499534213120ULL, 11258999068426240ULL, 22517998136852480ULL,
          45035996273704960ULL, 18014398509481984ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0},
         {0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 512ULL, 1280ULL, 2560ULL, 5120ULL, 10240ULL, 20480ULL,
          40960ULL, 16384ULL, 131584ULL, 328960ULL, 657920ULL, 1315840ULL, 2631680ULL, 5263360ULL, 10526720ULL,
          4210688ULL, 33685504ULL, 84213760ULL, 168427520ULL, 336855040ULL, 673710080ULL, 1347420160ULL, 2694840320ULL,
          1077936128ULL, 8623489024ULL, 21558722560ULL, 43117445120ULL, 86234890240ULL, 172469780480ULL,
          344939560960ULL, 689879121920ULL, 275951648768ULL, 2207613190144ULL, 5519032975360ULL, 11038065950720ULL,
          22076131901440ULL, 44152263802880ULL, 88304527605760ULL, 176609055211520ULL, 70643622084608ULL,
          565148976676864ULL, 1412872441692160ULL, 2825744883384320ULL, 5651489766768640ULL, 11302979533537280ULL,
          22605959067074560ULL, 45211918134149120ULL, 18084767253659648ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL, 0ULL,
          0}};

    static constexpr u64 PAWN_PASSED_MASK[2][64] =
        {{0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x3ULL, 0x7ULL, 0xeULL, 0x1cULL, 0x38ULL,
          0x70ULL, 0xe0ULL, 0xc0ULL, 0x303ULL, 0x707ULL, 0xe0eULL, 0x1c1cULL, 0x3838ULL, 0x7070ULL, 0xe0e0ULL,
          0xc0c0ULL, 0x30303ULL, 0x70707ULL, 0xe0e0eULL, 0x1c1c1cULL, 0x383838ULL, 0x707070ULL, 0xe0e0e0ULL,
          0xc0c0c0ULL, 0x3030303ULL, 0x7070707ULL, 0xe0e0e0eULL, 0x1c1c1c1cULL, 0x38383838ULL, 0x70707070ULL,
          0xe0e0e0e0ULL, 0xc0c0c0c0ULL, 0x303030303ULL, 0x707070707ULL, 0xe0e0e0e0eULL, 0x1c1c1c1c1cULL,
          0x3838383838ULL, 0x7070707070ULL, 0xe0e0e0e0e0ULL, 0xc0c0c0c0c0ULL, 0x30303030303ULL, 0x70707070707ULL,
          0xe0e0e0e0e0eULL, 0x1c1c1c1c1c1cULL, 0x383838383838ULL, 0x707070707070ULL, 0xe0e0e0e0e0e0ULL,
          0xc0c0c0c0c0c0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0},
         {0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x303030303030000ULL, 0x707070707070000ULL,
          0xe0e0e0e0e0e0000ULL, 0x1c1c1c1c1c1c0000ULL, 0x3838383838380000ULL, 0x7070707070700000ULL,
          0xe0e0e0e0e0e00000ULL, 0xc0c0c0c0c0c00000ULL, 0x303030303000000ULL, 0x707070707000000ULL,
          0xe0e0e0e0e000000ULL, 0x1c1c1c1c1c000000ULL, 0x3838383838000000ULL, 0x7070707070000000ULL,
          0xe0e0e0e0e0000000ULL, 0xc0c0c0c0c0000000ULL, 0x303030300000000ULL, 0x707070700000000ULL,
          0xe0e0e0e00000000ULL, 0x1c1c1c1c00000000ULL, 0x3838383800000000ULL, 0x7070707000000000ULL,
          0xe0e0e0e000000000ULL, 0xc0c0c0c000000000ULL, 0x303030000000000ULL, 0x707070000000000ULL,
          0xe0e0e0000000000ULL, 0x1c1c1c0000000000ULL, 0x3838380000000000ULL, 0x7070700000000000ULL,
          0xe0e0e00000000000ULL, 0xc0c0c00000000000ULL, 0x303000000000000ULL, 0x707000000000000ULL,
          0xe0e000000000000ULL, 0x1c1c000000000000ULL, 0x3838000000000000ULL, 0x7070000000000000ULL,
          0xe0e0000000000000ULL, 0xc0c0000000000000ULL, 0x300000000000000ULL, 0x700000000000000ULL,
          0xe00000000000000ULL, 0x1c00000000000000ULL, 0x3800000000000000ULL, 0x7000000000000000ULL,
          0xe000000000000000ULL, 0xc000000000000000ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0ULL, 0x0}};

    static constexpr short PAWN_PASSED[2][64] = {{0, 0, 0, 0, 0, 0, 0, 0,
                                                  200, 200, 200, 200, 200, 200, 200, 200, 130, 130, 130, 130, 130, 130,
                                                  130, 130, 45, 45, 45, 45, 45, 45, 45, 45, 12, 12, 12, 12, 12, 12, 12,
                                                  12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                  0, 0},
                                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                                  0,
                                                  12, 12, 12, 12, 12, 12, 12, 12, 45, 45, 45, 45, 45, 45, 45, 45,
                                                  130, 130, 130, 130, 130, 130, 130, 130, 200, 200, 200, 200, 200, 200,
                                                  200, 200, 0, 0, 0, 0, 0, 0, 0, 0}};

    static constexpr u64 PAWN_ISOLATED_MASK[64] = {0x202020202020202ULL, 0x505050505050505ULL, 0xA0A0A0A0A0A0A0AULL,
                                                   0x1414141414141414ULL, 0x2828282828282828ULL, 0x5050505050505050ULL,
                                                   0xA0A0A0A0A0A0A0A0ULL, 0x4040404040404040ULL, 0x202020202020202ULL,
                                                   0x505050505050505ULL, 0xA0A0A0A0A0A0A0AULL, 0x1414141414141414ULL,
                                                   0x2828282828282828ULL, 0x5050505050505050ULL, 0xA0A0A0A0A0A0A0A0ULL,
                                                   0x4040404040404040ULL, 0x202020202020202ULL, 0x505050505050505ULL,
                                                   0xA0A0A0A0A0A0A0AULL, 0x1414141414141414ULL, 0x2828282828282828ULL,
                                                   0x5050505050505050ULL, 0xA0A0A0A0A0A0A0A0ULL, 0x4040404040404040ULL,
                                                   0x202020202020202ULL, 0x505050505050505ULL, 0xA0A0A0A0A0A0A0AULL,
                                                   0x1414141414141414ULL, 0x2828282828282828ULL, 0x5050505050505050ULL,
                                                   0xA0A0A0A0A0A0A0A0ULL, 0x4040404040404040ULL, 0x202020202020202ULL,
                                                   0x505050505050505ULL, 0xA0A0A0A0A0A0A0AULL, 0x1414141414141414ULL,
                                                   0x2828282828282828ULL, 0x5050505050505050ULL, 0xA0A0A0A0A0A0A0A0ULL,
                                                   0x4040404040404040ULL, 0x202020202020202ULL, 0x505050505050505ULL,
                                                   0xA0A0A0A0A0A0A0AULL, 0x1414141414141414ULL, 0x2828282828282828ULL,
                                                   0x5050505050505050ULL, 0xA0A0A0A0A0A0A0A0ULL,
                                                   0x4040404040404040ULL, 0x202020202020202ULL, 0x505050505050505ULL,
                                                   0xA0A0A0A0A0A0A0AULL, 0x1414141414141414ULL, 0x2828282828282828ULL,
                                                   0x5050505050505050ULL, 0xA0A0A0A0A0A0A0A0ULL, 0x4040404040404040ULL,
                                                   0x202020202020202ULL, 0x505050505050505ULL, 0xA0A0A0A0A0A0A0AULL,
                                                   0x1414141414141414ULL, 0x2828282828282828ULL, 0x5050505050505050ULL,
                                                   0xA0A0A0A0A0A0A0A0ULL, 0x4040404040404040ULL};

    static constexpr char DISTANCE_KING_OPENING[64] = {-8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8,
                                                       -8, -8, -12, -12, -12, -12, -8, -8, -8, -8, -12, -16, -16, -12,
                                                       -8, -8, -8, -8, -12, -16, -16, -12, -8, -8, -8, -8, -12, -12,
                                                       -12, -12, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8, -8,
                                                       -8, -8, -8, -8};

    static constexpr char DISTANCE_KING_ENDING[64] = {12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                                                      12, 12, 16, 16, 16, 16, 12, 12, 12, 12, 16, 20, 20, 16, 12, 12,
                                                      12, 12, 16, 20, 20, 16, 12, 12, 12, 12, 16, 16, 16, 16, 12, 12,
                                                      12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12};

    static constexpr u64 LINK_ROOKS[64][64] = {
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2ULL, 0x6ULL, 0xeULL, 0x1eULL, 0x3eULL, 0x7eULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10100ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x101010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101010100ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010101010100ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4ULL, 0xcULL, 0x1cULL, 0x3cULL, 0x7cULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020200ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020202020200ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0x2ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8ULL, 0x18ULL, 0x38ULL, 0x78ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040400ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040400ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040404040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0x6ULL, 0x4ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10ULL, 0x30ULL, 0x70ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080800ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080800ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808080800ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xeULL, 0xcULL, 0x8ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20ULL, 0x60ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1010101000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101010101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0x1eULL, 0x1cULL, 0x18ULL, 0x10ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x40ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x202020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x20202020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0x3eULL, 0x3cULL, 0x38ULL, 0x30ULL, 0x20ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040404000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040404000ULL, 0xffffffffffffffffULL},
        {0x7eULL, 0x7cULL, 0x78ULL, 0x70ULL, 0x60ULL, 0x40ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x80808000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080808000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080808000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200ULL, 0x600ULL, 0xe00ULL, 0x1e00ULL, 0x3e00ULL, 0x7e00ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x10101010000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010101010000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400ULL, 0xc00ULL, 0x1c00ULL, 0x3c00ULL,
         0x7c00ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202020000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2020202020000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800ULL, 0x1800ULL, 0x3800ULL, 0x7800ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x600ULL, 0x400ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000ULL, 0x3000ULL, 0x7000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x808080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808080000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xe00ULL, 0xc00ULL,
         0x800ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000ULL, 0x6000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10100000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010100000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x101010100000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10101010100000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1e00ULL,
         0x1c00ULL, 0x1800ULL, 0x1000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x4000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x200000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020200000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020200000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202020200000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x3e00ULL,
         0x3c00ULL, 0x3800ULL, 0x3000ULL, 0x2000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040400000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040400000ULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7e00ULL,
         0x7c00ULL, 0x7800ULL, 0x7000ULL, 0x6000ULL, 0x4000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080800000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080800000ULL},
        {0x100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20000ULL, 0x60000ULL, 0xe0000ULL, 0x1e0000ULL, 0x3e0000ULL, 0x7e0000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1010101000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0x200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000ULL, 0xc0000ULL, 0x1c0000ULL, 0x3c0000ULL, 0x7c0000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020202000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000ULL, 0x180000ULL, 0x380000ULL, 0x780000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x404000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x60000ULL, 0x40000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000ULL, 0x300000ULL, 0x700000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x80808000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xe0000ULL, 0xc0000ULL, 0x80000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000ULL, 0x600000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x10101010000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1e0000ULL, 0x1c0000ULL, 0x180000ULL,
         0x100000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202020000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x3e0000ULL, 0x3c0000ULL, 0x380000ULL,
         0x300000ULL, 0x200000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040000000ULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7e0000ULL, 0x7c0000ULL, 0x780000ULL,
         0x700000ULL, 0x600000ULL, 0x400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080000000ULL},
        {0x10100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000ULL,
         0x6000000ULL, 0xe000000ULL, 0x1e000000ULL, 0x3e000000ULL, 0x7e000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10100000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1010100000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0x20200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4000000ULL, 0xc000000ULL, 0x1c000000ULL, 0x3c000000ULL, 0x7c000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x200000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020200000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8000000ULL, 0x18000000ULL, 0x38000000ULL, 0x78000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x400000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40400000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040400000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x6000000ULL, 0x4000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000ULL, 0x30000000ULL, 0x70000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080800000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xe000000ULL, 0xc000000ULL, 0x8000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000ULL, 0x60000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x10101000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1e000000ULL, 0x1c000000ULL, 0x18000000ULL, 0x10000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x3e000000ULL, 0x3c000000ULL, 0x38000000ULL, 0x30000000ULL,
         0x20000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404000000000ULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7e000000ULL, 0x7c000000ULL, 0x78000000ULL, 0x70000000ULL,
         0x60000000ULL, 0x40000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x808000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x80808000000000ULL},
        {0x1010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000000ULL, 0x600000000ULL,
         0xe00000000ULL, 0x1e00000000ULL, 0x3e00000000ULL, 0x7e00000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0x2020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400000000ULL,
         0xc00000000ULL, 0x1c00000000ULL, 0x3c00000000ULL, 0x7c00000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x200000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x800000000ULL, 0x1800000000ULL, 0x3800000000ULL, 0x7800000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x600000000ULL, 0x400000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1000000000ULL, 0x3000000000ULL, 0x7000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10100000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xe00000000ULL, 0xc00000000ULL, 0x800000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000ULL, 0x6000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10100000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1e00000000ULL, 0x1c00000000ULL, 0x1800000000ULL, 0x1000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x3e00000000ULL, 0x3c00000000ULL, 0x3800000000ULL, 0x3000000000ULL, 0x2000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x400000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x40400000000000ULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x80000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x7e00000000ULL, 0x7c00000000ULL, 0x7800000000ULL, 0x7000000000ULL, 0x6000000000ULL,
         0x4000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x800000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x80800000000000ULL},
        {0x101010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x100000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000000ULL, 0x60000000000ULL, 0xe0000000000ULL,
         0x1e0000000000ULL, 0x3e0000000000ULL, 0x7e0000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0x202020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000000ULL, 0xc0000000000ULL,
         0x1c0000000000ULL, 0x3c0000000000ULL, 0x7c0000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x404040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000000000ULL, 0x180000000000ULL,
         0x380000000000ULL, 0x780000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x808000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x60000000000ULL,
         0x40000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000000000ULL,
         0x300000000000ULL, 0x700000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010100000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xe0000000000ULL,
         0xc0000000000ULL, 0x80000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x200000000000ULL, 0x600000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x10000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020200000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1e0000000000ULL,
         0x1c0000000000ULL, 0x180000000000ULL, 0x100000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x400000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x3e0000000000ULL,
         0x3c0000000000ULL, 0x380000000000ULL, 0x300000000000ULL, 0x200000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x40000000000000ULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7e0000000000ULL,
         0x7c0000000000ULL, 0x780000000000ULL, 0x700000000000ULL, 0x600000000000ULL, 0x400000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x80000000000000ULL},
        {0x10101010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101010000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10101000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x10100000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2000000000000ULL, 0x6000000000000ULL, 0xe000000000000ULL, 0x1e000000000000ULL,
         0x3e000000000000ULL, 0x7e000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0x20202020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202020000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20202000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20200000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000000000ULL, 0xc000000000000ULL, 0x1c000000000000ULL,
         0x3c000000000000ULL, 0x7c000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x40404040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40400000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000000000ULL, 0x18000000000000ULL, 0x38000000000000ULL,
         0x78000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x80808000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x6000000000000ULL, 0x4000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x10000000000000ULL, 0x30000000000000ULL,
         0x70000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010101000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010100000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x101010000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x101000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x100000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xe000000000000ULL, 0xc000000000000ULL,
         0x8000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x20000000000000ULL,
         0x60000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x202020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020200000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202020000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x202000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x200000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1e000000000000ULL, 0x1c000000000000ULL,
         0x18000000000000ULL, 0x10000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x40000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040404000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x404040400000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404040000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x404000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x400000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x3e000000000000ULL, 0x3c000000000000ULL,
         0x38000000000000ULL, 0x30000000000000ULL, 0x20000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080808000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808080800000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x808080000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x808000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x7e000000000000ULL, 0x7c000000000000ULL,
         0x78000000000000ULL, 0x70000000000000ULL, 0x60000000000000ULL, 0x40000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL},
        {0x1010101010100ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010101010000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010101000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x1010100000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1010000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x200000000000000ULL, 0x600000000000000ULL, 0xe00000000000000ULL, 0x1e00000000000000ULL, 0x3e00000000000000ULL,
         0x7e00000000000000ULL},
        {0xffffffffffffffffULL, 0x2020202020200ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020202020000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020202000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2020200000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2020000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x2000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x400000000000000ULL, 0xc00000000000000ULL, 0x1c00000000000000ULL,
         0x3c00000000000000ULL, 0x7c00000000000000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404040400ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4040404040000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040404000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040400000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4040000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x4000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x200000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x800000000000000ULL, 0x1800000000000000ULL,
         0x3800000000000000ULL, 0x7800000000000000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808080800ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080808080000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x8080808000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080800000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8080000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x8000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x600000000000000ULL, 0x400000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x1000000000000000ULL,
         0x3000000000000000ULL, 0x7000000000000000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10101010101000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10101010100000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10101010000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10101000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10100000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x10000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xe00000000000000ULL, 0xc00000000000000ULL, 0x800000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x2000000000000000ULL, 0x6000000000000000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202020202000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202020200000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202020000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20202000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20200000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0x20000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x1e00000000000000ULL, 0x1c00000000000000ULL, 0x1800000000000000ULL, 0x1000000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x4000000000000000ULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040404000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040400000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404040000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40404000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40400000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x40000000000000ULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x3e00000000000000ULL, 0x3c00000000000000ULL, 0x3800000000000000ULL, 0x3000000000000000ULL,
         0x2000000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL},
        {0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080808000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080800000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808080000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80808000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80800000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0x80000000000000ULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL,
         0x7e00000000000000ULL, 0x7c00000000000000ULL, 0x7800000000000000ULL, 0x7000000000000000ULL,
         0x6000000000000000ULL, 0x4000000000000000ULL, 0xffffffffffffffffULL, 0xffffffffffffffffULL}
    };

}
