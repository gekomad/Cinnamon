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

#ifndef ENDGAME_H_
#define ENDGAME_H_

#include "ChessBoard.h"
#include <map>

using namespace _eval;

class Endgame : public virtual ChessBoard {

public:


    Endgame();

    virtual ~Endgame();


    int getEndgameValue(const int N_PIECE, const int side);

private:


    int KRKP(int loserSide, int tempo, int winnerKingPos, int loserKingPos, int rookPos, int pawnPos);

    int KQKP(int side, int winnerKingPos, int loserKingPos, int pawnPos);

    int KBBKN(int winnerKingPos, int loserKingPos, int knightPos) {

#ifdef DEBUG_MODE

        std::map<int, int> pieces1;
        std::map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[BISHOP_BLACK] = 2;
        pieces1[KNIGHT_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[BISHOP_WHITE] = 2;
        pieces2[KNIGHT_BLACK] = 1;

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        ASSERT_RANGE(winnerKingPos, 0, 63);
        ASSERT_RANGE(knightPos, 0, 63);
        ASSERT_RANGE(loserKingPos, 0, 63);

        return VALUEBISHOP + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]] + (Bits::DISTANCE[loserKingPos][knightPos]) * 32;
        // Bonus for driving the defending king and knight apart
        // Bonus for restricting the knight's mobility
        //result += Value((8 - popcount<Max15>(pos.attacks_from<KNIGHT>(nsq))) * 8);
    }

    int KQKR(int winnerKingPos, int loserKingPos) {

#ifdef DEBUG_MODE
        std::map<int, int> pieces1;
        std::map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[QUEEN_BLACK] = 1;
        pieces1[ROOK_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[QUEEN_WHITE] = 1;
        pieces2[ROOK_BLACK] = 1;

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        ASSERT_RANGE(winnerKingPos, 0, 63);
        ASSERT_RANGE(loserKingPos, 0, 63);
        return VALUEQUEEN - VALUEROOK + MateTable[loserKingPos] + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
    }

    int KBNK(int winnerKingPos, int loserKingPos) {

#ifdef DEBUG_MODE
        std::map<int, int> pieces1;
        std::map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[BISHOP_BLACK] = 1;
        pieces1[KNIGHT_BLACK] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[BISHOP_WHITE] = 1;
        pieces2[KNIGHT_WHITE] = 1;

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        ASSERT_RANGE(winnerKingPos, 0, 63);
        ASSERT_RANGE(loserKingPos, 0, 63);
        return VALUE_KNOWN_WIN + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]] + KBNKMateTable[loserKingPos];
    }

    int KRKB(int loserKingPos) {

#ifdef DEBUG_MODE
        std::map<int, int> pieces1;
        std::map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[ROOK_BLACK] = 1;
        pieces1[BISHOP_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[ROOK_WHITE] = 1;
        pieces2[BISHOP_BLACK] = 1;

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        ASSERT_RANGE(loserKingPos, 0, 63);
        return MateTable[loserKingPos];
    }

    int KRKN(int loserKingPos, int knightPos) {

#ifdef DEBUG_MODE
        std::map<int, int> pieces1;
        std::map<int, int> pieces2;

        pieces1[KING_BLACK] = 1;
        pieces1[KING_WHITE] = 1;
        pieces1[ROOK_BLACK] = 1;
        pieces1[KNIGHT_WHITE] = 1;

        pieces2[KING_BLACK] = 1;
        pieces2[KING_WHITE] = 1;
        pieces2[ROOK_WHITE] = 1;
        pieces2[KNIGHT_BLACK] = 1;

        ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

        ASSERT_RANGE(loserKingPos, 0, 63);
        return MateTable[loserKingPos] + penaltyKRKN[Bits::DISTANCE[loserKingPos][knightPos]];
    }

};

#endif
