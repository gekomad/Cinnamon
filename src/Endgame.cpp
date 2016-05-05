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

#include "Endgame.h"

/*
KXK
KBNK    ok
KPK
KRKP    ok
KRKB    ok
KRKN    ok
KQKP    ok
KQKR    ok
KBBKN   ok
KBPsK
KQKRPs
KRPKR
KRPPKRP
KPsK
KBPKB
KBPPKB
KBPKN
KNPK
KNPKB
KPKP
*/

Endgame::Endgame() {
    srand(time(NULL));
}


int Endgame::KQKP(int loserSide, int winnerKingPos, int loserKingPos, int pawnPos) {
#ifdef DEBUG_MODE
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

    ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

    int result = DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]];
    if ((DISTANCE[loserKingPos][pawnPos] != 1) || (RANK_AT[pawnPos] != (loserSide == WHITE ? 6 : 1)) /*|| (0x5a5a5a5a5a5a5a5aULL & POW2(pawnPos))*/) {// 0x5a5a5a5a5a5a5a5aULL = FILE B D E F G
        result += _board::VALUEQUEEN - _board::VALUEPAWN;
    }
    return result;
}

int Endgame::KBBKN(int winnerKingPos, int loserKingPos, int knightPos) {

#ifdef DEBUG_MODE

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

    ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

    ASSERT_RANGE(winnerKingPos, 0, 63);
    ASSERT_RANGE(knightPos, 0, 63);
    ASSERT_RANGE(loserKingPos, 0, 63);

    return _board::VALUEBISHOP + DistanceBonus[DISTANCE[winnerKingPos][loserKingPos]] + (DISTANCE[loserKingPos][knightPos]) * 32;
    // Bonus for driving the defending king and knight apart
    // Bonus for restricting the knight's mobility
    //result += Value((8 - popcount<Max15>(pos.attacks_from<KNIGHT>(nsq))) * 8);
}

Endgame::~Endgame() {
}

