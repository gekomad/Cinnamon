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

#include "Eval.h"

using namespace _eval;

Eval::Eval() {
}

Eval::~Eval() {
}

template<int side>
void Eval::openFile() {
    u64 side_rooks = chessboard.bit[ROOK_BLACK + side];
    structureEval.openFile = 0;
    structureEval.semiOpenFile[side] = 0;
    while (side_rooks) {
        int o = BITScanForward(side_rooks);
        if (!(FILE_[o] & (chessboard.bit[WHITE] | chessboard.bit[BLACK]))) {
            structureEval.openFile |= FILE_[o];
        } else if (FILE_[o] & chessboard.bit[side ^ 1]) {
            structureEval.semiOpenFile[side] |= FILE_[o];
        }
        RESET_LSB(side_rooks);
    }
}

template<int side, Eval::_Tphase phase>
int Eval::evaluatePawn() {
    INC(evaluationCount[side]);
    u64 ped_friends = chessboard.bit[side];
    if (!ped_friends) {
        ADD(SCORE_DEBUG.NO_PAWNS[side], -NO_PAWNS);
        return -NO_PAWNS;
    }
    structureEval.isolated[side] = 0;
    int result = MOB_PAWNS[getMobilityPawns(side, chessboard.bit[ENPASSANT_IDX], ped_friends, side == WHITE ? structureEval.allPiecesSide[BLACK] : structureEval.allPiecesSide[WHITE], ~structureEval.allPiecesSide[BLACK] | ~structureEval.allPiecesSide[WHITE])];
    ADD(SCORE_DEBUG.MOB_PAWNS[side], result);
    if (bitCount(chessboard.bit[side ^ 1]) == 8) {
        result -= ENEMIES_PAWNS_ALL;
        ADD(SCORE_DEBUG.ENEMIES_PAWNS_ALL[side], -ENEMIES_PAWNS_ALL);
    }
    result += ATTACK_KING * bitCount(ped_friends & structureEval.kingAttackers[side ^ 1]);
    ADD(SCORE_DEBUG.ATTACK_KING_PAWN[side], ATTACK_KING * bitCount(ped_friends & structureEval.kingAttackers[side ^ 1]));
    //space
    if (phase == OPEN) {
        result += PAWN_CENTER * bitCount(ped_friends & CENTER_MASK);
        ADD(SCORE_DEBUG.PAWN_CENTER[side], PAWN_CENTER * bitCount(ped_friends & CENTER_MASK));
    }
    u64 p = ped_friends;
    while (p) {
        int o = BITScanForward(p);
        u64 pos = POW2[o];
        if (phase != OPEN) {
            structureEval.kingSecurityDistance[side] += FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & pos ? 1 : 0);
            structureEval.kingSecurityDistance[side] -= ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & pos ? 1 : 0);
            ///  pawn in race
            if (PAWNS_7_2[side] & pos) {
                result += PAWN_7H;
                ADD(SCORE_DEBUG.PAWN_7H[side], PAWN_7H);
                if (((shiftForward<side, 8>(pos) & (~structureEval.allPieces)) || (structureEval.allPiecesSide[side ^ 1] & PAWN_FORK_MASK[side][o]))) {
                    result += PAWN_IN_RACE;
                    ADD(SCORE_DEBUG.PAWN_IN_RACE[side], PAWN_IN_RACE);
                }
            }
        }
        /// blocked
        result -= (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[side ^ 1])) && (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED : 0;
        ADD(SCORE_DEBUG.PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[side ^ 1])) && (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED : 0);
        /// unprotected
        if (!(ped_friends & PAWN_PROTECTED_MASK[side][o])) {
            result -= UNPROTECTED_PAWNS;
            ADD(SCORE_DEBUG.UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
        };
        /// isolated
        if (!(ped_friends & PAWN_ISOLATED_MASK[o])) {
            result -= PAWN_ISOLATED;
            ADD(SCORE_DEBUG.PAWN_ISOLATED[side], -PAWN_ISOLATED);
            structureEval.isolated[side] |= pos;
        }
        /// doubled
        if (NOTPOW2[o] & FILE_[o] & ped_friends) {
            result -= DOUBLED_PAWNS;
            ADD(SCORE_DEBUG.DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            /// doubled and isolated
            if (!(structureEval.isolated[side] & pos)) {
                ADD(SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                result -= DOUBLED_ISOLATED_PAWNS;
            }
        };
        /// backward
        if (!(ped_friends & PAWN_BACKWARD_MASK[side][o])) {
            ADD(SCORE_DEBUG.BACKWARD_PAWN[side], -BACKWARD_PAWN);
            result -= BACKWARD_PAWN;
        }
        /// passed
        if (!(chessboard.bit[side ^ 1] & PAWN_PASSED_MASK[side][o])) {
            ADD(SCORE_DEBUG.PAWN_PASSED[side], PAWN_PASSED[side][o]);
            result += PAWN_PASSED[side][o];
        }
        RESET_LSB(p);
    }
    return result;
}

template<int side, Eval::_Tphase phase>
int Eval::evaluateBishop(u64 enemies, u64 friends) {
    INC(evaluationCount[side]);
    u64 x = chessboard.bit[BISHOP_BLACK + side];
    if (!x) {
        return 0;
    }
    int result = 0;
    if (phase != OPEN && bitCount(x) > 1) {
        result += BONUS2BISHOP;
        ADD(SCORE_DEBUG.BONUS2BISHOP[side], BONUS2BISHOP);
    }
    while (x) {
        int o = BITScanForward(x);
        u64 captured = getDiagCapture(o, enemies | friends, enemies);
        ASSERT(bitCount(captured) + getDiagShiftCount(o, enemies | friends) < (int) (sizeof(MOB_BISHOP) / sizeof(int)));
        result += MOB_BISHOP[phase][bitCount(captured) + getDiagShiftCount(o, enemies | friends)];
        ADD(SCORE_DEBUG.MOB_BISHOP[side], MOB_BISHOP[phase][bitCount(captured) + getDiagShiftCount(o, enemies | friends)]);
        structureEval.kingSecurityDistance[side] += BISHOP_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0);
        ADD(SCORE_DEBUG.KING_SECURITY_BISHOP[side], BISHOP_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0));
        if (phase != OPEN) {
            structureEval.kingSecurityDistance[side] -= NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? ENEMY_NEAR_KING : 0;
            ADD(SCORE_DEBUG.KING_SECURITY_BISHOP[side ^ 1], -NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? ENEMY_NEAR_KING : 0);
        } else
            //attack center
        if (phase == OPEN) {
            if (side) {
                if (o == C1 || o == F1) {
                    ADD(SCORE_DEBUG.UNDEVELOPED_BISHOP[side], -UNDEVELOPED_BISHOP);
                    result -= UNDEVELOPED_BISHOP;
                }
            } else {
                if (o == C8 || o == F8) {
                    ADD(SCORE_DEBUG.UNDEVELOPED_BISHOP[side], -UNDEVELOPED_BISHOP);
                    result -= UNDEVELOPED_BISHOP;
                }
            }
        } else {
            if (BIG_DIAGONAL & POW2[o] && !(DIAGONAL[o] & structureEval.allPieces)) {
                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
                result += OPEN_FILE;
            }
            if (BIG_ANTIDIAGONAL & POW2[o] && !(ANTIDIAGONAL[o] & structureEval.allPieces)) {
                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
                result += OPEN_FILE;
            }
        }
        RESET_LSB(x);
    };
    return result;
}

