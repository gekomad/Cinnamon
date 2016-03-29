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

#include "ChessBoard.h"
#include <unordered_map>

class Endgame : public virtual ChessBoard {

public:

    Endgame();

    virtual ~Endgame();

    int getEndgameValue(const int N_PIECE, const int side);

private:

    const int DistanceBonus[8] = {0, 0, 100, 80, 60, 40, 20, 10};    //TODO stockfish
    const int VALUE_KNOWN_WIN = 15000;    //TODO stockfish
    const int penaltyKRKN[8] = {0, 10, 14, 20, 30, 42, 58, 80};    //TODO stockfish
    const int KBNKMateTable[64] = {    //TODO stockfish
            200, 190, 180, 170, 170, 180, 190, 200, 190, 180, 170, 160, 160, 170, 180, 190, 180, 170, 155, 140, 140, 155, 170, 180, 170, 160, 140, 120, 120, 140, 160, 170, 170, 160, 140, 120, 120, 140, 160, 170, 180, 170, 155, 140, 140, 155, 170, 180, 190, 180, 170, 160, 160, 170, 180, 190, 200, 190, 180, 170, 170, 180, 190, 200};

    const int MateTable[64] = {    //TODO stockfish
            100, 90, 80, 70, 70, 80, 90, 100, 90, 70, 60, 50, 50, 60, 70, 90, 80, 60, 40, 30, 30, 40, 60, 80, 70, 50, 30, 20, 20, 30, 50, 70, 70, 50, 30, 20, 20, 30, 50, 70, 80, 60, 40, 30, 30, 40, 60, 80, 90, 70, 60, 50, 50, 60, 70, 90, 100, 90, 80, 70, 70, 80, 90, 100,};


    template<int loserSide>
    int KRKP(int tempo, int winnerKingPos, int loserKingPos, int rookPos, int pawnPos) {
#ifdef DEBUG_MODE
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

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        // If the stronger side's king is in front of the pawn, it's a win
        if (FILE_AT[winnerKingPos] == FILE_AT[pawnPos]) {
            if (loserSide == BLACK && winnerKingPos < pawnPos) {
                return VALUEROOK - Bits::DISTANCE[winnerKingPos][pawnPos];
            }
            if (loserSide == WHITE && winnerKingPos > pawnPos) {
                return VALUEROOK - Bits::DISTANCE[winnerKingPos][pawnPos];
            }
        }
        // If the weaker side's king is too far from the pawn and the rook, it's a win
        if (Bits::DISTANCE[loserKingPos][pawnPos] - (tempo ^ 1) >= 3 && Bits::DISTANCE[loserKingPos][rookPos] >= 3) {
            return VALUEROOK - Bits::DISTANCE[winnerKingPos][pawnPos];
        }
        // If the pawn is far advanced and supported by the defending king, the position is drawish
        if (((loserSide == BLACK && RANK_AT[loserKingPos] <= 2) || (loserSide == WHITE && RANK_AT[loserKingPos] >= 5))
            && Bits::DISTANCE[loserKingPos][pawnPos] == 1 && ((loserSide == BLACK && RANK_AT[winnerKingPos] >= 3) || (loserSide == WHITE && RANK_AT[winnerKingPos] <= 4))
            && Bits::DISTANCE[winnerKingPos][pawnPos] - tempo > 2) {
            return 80 - Bits::DISTANCE[winnerKingPos][pawnPos] * 8;
        } else {
            constexpr int DELTA_S = loserSide == WHITE ? -8 : 8;
            int queeningSq = loserSide == BLACK ? Bits::BITScanForward(FILE_[pawnPos] & 0xffULL) : Bits::BITScanForward(FILE_[pawnPos] & 0xff00000000000000ULL);

            return 200 - 8 * (Bits::DISTANCE[winnerKingPos][pawnPos + DELTA_S]
                              - Bits::DISTANCE[loserKingPos][pawnPos + DELTA_S]
                              - Bits::DISTANCE[pawnPos][queeningSq]);
        }
    }

    int KQKP(int side, int winnerKingPos, int loserKingPos, int pawnPos);

    int KBBKN(int winnerKingPos, int loserKingPos, int knightPos);

    int KQKR(int winnerKingPos, int loserKingPos);

    int KBNK(int winnerKingPos, int loserKingPos);

    int KRKB(int loserKingPos);

    int KRKN(int loserKingPos, int knightPos);

};

