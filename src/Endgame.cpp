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
    assert(0);
    ASSERT_RANGE(side, 0, 1);
    ASSERT_RANGE(N_PIECE, 2, 32);
    int result = INT_MAX;
    int winnerSide = WHITE;
    switch (N_PIECE) {
        case 4 :
            if (chessboard[QUEEN_BLACK] && chessboard[PAWN_WHITE]) {
                result = KQKP(WHITE, Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[PAWN_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[QUEEN_WHITE] && chessboard[PAWN_BLACK]) {
                result = KQKP(BLACK, Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[PAWN_BLACK]));
            } else if (chessboard[ROOK_BLACK] && chessboard[PAWN_WHITE]) {
                result = KRKP(WHITE, side == BLACK, Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[ROOK_BLACK]), Bits::BITScanForward(chessboard[PAWN_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[ROOK_WHITE] && chessboard[PAWN_BLACK]) {
                result = KRKP(BLACK, side == WHITE, Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[ROOK_WHITE]), Bits::BITScanForward(chessboard[PAWN_BLACK]));
            } else if ((chessboard[BISHOP_BLACK] && chessboard[KNIGHT_BLACK])) {
                result = KBNK(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_WHITE]) {
                result = KBNK(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]));
            } else if (chessboard[ROOK_BLACK] && chessboard[BISHOP_WHITE]) {
                result = KRKB(Bits::BITScanForward(chessboard[KING_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[ROOK_WHITE] && chessboard[BISHOP_BLACK]) {
                result = KRKB(Bits::BITScanForward(chessboard[KING_BLACK]));
            } else if (chessboard[ROOK_BLACK] && chessboard[KNIGHT_WHITE]) {
                result = KRKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[ROOK_WHITE] && chessboard[KNIGHT_BLACK]) {
                result = KRKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_BLACK]));
            } else if (chessboard[ROOK_BLACK] && chessboard[QUEEN_WHITE]) {
                result = KQKR(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]));
            } else if (chessboard[ROOK_WHITE] && chessboard[QUEEN_BLACK]) {
                result = KQKR(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]));
                winnerSide = BLACK;
            }
            break;
        case 5:
            if (chessboard[KNIGHT_WHITE] && Bits::bitCount(chessboard[BISHOP_BLACK]) == 2) {
                result = KBBKN(Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KNIGHT_WHITE]));
                winnerSide = BLACK;
            } else if (chessboard[KNIGHT_BLACK] && Bits::bitCount(chessboard[BISHOP_WHITE]) == 2) {
                result = KBBKN(Bits::BITScanForward(chessboard[KING_WHITE]), Bits::BITScanForward(chessboard[KING_BLACK]), Bits::BITScanForward(chessboard[KNIGHT_BLACK]));
            }
            break;
        default:
        assert(0);
    }

    return winnerSide == side ? result : -result;
}

int Endgame::KRKP(int loserSide, int tempo, int winnerKingPos, int loserKingPos, int rookPos, int pawnPos) {
#ifdef DEBUG_MODE
    std::map<int, int> pieces1;
    std::map<int, int> pieces2;

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
    //int tempo = (side== strongerSide);
    int queeningSq = FILE_AT[pawnPos] | 0;
    // If the stronger side's king is in front of the pawn, it's a win
    if (winnerKingPos < pawnPos && FILE_AT[winnerKingPos] == FILE_AT[pawnPos]) {
        return _eval::VALUEROOK - Bits::DISTANCE[winnerKingPos][pawnPos];
    }
        // If the weaker side's king is too far from the pawn and the rook,
        // it's a win
    else if (Bits::DISTANCE[loserKingPos][pawnPos] - (tempo ^ 1) >= 3 && Bits::DISTANCE[loserKingPos][rookPos] >= 3) {
        return _eval::VALUEROOK - Bits::DISTANCE[winnerKingPos][pawnPos];
    }
        // If the pawn is far advanced and supported by the defending king,
        // the position is drawish
    else if (((loserSide == WHITE && RANK_AT[loserKingPos] <= 2) || (loserSide == BLACK && RANK_AT[loserKingPos] >= 5)) && Bits::DISTANCE[loserKingPos][pawnPos] == 1 && ((loserSide == BLACK && RANK_AT[winnerKingPos] >= 3) || (loserSide == WHITE && RANK_AT[winnerKingPos] <= 4)) && Bits::DISTANCE[winnerKingPos][pawnPos] - tempo > 2) {
        return 80 - Bits::DISTANCE[winnerKingPos][pawnPos] * 8;
    } else {
        return 200 - (Bits::DISTANCE[winnerKingPos][pawnPos] - 8) * 8 + (Bits::DISTANCE[loserKingPos][pawnPos] - 8) * 8 + Bits::DISTANCE[pawnPos][queeningSq] * 8;
    }
}

int Endgame::KQKP(int side, int winnerKingPos, int loserKingPos, int pawnPos) {
#ifdef DEBUG_MODE
    std::map<int, int> pieces1;
    std::map<int, int> pieces2;

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

    int result = _eval::VALUEQUEEN - _eval::VALUEPAWN + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
    if (Bits::DISTANCE[loserKingPos][pawnPos] == 1 && RANK_AT[pawnPos] == (side == WHITE ? 6 : 3)) {
        int f = FILE_AT[pawnPos];
        if (f == 0 || f == 2 || f == 5 || f == 7) {
            result = DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
        }
    }
    return result;
}

int Endgame::KBBKN(int winnerKingPos, int loserKingPos, int knightPos) {

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

    return _eval::VALUEBISHOP + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]] + (Bits::DISTANCE[loserKingPos][knightPos]) * 32;
    // Bonus for driving the defending king and knight apart
    // Bonus for restricting the knight's mobility
    //result += Value((8 - popcount<Max15>(pos.attacks_from<KNIGHT>(nsq))) * 8);
}

int Endgame::KQKR(int winnerKingPos, int loserKingPos) {

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
    return _eval::VALUEQUEEN - _eval::VALUEROOK + MateTable[loserKingPos] + DistanceBonus[Bits::DISTANCE[winnerKingPos][loserKingPos]];
}

int Endgame::KBNK(int winnerKingPos, int loserKingPos) {

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

int Endgame::KRKB(int loserKingPos) {

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

int Endgame::KRKN(int loserKingPos, int knightPos) {

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

Endgame::~Endgame() {
}

