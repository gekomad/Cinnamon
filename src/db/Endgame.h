/*
    Cinnamon is a UCI chess engine
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

#include "../ChessBoard.h"

#ifndef NDEBUG

#include <unordered_map>

#endif

class Endgame {

public:

    static bool win(const uchar side, const int nPieces, const _Tchessboard &chessboard) {
        switch (nPieces) {
            case 3 :
                if (chessboard[QUEEN_BLACK + side])return true;
                if (chessboard[ROOK_BLACK + side])return true;
            case 4:
                // KBBK
                if (bitCount(chessboard[BISHOP_BLACK + side]) == 2)return true;
                // KBNK
                if (chessboard[BISHOP_BLACK + side] && chessboard[KNIGHT_BLACK + side])return true;
                // KBKP
                // KBPK
                // KNKP
                // KNPK
                // KQBK
                if (chessboard[QUEEN_BLACK + side] && chessboard[BISHOP_BLACK + side])return true;
                // KQKB
                // KQKN
                // KQNK
                if (chessboard[QUEEN_BLACK + side] && chessboard[KNIGHT_BLACK + side])return true;
                // KQPK
                // KQQK
                if (bitCount(chessboard[QUEEN_BLACK + side]) == 2)return true;
                // KQRK
                if (chessboard[QUEEN_BLACK + side] && chessboard[ROOK_BLACK + side])return true;
                // KRBK
                if (chessboard[QUEEN_BLACK + side] && chessboard[BISHOP_BLACK + side])return true;
                // KRNK
                if (chessboard[ROOK_BLACK + side] && chessboard[KNIGHT_BLACK + side])return true;
                // KRPK
                // KRRK
                if (bitCount(chessboard[ROOK_BLACK + side]) == 2)return true;

            default:
                return false;
        }
    }

    static bool isDraw(const int nPieces, const _Tchessboard &chessboard) {
        switch (nPieces) {
            case 2 :
                //KK
                return true;
            case 3:
                //KBK
                if (chessboard[BISHOP_BLACK] || chessboard[BISHOP_WHITE])return true;
                //KNK
                if (chessboard[KNIGHT_BLACK] || chessboard[KNIGHT_WHITE])return true;
                break;
            case 4 :
                //KBKB
                if (chessboard[BISHOP_BLACK] && chessboard[BISHOP_WHITE])return true;
                //KNKN
                if (chessboard[KNIGHT_BLACK] && chessboard[KNIGHT_WHITE])return true;
                //KBKN
                if (chessboard[BISHOP_BLACK] && chessboard[KNIGHT_WHITE])return true;
                if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_BLACK])return true;
                //KNNK
                if (bitCount(chessboard[KNIGHT_BLACK]) == 2)return true;
                if (bitCount(chessboard[KNIGHT_WHITE]) == 2)return true;
                break;
            default:
                return false;
        }
        return false;
    }
/*
    static int getEndgameValue(int side, const _Tchessboard &chessboard, const int nPieces) {
        assert(nPieces != 999);
        ASSERT_RANGE(side, 0, 1);
        auto posKingBlack = BITScanForward(chessboard[KING_BLACK]);
        auto posKingWhite = BITScanForward(chessboard[KING_WHITE]);
        switch (nPieces) {
            case 4 :
                if (chessboard[QUEEN_BLACK]) {
                    if (chessboard[PAWN_WHITE]) {
                        int result = KQKP(WHITE, posKingBlack, posKingWhite,
                                          BITScanForward(chessboard[PAWN_WHITE]));
                        return side == BLACK ? result : -result;
                    } else if (chessboard[ROOK_WHITE]) {
                        int result = KQKR(posKingBlack, posKingWhite);
                        return side == BLACK ? result : -result;
                    }
                } else if (chessboard[QUEEN_WHITE]) {
                    if (chessboard[PAWN_BLACK]) {
                        int result = KQKP(BLACK, posKingWhite, posKingBlack,
                                          BITScanForward(chessboard[PAWN_BLACK]));
                        return side == WHITE ? result : -result;
                    } else if (chessboard[ROOK_BLACK]) {
                        int result = KQKR(posKingWhite, posKingBlack);
                        return side == WHITE ? result : -result;
                    }
                } else if (chessboard[ROOK_BLACK]) {
                    if (chessboard[PAWN_WHITE]) {
                        int result = KRKP<WHITE>(side == BLACK, posKingBlack,
                                                 posKingWhite, BITScanForward(chessboard[ROOK_BLACK]),
                                                 BITScanForward(chessboard[PAWN_WHITE]));
                        return side == BLACK ? result : -result;
                    } else if (chessboard[BISHOP_WHITE]) {
                        int result = KRKB(posKingWhite);
                        return side == BLACK ? result : -result;
                    } else if (chessboard[KNIGHT_WHITE]) {
                        int result = KRKN(posKingWhite, BITScanForward(chessboard[KNIGHT_WHITE]));
                        return side == BLACK ? result : -result;
                    }
                } else if (chessboard[ROOK_WHITE]) {
                    if (chessboard[PAWN_BLACK]) {
                        int result = KRKP<BLACK>(side == WHITE, posKingWhite,
                                                 posKingBlack, BITScanForward(chessboard[ROOK_WHITE]),
                                                 BITScanForward(chessboard[PAWN_BLACK]));
                        return side == WHITE ? result : -result;
                    } else if (chessboard[BISHOP_BLACK]) {
                        int result = KRKB(posKingBlack);
                        return side == WHITE ? result : -result;
                    } else if (chessboard[KNIGHT_BLACK]) {
                        int result = KRKN(posKingWhite, BITScanForward(chessboard[KNIGHT_BLACK]));
                        return side == WHITE ? result : -result;
                    }
                }
//                else if ((chessboard[BISHOP_BLACK] && chessboard[KNIGHT_BLACK])) {
//                    int result = KBNK(posKingBlack, posKingWhite);
//                    return side == BLACK ? result : -result;
//                } else if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_WHITE]) {
//                    int result = KBNK(posKingWhite, posKingBlack);
//                    return side == WHITE ? result : -result;
//                }
                break;
            case 5:
                if (chessboard[KNIGHT_WHITE] && bitCount(chessboard[BISHOP_BLACK]) == 2) {
                    int result = KBBKN(posKingBlack, posKingWhite,
                                       BITScanForward(chessboard[KNIGHT_WHITE]));
                    return side == BLACK ? result : -result;
                } else if (chessboard[KNIGHT_BLACK] && bitCount(chessboard[BISHOP_WHITE]) == 2) {
                    int result = KBBKN(posKingWhite, posKingBlack,
                                       BITScanForward(chessboard[KNIGHT_BLACK]));
                    return side == WHITE ? result : -result;
                }
                break;
            default:
                break;
        }

        return INT_MAX;
    }

private:
    Endgame();

    constexpr static int DistanceBonus[8] = {0, 0, 100, 80, 60, 40, 20, 10};
    constexpr static int VALUE_KNOWN_WIN = 15000;
    constexpr static int penaltyKRKN[8] = {0, 10, 14, 20, 30, 42, 58, 80};
    constexpr static int KBNKMateTable[64] = {
            200, 190, 180, 170, 170, 180, 190, 200,
            190, 180, 170, 160, 160, 170, 180, 190,
            180, 170, 155, 140, 140, 155, 170, 180,
            170, 160, 140, 120, 120, 140, 160, 170,
            170, 160, 140, 120, 120, 140, 160, 170,
            180, 170, 155, 140, 140, 155, 170, 180,
            190, 180, 170, 160, 160, 170, 180, 190,
            200, 190, 180, 170, 170, 180, 190, 200};

    constexpr static int MateTable[64] = {
            100, 90, 80, 70, 70, 80, 90, 100, 90, 70, 60, 50, 50, 60, 70, 90, 80, 60, 40, 30, 30, 40, 60, 80, 70, 50,
            30, 20, 20, 30, 50, 70, 70, 50, 30, 20, 20, 30, 50, 70, 80, 60, 40, 30, 30, 40, 60, 80, 90, 70, 60, 50, 50,
            60, 70, 90, 100, 90, 80, 70, 70, 80, 90, 100,};


    template<int loserSide>
    static int KRKP(int tempo, int winnerKingPos, int loserKingPos, int rookPos, int pawnPos) {
#ifndef NDEBUG
        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[ROOK_BLACK] = 1;
        pieces1[PAWN_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[ROOK_WHITE] = 1;
        pieces2[PAWN_BLACK] = 1;


#endif

        // If the stronger side's king is in front of the pawn, it's a win
        if (FILE_AT[winnerKingPos] == FILE_AT[pawnPos]) {
            if (loserSide == BLACK && winnerKingPos < pawnPos) {
                return VALUEROOK - DISTANCE[winnerKingPos][pawnPos];
            }
            if (loserSide == WHITE && winnerKingPos > pawnPos) {
                return VALUEROOK - DISTANCE[winnerKingPos][pawnPos];
            }
        }
        // If the weaker side's king is too far from the pawn and the rook, it's a win
        if (DISTANCE[loserKingPos][pawnPos] - (tempo ^ 1) >= 3 && DISTANCE[loserKingPos][rookPos] >= 3) {
            return VALUEROOK - DISTANCE[winnerKingPos][pawnPos];
        }
        // If the pawn is far advanced and supported by the defending king, the position is drawish
        if (((loserSide == BLACK && RANK_AT[loserKingPos] <= 2) || (loserSide == WHITE && RANK_AT[loserKingPos] >= 5))
            && DISTANCE[loserKingPos][pawnPos] == 1 &&
            ((loserSide == BLACK && RANK_AT[winnerKingPos] >= 3) || (loserSide == WHITE && RANK_AT[winnerKingPos] <= 4))
            && DISTANCE[winnerKingPos][pawnPos] - tempo > 2) {
            return 80 - DISTANCE[winnerKingPos][pawnPos] * 8;
        } else {
            constexpr int DELTA_S = loserSide == WHITE ? -8 : 8;

            const int queeningSq = BITScanForward(FILE_[pawnPos] & RANK_1_8[loserSide ^ 1]);

            return 200 - 8 * (DISTANCE[winnerKingPos][pawnPos + DELTA_S]
                              - DISTANCE[loserKingPos][pawnPos + DELTA_S]
                              - DISTANCE[pawnPos][queeningSq]);
        }
    }

    static int KQKR(int winnerKingPos, int loserKingPos) {

#ifndef NDEBUG
        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[QUEEN_BLACK] = 1;
        pieces1[ROOK_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[QUEEN_WHITE] = 1;
        pieces2[ROOK_BLACK] = 1;


        ASSERT_RANGE(winnerKingPos, 0, 63);
        ASSERT_RANGE(loserKingPos, 0, 63);
#endif

        return VALUEQUEEN - VALUEROOK + MateTable[loserKingPos] +
               DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]];
    }

//    int KBNK(int winnerKingPos, int loserKingPos) {
//
//#ifndef NDEBUG
//        std::unordered_map<int, int> pieces1;
//        std::unordered_map<int, int> pieces2;
//
//        pieces1[KING_BLACK] = 1;
//        pieces1[KING_WHITE] = 1;
//        pieces1[BISHOP_BLACK] = 1;
//        pieces1[KNIGHT_BLACK] = 1;
//
//        pieces2[KING_BLACK] = 1;
//        pieces2[KING_WHITE] = 1;
//        pieces2[BISHOP_WHITE] = 1;
//        pieces2[KNIGHT_WHITE] = 1;
//
//
//        ASSERT_RANGE(winnerKingPos, 0, 63);
//        ASSERT_RANGE(loserKingPos, 0, 63);
//#endif
//
//        auto a = VALUE_KNOWN_WIN + DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]];
//        auto b = KBNKMateTable[loserKingPos];
//        return a + b;
//    }

    static int KRKB(int loserKingPos) {

#ifndef NDEBUG
        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[ROOK_BLACK] = 1;
        pieces1[BISHOP_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[ROOK_WHITE] = 1;
        pieces2[BISHOP_BLACK] = 1;


        ASSERT_RANGE(loserKingPos, 0, 63);
#endif

        return MateTable[loserKingPos];
    }

    static int KRKN(int loserKingPos, int knightPos) {

#ifndef NDEBUG
        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[ROOK_BLACK] = 1;
        pieces1[KNIGHT_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[ROOK_WHITE] = 1;
        pieces2[KNIGHT_BLACK] = 1;


        ASSERT_RANGE(loserKingPos, 0, 63);
#endif

        return MateTable[loserKingPos] + penaltyKRKN[DISTANCE[loserKingPos][knightPos]];
    }


    static int KQKP(int loserSide, int winnerKingPos, int loserKingPos, int pawnPos) {
#ifndef NDEBUG
        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[QUEEN_BLACK] = 1;
        pieces1[PAWN_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[QUEEN_WHITE] = 1;
        pieces2[PAWN_BLACK] = 1;


#endif

        static int result = DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]];
        if ((DISTANCE[loserKingPos][pawnPos] != 1) || (RANK_AT[pawnPos] != (loserSide == WHITE ? 6
                                                                                               : 1))
            ) {// 0x5a5a5a5a5a5a5a5aULL = FILE B D E F G
            result += VALUEQUEEN - VALUEPAWN;
        }
        return result;
    }

    static int KBBKN(int winnerKingPos, int loserKingPos, int knightPos) {

#ifndef NDEBUG

        std::unordered_map<int, int> pieces1;
        std::unordered_map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[BISHOP_BLACK] = 2;
        pieces1[KNIGHT_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[BISHOP_WHITE] = 2;
        pieces2[KNIGHT_BLACK] = 1;


        ASSERT_RANGE(winnerKingPos, 0, 63);
        ASSERT_RANGE(knightPos, 0, 63);
        ASSERT_RANGE(loserKingPos, 0, 63);
#endif

        return VALUEBISHOP + DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]] +
               (DISTANCE[loserKingPos][knightPos]) * 32;
        // Bonus for driving the defending king and knight apart
        // Bonus for restricting the knight's mobility
        //result += Value((8 - popcount<Max15>(pos.attacks_from<KNIGHT>(nsq))) * 8);
    }

*/
};

