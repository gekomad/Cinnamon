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

int Endgame::getEndgameValue(const int N_PIECE, const int side) {
    ASSERT(N_PIECE != 999);
    ASSERT_RANGE(side, 0, 1);

    int result = INT_MAX;
    int winnerSide = -1;
    switch (N_PIECE) {
        case 4 :
            if (chessboard[QUEEN_BLACK]) {
                if (chessboard[PAWN_WHITE]) {
                    result = KQKP(WHITE, Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[PAWN_WHITE]));
                    winnerSide = BLACK;
                } else if (chessboard[ROOK_WHITE]) {
                    result = KQKR(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]));
                    winnerSide = BLACK;
                }
            } else if (chessboard[QUEEN_WHITE]) {
                if (chessboard[PAWN_BLACK]) {
                    result = KQKP(BLACK, Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[PAWN_BLACK]));
                    winnerSide = WHITE;
                } else if (chessboard[ROOK_BLACK]) {
                    result = KQKR(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]));
                    winnerSide = WHITE;
                }
            } else if (chessboard[ROOK_BLACK]) {
                if (chessboard[PAWN_WHITE]) {
                    result = KRKP<WHITE>(side == BLACK, Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[ROOK_BLACK]), Bits::BITScanForward(chessboard[PAWN_WHITE]));
                    winnerSide = BLACK;
                } else if (chessboard[BISHOP_WHITE]) {
                    result = KRKB(Bits::BITScanForward(chessboard[KING_WHITE]));
                    winnerSide = BLACK;
                } else if (chessboard[KNIGHT_WHITE]) {
                    result = KRKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_WHITE]));
                    winnerSide = BLACK;
                }
            } else if (chessboard[ROOK_WHITE]) {
                if (chessboard[PAWN_BLACK]) {
                    result = KRKP<BLACK>(side == WHITE, Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[ROOK_WHITE]), Bits::BITScanForward(chessboard[PAWN_BLACK]));
                    winnerSide = WHITE;
                } else if (chessboard[BISHOP_BLACK]) {
                    result = KRKB(Bits::BITScanForward(chessboard[KING_BLACK]));
                    winnerSide = WHITE;
                } else if (chessboard[KNIGHT_BLACK]) {
                    result = KRKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_BLACK]));
                    winnerSide = WHITE;
                }
            } else if ((chessboard[BISHOP_BLACK] && chessboard[KNIGHT_BLACK])) {
                result = KBNK(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_WHITE]) {
                result = KBNK(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]));
                winnerSide = WHITE;
            }
            break;
        case 5:
            if (chessboard[KNIGHT_WHITE] && Bits::bitCount(chessboard[BISHOP_BLACK]) == 2) {
                result = KBBKN(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[KNIGHT_BLACK] && Bits::bitCount(chessboard[BISHOP_WHITE]) == 2) {
                result = KBBKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KNIGHT_BLACK]));
                winnerSide = WHITE;
            }
            break;
        default:
            break;
    }
    if (winnerSide == -1)return INT_MAX;
    return winnerSide == side ? result : -result;
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

    int result = DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
    if ((Bits::DISTANCE[loserKingPos][pawnPos] != 1) || (RANK_AT[pawnPos] != (loserSide == WHITE ? 6 : 1)) /*|| (0x5a5a5a5a5a5a5a5aULL & POW2(pawnPos))*/) {// 0x5a5a5a5a5a5a5a5aULL = FILE B D E F G
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

    return _board::VALUEBISHOP + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]] + (Bits::DISTANCE[loserKingPos][knightPos]) * 32;
    // Bonus for driving the defending king and knight apart
    // Bonus for restricting the knight's mobility
    //result += Value((8 - popcount<Max15>(pos.attacks_from<KNIGHT>(nsq))) * 8);
}

int Endgame::KQKR(int winnerKingPos, int loserKingPos) {

#ifdef DEBUG_MODE
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

    ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

    ASSERT_RANGE(winnerKingPos, 0, 63);
    ASSERT_RANGE(loserKingPos, 0, 63);
    return _board::VALUEQUEEN - _board::VALUEROOK + MateTable[loserKingPos] + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
}

int Endgame::KBNK(int winnerKingPos, int loserKingPos) {

#ifdef DEBUG_MODE
    std::unordered_map<int, int> pieces1;
    std::unordered_map<int, int> pieces2;

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

int Endgame::KRKB(int loserKingPos) {

#ifdef DEBUG_MODE
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

    ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

    ASSERT_RANGE(loserKingPos, 0, 63);
    return MateTable[loserKingPos];
}

int Endgame::KRKN(int loserKingPos, int knightPos) {

#ifdef DEBUG_MODE
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

    ASSERT(checkNPieces(pieces1) || checkNPieces(pieces2));
#endif

    ASSERT_RANGE(loserKingPos, 0, 63);
    return MateTable[loserKingPos] + penaltyKRKN[Bits::DISTANCE[loserKingPos][knightPos]];
}

Endgame::~Endgame() {
}