template<int side, Eval::_Tphase phase>
int Eval::evaluateQueen(u64 enemies, u64 friends) {
    INC(evaluationCount[side]);
    int result = 0;
    u64 queen = chessboard.bit[QUEEN_BLACK + side];
    while (queen) {
        int o = BITScanForward(queen);
        ASSERT(getMobilityQueen(o, enemies, friends) < (int) (sizeof(MOB_QUEEN[phase]) / sizeof(int)));
        ASSERT(structureEval.allPieces == enemies | friends);
        result += MOB_QUEEN[phase][getMobilityQueen(o, enemies, structureEval.allPieces)];
        ADD(SCORE_DEBUG.MOB_QUEEN[side], MOB_QUEEN[phase][getMobilityQueen(o, enemies, structureEval.allPieces)]);
        if (phase != OPEN) {
            structureEval.kingSecurityDistance[side] += FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_QUEEN[side], FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0));
            structureEval.kingSecurityDistance[side] -= ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_QUEEN[side ^ 1], -ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? 1 : 0));
        }
        if ((chessboard.bit[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG.HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            result += HALF_OPEN_FILE_Q;
        }
        if ((FILE_[o] & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG.OPEN_FILE_Q[side], OPEN_FILE_Q);
            result += OPEN_FILE_Q;
        }
        if (DIAGONAL_ANTIDIAGONAL[o] & chessboard.bit[BISHOP_BLACK + side]) {
            ADD(SCORE_DEBUG.BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            result += BISHOP_ON_QUEEN;
        }
        RESET_LSB(queen);
    };
    return result;
}

template<int side, Eval::_Tphase phase>
int Eval::evaluateKnight(const u64 enemiesPawns, const u64 squares) {
    INC(evaluationCount[side]);
    int result = 0;
    u64 x = chessboard.bit[KNIGHT_BLACK + side];
    if (phase == OPEN) {
        result -= side ? bitCount(x & 0x42ULL) * UNDEVELOPED : bitCount(x & 0x4200000000000000ULL) * UNDEVELOPED;
        ADD(SCORE_DEBUG.UNDEVELOPED_KNIGHT[side], side ? -bitCount(x & 0x42ULL) * UNDEVELOPED : -bitCount(x & 0x4200000000000000ULL) * UNDEVELOPED);
    }
    if (side == WHITE) {
        if ((A7bit & x) && (B7bit & enemiesPawns) && (C6A6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((H7bit & x) && (G7bit & enemiesPawns) && (F6H6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((A8bit & x) && (A7C7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((H8bit & x) && (H7G7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
    } else {
        if ((A2bit & x) && (B2bit & enemiesPawns) && (C3A3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((H2bit & x) && (G2bit & enemiesPawns) && (F3H3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((A1bit & x) && (A2C2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
        if ((H1bit & x) && (H2G2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG.KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result -= KNIGHT_TRAPPED;
        }
    }
    while (x) {
        int pos = BITScanForward(x);
        if (phase != OPEN) {
            structureEval.kingSecurityDistance[side] += FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[pos] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_KNIGHT[side], FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[pos] ? 1 : 0));
            structureEval.kingSecurityDistance[side] -= ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[pos] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_KNIGHT[side ^ 1], -ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[pos] ? 1 : 0));
        }
        //mobility
        ASSERT(bitCount(squares & KNIGHT_MASK[pos]) < (int) (sizeof(MOB_KNIGHT) / sizeof(int)));
        result += MOB_KNIGHT[bitCount(squares & KNIGHT_MASK[pos])];
        ADD(SCORE_DEBUG.MOB_KNIGHT[side], MOB_KNIGHT[bitCount(squares & KNIGHT_MASK[pos])]);
        RESET_LSB(x);
    };
    return result;
}

template<int side, Eval::_Tphase phase>
int Eval::evaluateRook(const u64 king, u64 enemies, u64 friends) {
    INC(evaluationCount[side]);
    int o, result = 0;
    u64 x = chessboard.bit[ROOK_BLACK + side];
    if (!x) {
        return 0;
    }
    if (phase == MIDDLE) {
        if (!side && (o = bitCount(x & RANK_1))) {
            ADD(SCORE_DEBUG.ROOK_7TH_RANK[side], ROOK_7TH_RANK * o);
            result += ROOK_7TH_RANK * o;
        }
        if (side && (o = bitCount(x & RANK_6))) {
            ADD(SCORE_DEBUG.ROOK_7TH_RANK[side], ROOK_7TH_RANK * o);
            result += ROOK_7TH_RANK * o;
        }
    }
    if (side == WHITE) {
        if (((F1G1bit & king) && (H1H2G1bit & x)) || ((C1B1bit & king) && (A1A2B1bit & x))) {
            ADD(SCORE_DEBUG.ROOK_TRAPPED[side], -ROOK_TRAPPED);
            result -= ROOK_TRAPPED;
        }
    } else {
        if (((F8G8bit & king) && (H8H7G8bit & x)) || ((C8B8bit & king) && (A8A7B8bit & x))) {
            ADD(SCORE_DEBUG.ROOK_TRAPPED[side], -ROOK_TRAPPED);
            result -= ROOK_TRAPPED;
        }
    }
    int firstRook = -1;
    int secondRook = -1;
    while (x) {
        o = BITScanForward(x);
        //mobility
        ASSERT(getMobilityRook(o, enemies, friends) < (int) (sizeof(MOB_ROOK[phase]) / sizeof(int)));
        result += MOB_ROOK[phase][getMobilityRook(o, enemies, friends)];
        ADD(SCORE_DEBUG.MOB_ROOK[side], MOB_ROOK[phase][getMobilityRook(o, enemies, friends)]);
        if (firstRook == -1) {
            firstRook = o;
        } else {
            secondRook = o;
        }
        if (phase != OPEN) {
            structureEval.kingSecurityDistance[side] += FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_ROOK[side], FRIEND_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side]] & POW2[o] ? 1 : 0));
            structureEval.kingSecurityDistance[side] -= ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? 1 : 0);
            ADD(SCORE_DEBUG.KING_SECURITY_ROOK[side ^ 1], -ENEMY_NEAR_KING * (NEAR_MASK2[structureEval.posKing[side ^ 1]] & POW2[o] ? 1 : 0));
            // Penalise if Rook is Blocked Horizontally
            if ((RANK_BOUND[o] & structureEval.allPieces) == RANK_BOUND[o]) {
                ADD(SCORE_DEBUG.ROOK_BLOCKED[side], -ROOK_BLOCKED);
                result -= ROOK_BLOCKED;
            };
        }
        if (!(chessboard.bit[side] & FILE_[o])) {
            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
            result += OPEN_FILE;
        }
        if (!(chessboard.bit[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
            result += OPEN_FILE;
        }
        RESET_LSB(x);
    };
    if (firstRook != -1 && secondRook != -1) {
        if ((!(LINK_ROOKS[firstRook][secondRook] & structureEval.allPieces))) {
            ADD(SCORE_DEBUG.CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            result += CONNECTED_ROOKS;
        }
    }
    return result;
}

template<Eval::_Tphase phase>
int Eval::evaluateKing(int side, u64 squares) {
    ASSERT(evaluationCount[side] == 5);
    int result = 0;
    uchar pos_king = structureEval.posKing[side];
    if (phase == END) {
        ADD(SCORE_DEBUG.DISTANCE_KING[side], DISTANCE_KING_ENDING[pos_king]);
        result = DISTANCE_KING_ENDING[pos_king];
    } else {
        ADD(SCORE_DEBUG.DISTANCE_KING[side], DISTANCE_KING_OPENING[pos_king]);
        result = DISTANCE_KING_OPENING[pos_king];
    }
    u64 POW2_king = POW2[pos_king];
    //mobility
    ASSERT(bitCount(squares & NEAR_MASK1[pos_king]) < (int) (sizeof(MOB_KING[phase]) / sizeof(int)));
    result += MOB_KING[phase][bitCount(squares & NEAR_MASK1[pos_king])];
    ADD(SCORE_DEBUG.MOB_KING[side], MOB_KING[phase][bitCount(squares & NEAR_MASK1[pos_king])]);
    if (phase != OPEN) {
        if ((structureEval.openFile & POW2_king) || (structureEval.semiOpenFile[side ^ 1] & POW2_king)) {
            ADD(SCORE_DEBUG.END_OPENING_KING[side], -END_OPENING);
            result -= END_OPENING;
            if (bitCount(RANK[pos_king]) < 4) {
                ADD(SCORE_DEBUG.END_OPENING_KING[side], -END_OPENING);
                result -= END_OPENING;
            }
        }
    }
    ASSERT(pos_king < 64);
    if (!(NEAR_MASK1[pos_king] & chessboard.bit[side])) {
        ADD(SCORE_DEBUG.PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        result -= PAWN_NEAR_KING;
    }
    result += structureEval.kingSecurityDistance[side];
    return result;
}

int Eval::getScore(const int side, const int nPieces, const int alpha, const int beta, const bool trace) {

    int lazyscore_white = lazyEvalSide<WHITE>();
    int lazyscore_black = lazyEvalSide<BLACK>();
    int lazyscore = lazyscore_black - lazyscore_white;
    if (side) {
        lazyscore = -lazyscore;
    }
    if (lazyscore > (beta + FUTIL_MARGIN) || lazyscore < (alpha - FUTIL_MARGIN)) {
        INC(lazyEvalCuts);
        return lazyscore;
    }
    memset(structureEval.kingSecurityDistance, 0, sizeof(structureEval.kingSecurityDistance));
    structureEval.posKing[BLACK] = (uchar) BITScanForward(chessboard.bit[KING_BLACK]);
    structureEval.posKing[WHITE] = (uchar) BITScanForward(chessboard.bit[KING_WHITE]);
//    int endGameValue;
//    if (side == WHITE)endGameValue = getEndgameValue<WHITE>(structureEval, nPieces);
//    else endGameValue = getEndgameValue<BLACK>(structureEval, nPieces);
//    if (endGameValue != INT_MAX) {
//        return endGameValue;
//    }

#ifdef DEBUG_MODE
    evaluationCount[WHITE] = evaluationCount[BLACK] = 0;
    memset(&SCORE_DEBUG, 0, sizeof(_TSCORE_DEBUG));
#endif

    structureEval.allPiecesNoPawns[BLACK] = getBitmapNoPawns<BLACK>();
    structureEval.allPiecesNoPawns[WHITE] = getBitmapNoPawns<WHITE>();
    int npieces = bitCount(structureEval.allPiecesNoPawns[BLACK] | structureEval.allPiecesNoPawns[WHITE]);

    _Tphase phase;
    if (npieces < 6) {
        phase = END;
    } else if (npieces < 13) {
        phase = MIDDLE;
    } else {
        phase = OPEN;
    }

    structureEval.allPiecesSide[BLACK] = structureEval.allPiecesNoPawns[BLACK] | chessboard.bit[PAWN_BLACK];
    structureEval.allPiecesSide[WHITE] = structureEval.allPiecesNoPawns[WHITE] | chessboard.bit[PAWN_WHITE];
    structureEval.allPieces = structureEval.allPiecesSide[BLACK] | structureEval.allPiecesSide[WHITE];

    structureEval.kingAttackers[WHITE] = getAllAttackers<WHITE>(structureEval.posKing[WHITE], structureEval.allPieces);
    structureEval.kingAttackers[BLACK] = getAllAttackers<BLACK>(structureEval.posKing[BLACK], structureEval.allPieces);

    openFile<WHITE>();
    openFile<BLACK>();
    int bonus_attack_king_black = 0;
    int bonus_attack_king_white = 0;
    if (phase != OPEN) {
        bonus_attack_king_black = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[WHITE])];
        bonus_attack_king_white = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[BLACK])];
    }
    _Tresult Tresult;
    switch (phase) {
        case OPEN :
            getRes<OPEN>(Tresult);
            break;
        case END :
            getRes<END>(Tresult);
            break;
        case MIDDLE:
            getRes<MIDDLE>(Tresult);
            break;
        default:
            break;
    }

    ASSERT(getMobilityCastle(WHITE, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[phase]) / sizeof(int)));
    ASSERT(getMobilityCastle(BLACK, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[phase]) / sizeof(int)));
    int mobWhite = MOB_CASTLE[phase][getMobilityCastle(WHITE, structureEval.allPieces)];
    int mobBlack = MOB_CASTLE[phase][getMobilityCastle(BLACK, structureEval.allPieces)];
    int attack_king_white = ATTACK_KING * bitCount(structureEval.kingAttackers[BLACK]);
    int attack_king_black = ATTACK_KING * bitCount(structureEval.kingAttackers[WHITE]);
    side == WHITE ? lazyscore_black -= 5 : lazyscore_white += 5;
    int result = (mobBlack + attack_king_black + bonus_attack_king_black + lazyscore_black + Tresult.pawns[BLACK] + Tresult.knights[BLACK] + Tresult.bishop[BLACK] + Tresult.rooks[BLACK] + Tresult.queens[BLACK] + Tresult.kings[BLACK]) - (mobWhite + attack_king_white + bonus_attack_king_white + lazyscore_white + Tresult.pawns[WHITE] + Tresult.knights[WHITE] + Tresult.bishop[WHITE] + Tresult.rooks[WHITE] + Tresult.queens[WHITE] + Tresult.kings[WHITE]);

#ifdef DEBUG_MODE
    if (trace) {
        const string HEADER = "\n\t\t\t\t\tTOT (white)\t\t  WHITE\t\tBLACK\n";
        if (side == WHITE) cout << "\nTotal (white)..........   " << (double) -result / 100.0 << "\n";
        else
            cout << "\nTotal (black)..........   " << (double) result / 100.0 << "\n";
        cout << "PHASE: ";
        if (phase == OPEN) {
            cout << " OPEN\n";
        } else if (phase == MIDDLE) {
            cout << " MIDDLE\n";
        } else {
            cout << " END\n";
        }
        cout << "OPEN FILE: ";
        if (!structureEval.openFile)cout << "none"; else
            for (int i = 0; i < 8; i++) if (POW2[i] & structureEval.openFile)cout << (char) (65 + i) << " ";
        cout << "\n";


        cout << "VALUES:";
        cout << "\tPAWN: " << (double) _board::VALUEPAWN / 100.0;
        cout << " ROOK: " << (double) _board::VALUEROOK / 100.0;
        cout << " BISHOP: " << (double) _board::VALUEBISHOP / 100.0;
        cout << " KNIGHT: " << (double) _board::VALUEKNIGHT / 100.0;
        cout << " QUEEN: " << (double) _board::VALUEQUEEN / 100.0 << "\n\n";

        cout << HEADER;
        cout << "Material:         " << setw(10) << (double) (lazyscore_white - lazyscore_black) / 100.0 << setw(15) << (double) (lazyscore_white) / 100.0 << setw(10) << (double) (lazyscore_black) / 100.0 << "\n";
//        cout << "Semi-open file:   " << setw(10) << (double) (SCORE_DEBUG.HALF_OPEN_FILE[WHITE] - SCORE_DEBUG.HALF_OPEN_FILE[BLACK]) / 100.0 << setw(15) << (double) (SCORE_DEBUG.HALF_OPEN_FILE[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.HALF_OPEN_FILE[BLACK]) / 100.0 << "\n";
        cout << "Mobility:         " << setw(10) << (double) (mobWhite - mobBlack) / 100.0 << setw(15) << (double) (mobWhite) / 100.0 << setw(10) << (double) (mobBlack) / 100.0 << "\n";
//        cout << "Attack king:      " << setw(10) << (double) (attack_king_white - attack_king_black) / 100.0 << setw(15) << (double) (attack_king_white) / 100.0 << setw(10) << (double) (attack_king_black) / 100.0 << "\n";
        cout << "Bonus attack king:" << setw(10) << (double) (bonus_attack_king_white - bonus_attack_king_black) / 100.0 << setw(15) << (double) (bonus_attack_king_white) / 100.0 << setw(10) << (double) (bonus_attack_king_black) / 100.0 << "\n";

        cout << HEADER;
        cout << "Pawn:             " << setw(10) << (double) (Tresult.pawns[WHITE] - Tresult.pawns[BLACK]) / 100.0 << setw(15) << (double) (Tresult.pawns[WHITE]) / 100.0 << setw(10) << (double) (Tresult.pawns[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_PAWNS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "       attack king:              " << setw(10) << (double) (SCORE_DEBUG.ATTACK_KING_PAWN[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ATTACK_KING_PAWN[BLACK]) / 100.0 << "\n";
        cout << "       center:                   " << setw(10) << (double) (SCORE_DEBUG.PAWN_CENTER[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_CENTER[BLACK]) / 100.0 << "\n";
        cout << "       7h:                       " << setw(10) << (double) (SCORE_DEBUG.PAWN_7H[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_7H[BLACK]) / 100.0 << "\n";
        cout << "       in race:                  " << setw(10) << (double) (SCORE_DEBUG.PAWN_IN_RACE[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_IN_RACE[BLACK]) / 100.0 << "\n";
        cout << "       blocked:                  " << setw(10) << (double) (SCORE_DEBUG.PAWN_BLOCKED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_BLOCKED[BLACK]) / 100.0 << "\n";
        cout << "       unprotected:              " << setw(10) << (double) (SCORE_DEBUG.UNPROTECTED_PAWNS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.UNPROTECTED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "       isolated                  " << setw(10) << (double) (SCORE_DEBUG.PAWN_ISOLATED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_ISOLATED[BLACK]) / 100.0 << "\n";
        cout << "       double                    " << setw(10) << (double) (SCORE_DEBUG.DOUBLED_PAWNS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.DOUBLED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "       double isolated           " << setw(10) << (double) (SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "       backward                  " << setw(10) << (double) (SCORE_DEBUG.BACKWARD_PAWN[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.BACKWARD_PAWN[BLACK]) / 100.0 << "\n";
        cout << "       fork:                     " << setw(10) << (double) (SCORE_DEBUG.FORK_SCORE[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.FORK_SCORE[BLACK]) / 100.0 << "\n";
        cout << "       passed:                   " << setw(10) << (double) (SCORE_DEBUG.PAWN_PASSED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_PASSED[BLACK]) / 100.0 << "\n";
        cout << "       all enemies:              " << setw(10) << (double) (SCORE_DEBUG.ENEMIES_PAWNS_ALL[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ENEMIES_PAWNS_ALL[BLACK]) / 100.0 << "\n";
        cout << "       none:                     " << setw(10) << (double) (SCORE_DEBUG.NO_PAWNS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.NO_PAWNS[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "Knight:           " << setw(10) << (double) (Tresult.knights[WHITE] - Tresult.knights[BLACK]) / 100.0 << setw(15) << (double) (Tresult.knights[WHITE]) / 100.0 << setw(10) << (double) (Tresult.knights[BLACK]) / 100.0 << "\n";
        cout << "       undevelop:                " << setw(10) << (double) (SCORE_DEBUG.UNDEVELOPED_KNIGHT[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.UNDEVELOPED_KNIGHT[BLACK]) / 100.0 << "\n";
        cout << "       trapped:                  " << setw(10) << (double) (SCORE_DEBUG.KNIGHT_TRAPPED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.KNIGHT_TRAPPED[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_KNIGHT[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_KNIGHT[BLACK]) / 100.0 << "\n";
//        cout << "       near enemy king           " << setw(10) << (double) (SCORE_DEBUG.XKNIGHT_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.XKNIGHT_NEAR_KING[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "Bishop:           " << setw(10) << (double) (Tresult.bishop[WHITE] - Tresult.bishop[BLACK]) / 100.0 << setw(15) << (double) (Tresult.bishop[WHITE]) / 100.0 << setw(10) << (double) (Tresult.bishop[BLACK]) / 100.0 << "\n";
        cout << "       bad:                      " << setw(10) << (double) (SCORE_DEBUG.BAD_BISHOP[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.BAD_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_BISHOP[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "       undevelop:                " << setw(10) << (double) (SCORE_DEBUG.UNDEVELOPED_BISHOP[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.UNDEVELOPED_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "       open diag:                " << setw(10) << (double) (SCORE_DEBUG.OPEN_DIAG_BISHOP[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.OPEN_DIAG_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "       bonus 2 bishops:          " << setw(10) << (double) (SCORE_DEBUG.BONUS2BISHOP[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.BONUS2BISHOP[BLACK]) / 100.0 << "\n";
//        cout << "       near enemy king           " << setw(10) << (double) (SCORE_DEBUG.XBISHOP_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.XBISHOP_NEAR_KING[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "Rook:             " << setw(10) << (double) (Tresult.rooks[WHITE] - Tresult.rooks[BLACK]) / 100.0 << setw(15) << (double) (Tresult.rooks[WHITE]) / 100.0 << setw(10) << (double) (Tresult.rooks[BLACK]) / 100.0 << "\n";
        cout << "       7th:                      " << setw(10) << (double) (SCORE_DEBUG.ROOK_7TH_RANK[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_7TH_RANK[BLACK]) / 100.0 << "\n";
        cout << "       trapped:                  " << setw(10) << (double) (SCORE_DEBUG.ROOK_TRAPPED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_TRAPPED[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_ROOK[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_ROOK[BLACK]) / 100.0 << "\n";
        cout << "       blocked:                  " << setw(10) << (double) (SCORE_DEBUG.ROOK_BLOCKED[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_BLOCKED[BLACK]) / 100.0 << "\n";
        cout << "       open file:                " << setw(10) << (double) (SCORE_DEBUG.ROOK_OPEN_FILE[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_OPEN_FILE[BLACK]) / 100.0 << "\n";
//        cout << "       semi open file:           " << setw(10) << (double) (SCORE_DEBUG.ROOK_SEMI_OPEN_FILE[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_SEMI_OPEN_FILE[BLACK]) / 100.0 << "\n";
        cout << "       connected:                " << setw(10) << (double) (SCORE_DEBUG.CONNECTED_ROOKS[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.CONNECTED_ROOKS[BLACK]) / 100.0 << "\n";
//        cout << "       near enemy king           " << setw(10) << (double) (SCORE_DEBUG.XROOK_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.XROOK_NEAR_KING[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "Queen:            " << setw(10) << (double) (Tresult.queens[WHITE] - Tresult.queens[BLACK]) / 100.0 << setw(15) << (double) (Tresult.queens[WHITE]) / 100.0 << setw(10) << (double) (Tresult.queens[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_QUEEN[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_QUEEN[BLACK]) / 100.0 << "\n";
        cout << "       bishop on queen:          " << setw(10) << (double) (SCORE_DEBUG.BISHOP_ON_QUEEN[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.BISHOP_ON_QUEEN[BLACK]) / 100.0 << "\n";
//        cout << "       near enemy king           " << setw(10) << (double) (SCORE_DEBUG.XQUEEN_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.XQUEEN_NEAR_KING[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "King:             " << setw(10) << (double) (Tresult.kings[WHITE] - Tresult.kings[BLACK]) / 100.0 << setw(15) << (double) (Tresult.kings[WHITE]) / 100.0 << setw(10) << (double) (Tresult.kings[BLACK]) / 100.0 << "\n";
        cout << "       distance:                 " << setw(10) << (double) (SCORE_DEBUG.DISTANCE_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.DISTANCE_KING[BLACK]) / 100.0 << "\n";
        cout << "       open file:                " << setw(10) << (double) (SCORE_DEBUG.END_OPENING_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.END_OPENING_KING[BLACK]) / 100.0 << "\n";
        cout << "       pawn near:                " << setw(10) << (double) (SCORE_DEBUG.PAWN_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_NEAR_KING[BLACK]) / 100.0 << "\n";
//        cout << "       pawn storm:               " << setw(10) << (double) (SCORE_DEBUG.PAWN_STORM[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.PAWN_STORM[BLACK]) / 100.0 << "\n";
        cout << "       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_KING[BLACK]) / 100.0 << "\n";
//        cout << "       bishop near king:         " << setw(10) << (double) (SCORE_DEBUG.BISHOP_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.BISHOP_NEAR_KING[BLACK]) / 100.0 << "\n";
//        cout << "       queen near king:          " << setw(10) << (double) (SCORE_DEBUG.QUEEN_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.QUEEN_NEAR_KING[BLACK]) / 100.0 << "\n";
//        cout << "       knight near king:         " << setw(10) << (double) (SCORE_DEBUG.KNIGHT_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.KNIGHT_NEAR_KING[BLACK]) / 100.0 << "\n";
//        cout << "       rook near king:           " << setw(10) << (double) (SCORE_DEBUG.ROOK_NEAR_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.ROOK_NEAR_KING[BLACK]) / 100.0 << "\n";
        cout << endl;
    }
#endif
    return side ? -result : result;
}

