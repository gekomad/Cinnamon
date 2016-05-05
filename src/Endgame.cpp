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

template<int side>
int Endgame::getEndgameValue(const _Tboard &structureEval, const int N_PIECE) {
    ASSERT(N_PIECE != 999);
    ASSERT_RANGE(side, 0, 1);

    switch (N_PIECE) {
        case 4 :
            if (chessboard[QUEEN_BLACK]) {
                if (chessboard[PAWN_WHITE]) {
                    int result = KQKP(WHITE, structureEval.posKing[BLACK], structureEval.posKing[WHITE], BITScanForward(chessboard[PAWN_WHITE]));
                    return side == BLACK ? result : -result;
                } else if (chessboard[ROOK_WHITE]) {
                    int result = KQKR(structureEval.posKing[BLACK], structureEval.posKing[WHITE]);
                    return side == BLACK ? result : -result;
                }
            } else if (chessboard[QUEEN_WHITE]) {
                if (chessboard[PAWN_BLACK]) {
                    int result = KQKP(BLACK, structureEval.posKing[WHITE], structureEval.posKing[BLACK], BITScanForward(chessboard[PAWN_BLACK]));
                    return side == WHITE ? result : -result;
                } else if (chessboard[ROOK_BLACK]) {
                    result = KQKR(structureEval.posKing[WHITE], structureEval.posKing[BLACK]);
                    return side == WHITE ? result : -result;
                }
            } else if (chessboard[ROOK_BLACK]) {
                if (chessboard[PAWN_WHITE]) {
                    int result = KRKP<WHITE>(side == BLACK, structureEval.posKing[BLACK], structureEval.posKing[WHITE], BITScanForward(chessboard[ROOK_BLACK]), BITScanForward(chessboard[PAWN_WHITE]));
                    return side == BLACK ? result : -result;
                } else if (chessboard[BISHOP_WHITE]) {
                    int result = KRKB(structureEval.posKing[WHITE]);
                    return side == BLACK ? result : -result;
                } else if (chessboard[KNIGHT_WHITE]) {
                    int result = KRKN(structureEval.posKing[WHITE], BITScanForward(chessboard[KNIGHT_WHITE]));
                    return side == BLACK ? result : -result;
                }
            } else if (chessboard[ROOK_WHITE]) {
                if (chessboard[PAWN_BLACK]) {
                    int result = KRKP<BLACK>(side == WHITE, structureEval.posKing[WHITE], structureEval.posKing[BLACK], BITScanForward(chessboard[ROOK_WHITE]), BITScanForward(chessboard[PAWN_BLACK]));
                    return side == WHITE ? result : -result;
                } else if (chessboard[BISHOP_BLACK]) {
                    int result = KRKB(structureEval.posKing[BLACK]);
                    return side == WHITE ? result : -result;
                } else if (chessboard[KNIGHT_BLACK]) {
                    int result = KRKN(structureEval.posKing[WHITE], BITScanForward(chessboard[KNIGHT_BLACK]));
                    return side == WHITE ? result : -result;
                }
            } else if ((chessboard[BISHOP_BLACK] && chessboard[KNIGHT_BLACK])) {
                int result = KBNK(structureEval.posKing[BLACK], structureEval.posKing[WHITE]);
                return side == BLACK ? result : -result;
            } else if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_WHITE]) {
                int result = KBNK(structureEval.posKing[WHITE], structureEval.posKing[BLACK]);
                return side == WHITE ? result : -result;
            }
            break;
        case 5:
            if (chessboard[KNIGHT_WHITE] && bitCount(chessboard[BISHOP_BLACK]) == 2) {
                int result = KBBKN(structureEval.posKing[BLACK], structureEval.posKing[WHITE], BITScanForward(chessboard[KNIGHT_WHITE]));
                return side == BLACK ? result : -result;
            } else if (chessboard[KNIGHT_BLACK] && bitCount(chessboard[BISHOP_WHITE]) == 2) {
                int result = KBBKN(structureEval.posKing[WHITE], structureEval.posKing[BLACK], BITScanForward(chessboard[KNIGHT_BLACK]));
                return side == WHITE ? result : -result;
            }
            break;
        default:
            break;
    }
    return INT_MAX;
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

