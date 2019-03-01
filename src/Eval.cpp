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
u64 *Eval::evalHash;

Eval::Eval() {
    if (evalHash == nullptr);
    evalHash = (u64 *) calloc(hashSize, sizeof(u64));
}

Eval::~Eval() {
    free(evalHash);
    evalHash = nullptr;
}

template<int side>
void Eval::openFile() {
    structureEval.openFile = 0;
    structureEval.semiOpenFile[side] = 0;

    for (u64 side_rooks = chessboard[ROOK_BLACK + side]; side_rooks; RESET_LSB(side_rooks)) {
        const int o = BITScanForward(side_rooks);
        if (!(FILE_[o] & (chessboard[WHITE] | chessboard[BLACK])))
            structureEval.openFile |= FILE_[o];
        else if (FILE_[o] & chessboard[side ^ 1])
            structureEval.semiOpenFile[side] |= FILE_[o];
    }
}

/**
 * evaluate pawns for color at phase
 * 1. if no pawns returns -NO_PAWNS
 * 3. if other side has all pawns substracts ENEMIES_ALL_PAWNS
 * 4. add ATTACK_KING * number of attacking pawn to other king
 * 5. space - in OPEN phase PAWN_CENTER * CENTER_MASK
 * 6. // pinned - in END phase substracts 20 * each pinned pawn
 * 7. *king security* - in OPEN phase add at kingSecurityDistance FRIEND_NEAR_KING * each pawn near to king and substracts ENEMY_NEAR_KING * each enemy pawn near to king
 * 8. pawn in 8th - if pawn is in 7' add PAWN_7H. If pawn can go forward add PAWN_IN_8TH for each pawn
 * 9. unprotected - no friends pawn protect it
 * 10. blocked - pawn can't go on
 * 11. isolated - there aren't friend pawns on the two sides - subtracts PAWN_ISOLATED for each pawn
 * 12. doubled - there aren't friend pawns on the two sides - subtracts DOUBLED_PAWNS for each pawn. If it is isolated too substracts DOUBLED_ISOLATED_PAWNS
 * 13. backward - if there isn't friend pawns on sides or on sides in 1 rank below subtracts BACKWARD_PAWN
 * 14. passed - if there isn't friend pawns forward and forward on sides until 8' rank add PAWN_PASSED[side][pos]

 */
template<int side>
pair<short, short>  Eval::evaluatePawn() {
    INC(evaluationCount[side]);
    const u64 ped_friends = chessboard[side];
    if (!ped_friends) {
        ADD(SCORE_DEBUG[MG].NO_PAWNS[side], -NO_PAWNS);
        ADD(SCORE_DEBUG[EG].NO_PAWNS[side], -NO_PAWNS);
        return pair<short, short>(-NO_PAWNS, -NO_PAWNS);
    }
    structureEval.isolated[side] = 0;
    short x;
    short result_mg = 0;
    short result_eg = 0;
    constexpr int xside = side ^1;
    if (bitCount(chessboard[xside]) == 8) {
        result_mg -= ENEMIES_PAWNS_ALL;
        result_eg -= ENEMIES_PAWNS_ALL;
        ADD(SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[side], -ENEMIES_PAWNS_ALL);
        ADD(SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[side], -ENEMIES_PAWNS_ALL);
    }

    // 4.
    x = ATTACK_KING * bitCount(ped_friends & structureEval.kingAttackers[xside]);
    result_mg += x;
    result_eg += x;
    ADD(SCORE_DEBUG[MG].ATTACK_KING_PAWN[side], x);
    ADD(SCORE_DEBUG[EG].ATTACK_KING_PAWN[side], x);

// 5. space
//    if (phase == OPEN) {
    result_mg +=
        PAWN_CENTER * bitCount(ped_friends & CENTER_MASK);
    ADD(SCORE_DEBUG[MG].PAWN_CENTER[side], PAWN_CENTER * bitCount(ped_friends & CENTER_MASK));
//    }
// 6. pinned
//    if (phase == END) {
//        result -= 20 * bitCount(structureEval.pinned[side] & ped_friends);
//    }


// 7.
//    if (phase != OPEN) {
    structureEval.kingSecurityDistanceEG[side] +=
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & ped_friends);

    structureEval.kingSecurityDistanceEG[side] -=
        ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & ped_friends);
//    }

// 8.  pawn in 8th
//    if (phase != OPEN) {
    const u64 pawnsIn7 = PAWNS_7_2[side] & ped_friends;
    result_eg +=
        PAWN_IN_7TH * bitCount(pawnsIn7);
    ADD(SCORE_DEBUG[EG].PAWN_7H[side], PAWN_IN_7TH * bitCount(pawnsIn7));

// TODO se casa in 8 non è attaccabile add bonus
    const u64 pawnsIn8 = (shiftForward<side, 8>(pawnsIn7) & (~structureEval.allPieces) ||
        structureEval.allPiecesSide[xside] &
            (shiftForward<side, 7>(pawnsIn7) |
                shiftForward<side, 9>(pawnsIn7)));
    result_eg +=
        PAWN_IN_8TH * (bitCount(pawnsIn8));
    ADD(SCORE_DEBUG[EG].PAWN_IN_8TH[side], PAWN_IN_8TH * (bitCount(pawnsIn8)));
    ADD(SCORE_DEBUG[EG].PAWN_IN_8TH[side], PAWN_IN_8TH * (bitCount(pawnsIn8)));
//    }

    for (
        u64 p = ped_friends;
        p; RESET_LSB(p)) {
        const int o = BITScanForward(p);
        u64 pos = POW2[o];

/// blocked
        x = (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside]))
                && (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED : 0;
        result_mg -=
            x;
        result_eg -=
            x;
        ADD(SCORE_DEBUG[MG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED : 0);
        ADD(SCORE_DEBUG[EG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED : 0);
/// unprotected
        if (!(
            ped_friends & PAWN_PROTECTED_MASK[side][o]
        )) {
            result_mg -=
                UNPROTECTED_PAWNS;
            result_eg -=
                UNPROTECTED_PAWNS;
            ADD(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
            ADD(SCORE_DEBUG[EG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
        }
/// isolated
        if (!(
            ped_friends & PAWN_ISOLATED_MASK[o]
        )) {
            result_mg -=
                PAWN_ISOLATED;
            result_eg -=
                PAWN_ISOLATED;
            ADD(SCORE_DEBUG[MG].PAWN_ISOLATED[side], -PAWN_ISOLATED);
            ADD(SCORE_DEBUG[EG].PAWN_ISOLATED[side], -PAWN_ISOLATED);
            structureEval.isolated[side] |=
                pos;
        }
/// doubled
        if (NOTPOW2[o] & FILE_[o] & ped_friends) {
            result_mg -=
                DOUBLED_PAWNS;
            result_eg -=
                DOUBLED_PAWNS;
            ADD(SCORE_DEBUG[EG].DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            ADD(SCORE_DEBUG[MG].DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
/// doubled and isolated
            if (!(structureEval.isolated[side] & pos)) {
                ADD(SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                ADD(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                result_mg -=
                    DOUBLED_ISOLATED_PAWNS;
                result_eg -=
                    DOUBLED_ISOLATED_PAWNS;
            }
        }
/// backward
        if (!(
            ped_friends & PAWN_BACKWARD_MASK[side][o]
        )) {
            ADD(SCORE_DEBUG[EG].BACKWARD_PAWN[side], -BACKWARD_PAWN);
            ADD(SCORE_DEBUG[MG].BACKWARD_PAWN[side], -BACKWARD_PAWN);
            result_mg -=
                BACKWARD_PAWN;
            result_eg -=
                BACKWARD_PAWN;
        }
/// passed
        if (!(chessboard[xside] & PAWN_PASSED_MASK[side][o])) {
            ADD(SCORE_DEBUG[EG].PAWN_PASSED[side], PAWN_PASSED[side][o]);
            ADD(SCORE_DEBUG[MG].PAWN_PASSED[side], PAWN_PASSED[side][o]);
            result_mg += PAWN_PASSED[side][o];
            result_eg += PAWN_PASSED[side][o];
        }
    }
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate bishop for color at phase
 * 1. if no bishops returns 0
 * 2. if two bishops add BONUS2BISHOP
 * 3 *king security* - in OPEN phase substracts at kingSecurityDistance ENEMY_NEAR_KING for each bishop close to enmey king
 * 4. undevelop - substracts UNDEVELOPED_BISHOP for each undeveloped bishop
 * 5. mobility add MOB_BISHOP[phase][???]
 * 6. if only one bishop and pawns on same square color substracts n_pawns * BISHOP_PAWN_ON_SAME_COLOR
 * pinned ?
 * 7. outposts
 */
template<int side>
pair<short, short>  Eval::evaluateBishop(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 bishop = chessboard[BISHOP_BLACK + side];

    // 1.
    if (!bishop) pair<short, short>(0, 0);

    short result_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    short result_eg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    const int nBishop = bitCount(bishop);
    short x;
    // 2.
    if (nBishop == 1) {

        x = BISHOP_PAWN_ON_SAME_COLOR * bitCount(chessboard[side] & ChessBoard::colors(BITScanForward(bishop)));
        result_mg -= x;
        result_eg -= x;
        ADD(SCORE_DEBUG[EG].BISHOP_PAWN_ON_SAME_COLOR[side], x);
        ADD(SCORE_DEBUG[MG].BISHOP_PAWN_ON_SAME_COLOR[side], x);
    } else {
        // 2.
        if (/*phase != OPEN &&*/ nBishop > 1) {
            result_eg += BONUS2BISHOP;
            ADD(SCORE_DEBUG[EG].BONUS2BISHOP[side], BONUS2BISHOP);
        }
    }

    // 3. *king security*
//    if (phase != OPEN) {
    structureEval.kingSecurityDistanceEG[side] -=
        ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & bishop);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_BISHOP[side],
        -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & bishop));
//    }

    // 4. undevelop
//    if (phase != END) {
    result_mg -= UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop);
    ADD(SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[side], UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop));
//    }

    for (; bishop; RESET_LSB(bishop)) {
        const int o = BITScanForward(bishop);
        // 5. mobility
        u64 captured = getDiagCapture(o, structureEval.allPieces, enemies);
        ASSERT(bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces)
                   < (int) (sizeof(MOB_BISHOP) / sizeof(int)));
        result_mg += MOB_BISHOP[MG][bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces)];
        result_eg += MOB_BISHOP[EG][bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces)];

        ADD(SCORE_DEBUG[MG].MOB_BISHOP[side], x);
        ADD(SCORE_DEBUG[EG].MOB_BISHOP[side], x);

        // 6.
//        if (phase != OPEN) {
        if (BIG_DIAGONAL & POW2[o] && !(DIAGONAL[o] & structureEval.allPieces)) { //TODO sbagliato
            ADD(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[side], OPEN_FILE);
            result_eg += OPEN_FILE;
        }
        if (BIG_ANTIDIAGONAL & POW2[o] && !(ANTIDIAGONAL[o] & structureEval.allPieces)) {//TODO sbagliato
            ADD(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[side], OPEN_FILE);
            result_eg += OPEN_FILE;
        }
//        }

        // 7. outposts
        auto p = BISHOP_OUTPOST[side][o];
        constexpr int xside = side ^1;
        //enemy pawn doesn't attack bishop
        if (p && !(PAWN_FORK_MASK[side ^ 1][o] & chessboard[side ^ 1])) {
            //friend paws defends bishop
            if (PAWN_FORK_MASK[side ^ 1][o] & chessboard[side]) {
                result_mg += p;
                result_eg += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(o))) {
                    result_eg += p;
                    result_mg += p;
                }
            }
        }
        RESET_LSB(bishop);
    }
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate queen for color at phase
 * 1. // pinned
 * 2. *king security* - in OPEN phase add at kingSecurityDistance FRIEND_NEAR_KING for each queen near to king and substracts ENEMY_NEAR_KING for each queen near to enemy king
 * 3. mobility - MOB_QUEEN[phase][position]
 * 4. half open file - if there is a enemy pawn on same file add HALF_OPEN_FILE_Q
 * 5. open file - if there is any pieces on same file add OPEN_FILE_Q
 * 6. 5. bishop on queen - if there is a bishop on same diagonal add BISHOP_ON_QUEEN
 */
template<int side>
pair<short, short> Eval::evaluateQueen(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 queen = chessboard[QUEEN_BLACK + side];
    short result_mg = 0;//20 * bitCount(structureEval.pinned[side] & queen);
    short result_eg = 0;//20 * bitCount(structureEval.pinned[side] & queen);
    short x;
    // 2. *king security*
//    if (phase != OPEN) {
    structureEval.kingSecurityDistanceEG[side] +=
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_QUEEN[side],
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen));

    structureEval.kingSecurityDistanceEG[side] -=
        ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & queen);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_QUEEN[side ^ 1],
        -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & queen));
//    }

    for (; queen; RESET_LSB(queen)) {
        const int o = BITScanForward(queen);
        ASSERT(structureEval.allPieces == structureEval.allPieces);
        // 3. mobility
        result_mg += MOB_QUEEN[MG][getMobilityQueen(o, enemies, structureEval.allPieces)];
        result_mg += MOB_QUEEN[EG][getMobilityQueen(o, enemies, structureEval.allPieces)];

        ADD(SCORE_DEBUG[EG].MOB_QUEEN[side], x);
        ADD(SCORE_DEBUG[MG].MOB_QUEEN[side], x);

        // 4. half open file
        if ((chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG[MG].HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            ADD(SCORE_DEBUG[EG].HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            result_mg += HALF_OPEN_FILE_Q; //TODO + o - ?
            result_eg += HALF_OPEN_FILE_Q; //TODO + o - ?
        }

        // 5. open file
        if ((FILE_[o] & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG[MG].OPEN_FILE_Q[side], OPEN_FILE_Q);
            ADD(SCORE_DEBUG[EG].OPEN_FILE_Q[side], OPEN_FILE_Q);
            result_mg += OPEN_FILE_Q; //TODO + o - ?
            result_eg += OPEN_FILE_Q; //TODO + o - ?
        }

        // 6. bishop on queen
        if (DIAGONAL_ANTIDIAGONAL[o] & chessboard[BISHOP_BLACK + side]) {
            ADD(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            ADD(SCORE_DEBUG[EG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            result_mg += BISHOP_ON_QUEEN;
            result_eg += BISHOP_ON_QUEEN;
        }
    }
    return pair<short, short>(result_mg, result_eg);
}

/**
 * evaluate knight for color at phase
 * 1. // pinned
 * 2. undevelop - substracts UNDEVELOPED_KNIGHT for each undeveloped knight
 * 3. trapped TODO
 * 4. *king security* - in OPEN phase add at kingSecurityDistance FRIEND_NEAR_KING for each knight near to king and substracts ENEMY_NEAR_KING for each knight near to enemy king
 * 5. mobility
 * 6. outposts
*/

template<int side>
pair<short, short> Eval::evaluateKnight(const u64 enemiesPawns, const u64 notMyBits) {
    INC(evaluationCount[side]);
    u64 knight = chessboard[KNIGHT_BLACK + side];
    //if (!x) return 0;TODO

    // 1. pinned
    short result_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    short result_eg = 0;//20 * bitCount(structureEval.pinned[side] & x);

    // 2. undevelop
//    if (phase == OPEN) {
    result_mg -= bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT;
    ADD(SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[side],
        bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT);
//    }
    short x;
    // 3. trapped
    if (side == WHITE) {
        if ((A7bit & knight) && (B7bit & enemiesPawns) && (C6A6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((H7bit & knight) && (G7bit & enemiesPawns) && (F6H6bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((A8bit & knight) && (A7C7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((H8bit & knight) && (H7G7bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
    } else {
        if ((A2bit & knight) && (B2bit & enemiesPawns) && (C3A3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((H2bit & knight) && (G2bit & enemiesPawns) && (F3H3bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((A1bit & knight) && (A2C2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
        if ((H1bit & knight) && (H2G2bit & enemiesPawns)) {
            ADD(SCORE_DEBUG[MG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            ADD(SCORE_DEBUG[EG].KNIGHT_TRAPPED[side], -KNIGHT_TRAPPED);
            result_mg -= KNIGHT_TRAPPED;
            result_eg -= KNIGHT_TRAPPED;
        }
    }

    // 4. king security
//    if (phase != OPEN) {
    structureEval.kingSecurityDistanceEG[side] +=
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_KNIGHT[side],
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight));

    structureEval.kingSecurityDistanceEG[side] -=
        ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & knight);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_KNIGHT[side ^ 1],
        -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & knight));
//    }
    for (; knight; RESET_LSB(knight)) {
        const int pos = BITScanForward(knight);

        // 5. mobility
        ASSERT(bitCount(notMyBits & KNIGHT_MASK[pos]) < (int) (sizeof(MOB_KNIGHT) / sizeof(int)));
        x = MOB_KNIGHT[bitCount(notMyBits & KNIGHT_MASK[pos])];
        result_mg += x;
        result_eg += x;
        ADD(SCORE_DEBUG[MG].MOB_KNIGHT[side], x);
        ADD(SCORE_DEBUG[EG].MOB_KNIGHT[side], x);

        // 6. outposts
        auto p = KNIGHT_OUTPOST[side][pos];
        constexpr int xside = side ^1;
        //enemy pawn doesn't attack knight
        if (p && !(PAWN_FORK_MASK[side ^ 1][pos] & chessboard[side ^ 1])) {
            //friend paws defends knight
            if (PAWN_FORK_MASK[side ^ 1][pos] & chessboard[side]) {
                result_mg += p;
                result_eg += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(pos))) {
                    result_mg += p;
                    result_eg += p;
                }
            }
        }
    }
    return pair<short, short>(result_mg, result_eg);
}


/**
 * evaluate rook for color at phase
 * 1. if no rooks returns 0
 * 2. // pinned
 * 3. in middle if in 7th - add ROOK_7TH_RANK for each rook in 7th
 * 4. *king security* - in OPEN phase add at kingSecurityDistance FRIEND_NEAR_KING for each rook near to king and substracts ENEMY_NEAR_KING for each rook near to enemy king
 * 5. add OPEN_FILE/HALF_OPEN_FILE if the rook is on open/semiopen file
 * 6. trapped
 * 7. 2 linked towers
 * 8. Penalise if Rook is Blocked Horizontally
*/
template<int side>
pair<short, short> Eval::evaluateRook(const u64 king, const u64 enemies, const u64 friends) {
    INC(evaluationCount[side]);

    u64 rook = chessboard[ROOK_BLACK + side];
    if (!rook) return pair<short, short>(0, 0);

    const int nRooks = bitCount(rook);
    // 2.
    short result_mg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    short result_eg = 0;//20 * bitCount(structureEval.pinned[side] & x);
    constexpr int xside = side ^1;
    // 3. in 7th
//    if (phase == MIDDLE) {
    result_mg += ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]);
    ADD(SCORE_DEBUG[MG].ROOK_7TH_RANK[side], ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]));
//    }

    // 4. king security
//    if (phase != OPEN) {
    structureEval.kingSecurityDistanceEG[side] +=
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_ROOK[side],
        FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook));

    structureEval.kingSecurityDistanceEG[side] -=
        ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook);
    ADD(SCORE_DEBUG[EG].KING_SECURITY_ROOK[xside],
        -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook));

//    }

    // .6
    if (((F1G1bit[side] & king) && (H1H2G1bit[side] & rook)) || ((C1B1bit[side] & king) && (A1A2B1bit[side] & rook))) {
        ADD(SCORE_DEBUG[MG].ROOK_TRAPPED[side], -ROOK_TRAPPED);
        ADD(SCORE_DEBUG[EG].ROOK_TRAPPED[side], -ROOK_TRAPPED);
        result_mg -= ROOK_TRAPPED;
        result_eg -= ROOK_TRAPPED;
    }

    // .7
    if (nRooks == 2) {
        const int firstRook = BITScanForward(rook);
        const int secondRook = BITScanReverse(rook);
        if ((!(LINK_ROOKS[firstRook][secondRook] & structureEval.allPieces))) {
            ADD(SCORE_DEBUG[MG].CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            ADD(SCORE_DEBUG[EG].CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            result_mg += CONNECTED_ROOKS;
            result_eg += CONNECTED_ROOKS;
        }
    }

    for (; rook; RESET_LSB(rook)) {
        const int o = BITScanForward(rook);
        //mobility
        ASSERT(getMobilityRook(o, enemies, friends) < (int) (sizeof(MOB_ROOK[EG]) / sizeof(int)));
        result_mg += MOB_ROOK[MG][getMobilityRook(o, enemies, friends)];
        result_eg += MOB_ROOK[EG][getMobilityRook(o, enemies, friends)];
        ADD(SCORE_DEBUG[MG].MOB_ROOK[side], MOB_ROOK[MG][getMobilityRook(o, enemies, friends)]);
        ADD(SCORE_DEBUG[EG].MOB_ROOK[side], MOB_ROOK[EG][getMobilityRook(o, enemies, friends)]);

//        if (phase != OPEN) {
        // .8 Penalise if Rook is Blocked Horizontally
        if ((RANK_BOUND[o] & structureEval.allPieces) == RANK_BOUND[o]) {
            ADD(SCORE_DEBUG[EG].ROOK_BLOCKED[side], -ROOK_BLOCKED);
            result_eg -= ROOK_BLOCKED;
        }
//        }

        // .5
        if (!(chessboard[side] & FILE_[o])) {
            ADD(SCORE_DEBUG[MG].ROOK_OPEN_FILE[side], OPEN_FILE);
            ADD(SCORE_DEBUG[EG].ROOK_OPEN_FILE[side], OPEN_FILE);
            result_mg += OPEN_FILE;
            result_eg += OPEN_FILE;
        }
        if (!(chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG[MG].ROOK_OPEN_FILE[side], OPEN_FILE);
            ADD(SCORE_DEBUG[EG].ROOK_OPEN_FILE[side], OPEN_FILE);
            result_mg += OPEN_FILE;
            result_eg += OPEN_FILE;
        }
    }
    return pair<short, short>(result_mg, result_eg);
}

pair<short, short> Eval::evaluateKing(int side, u64 squares) {
    ASSERT(evaluationCount[side] == 5);
    short result_mg = 0;
    short result_eg = 0;
    uchar pos_king = structureEval.posKing[side];
//    if (phase == END) {
    ADD(SCORE_DEBUG[EG].DISTANCE_KING[side], DISTANCE_KING_ENDING[pos_king]);
    result_eg = DISTANCE_KING_ENDING[pos_king];
//    } else {
    ADD(SCORE_DEBUG[MG].DISTANCE_KING[side], DISTANCE_KING_OPENING[pos_king]);
    result_mg = DISTANCE_KING_OPENING[pos_king];
//    }
    u64 POW2_king = POW2[pos_king];
    //mobility
    ASSERT(bitCount(squares & NEAR_MASK1[pos_king]) < (int) (sizeof(MOB_KING[EG]) / sizeof(int)));
    result_mg += MOB_KING[MG][bitCount(squares & NEAR_MASK1[pos_king])];
    result_eg += MOB_KING[EG][bitCount(squares & NEAR_MASK1[pos_king])];

    ADD(SCORE_DEBUG[MG].MOB_KING[side], MOB_KING[MG][bitCount(squares & NEAR_MASK1[pos_king])]);
    ADD(SCORE_DEBUG[EG].MOB_KING[side], MOB_KING[EG][bitCount(squares & NEAR_MASK1[pos_king])]);
//    if (phase != OPEN) {
    if ((structureEval.openFile & POW2_king) || (structureEval.semiOpenFile[side ^ 1] & POW2_king)) {
        ADD(SCORE_DEBUG[EG].END_OPENING_KING[side], -END_OPENING);
        result_eg -= END_OPENING;
        if (bitCount(RANK[pos_king]) < 4) {
            ADD(SCORE_DEBUG[EG].END_OPENING_KING[side], -END_OPENING);
            result_eg -= END_OPENING;
        }
    }
//    }
    ASSERT(pos_king < 64);
    if (!(NEAR_MASK1[pos_king] & chessboard[side])) {
        ADD(SCORE_DEBUG[MG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        ADD(SCORE_DEBUG[EG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        result_mg -= PAWN_NEAR_KING;
        result_eg -= PAWN_NEAR_KING;
    }
    result_eg += structureEval.kingSecurityDistanceEG[side];
    return pair<short, short>(result_mg, result_eg);
}

void Eval::storeHashValue(const u64 key, const short value) {
    evalHash[key % hashSize] = (key & keyMask) | (value & valueMask); //TODO lockless
    ASSERT(value == getHashValue(key));
}

short Eval::getHashValue(const u64 key) const {
    const u64 kv = evalHash[key % hashSize];
    if ((kv & keyMask) == (key & keyMask))
        return (short) (kv & valueMask);

    return noHashValue;
}

short Eval::getScore(const u64 key, const int side, const int N_PIECE, const int alpha, const int beta,
                     const bool trace) {

    const short hashValue = getHashValue(key);
    if (hashValue != noHashValue)
        return side ? -hashValue : hashValue;

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

#ifdef DEBUG_MODE



    evaluationCount[WHITE] = evaluationCount[BLACK] = 0;
    memset(&SCORE_DEBUG, 0, sizeof(_TSCORE_DEBUG));
#endif
    memset(structureEval.kingSecurityDistanceEG, 0, sizeof(structureEval.kingSecurityDistanceEG));
    int npieces = getNpiecesNoPawnNoKing<WHITE>() + getNpiecesNoPawnNoKing<BLACK>();

    structureEval.allPiecesNoPawns[BLACK] = getBitmapNoPawns<BLACK>();
    structureEval.allPiecesNoPawns[WHITE] = getBitmapNoPawns<WHITE>();
    structureEval.allPiecesSide[BLACK] = structureEval.allPiecesNoPawns[BLACK] | chessboard[PAWN_BLACK];
    structureEval.allPiecesSide[WHITE] = structureEval.allPiecesNoPawns[WHITE] | chessboard[PAWN_WHITE];
    structureEval.allPieces = structureEval.allPiecesSide[BLACK] | structureEval.allPiecesSide[WHITE];
    structureEval.posKing[BLACK] = (uchar) BITScanForward(chessboard[KING_BLACK]);
    structureEval.posKing[WHITE] = (uchar) BITScanForward(chessboard[KING_WHITE]);
    structureEval.kingAttackers[WHITE] = getAllAttackers<WHITE>(structureEval.posKing[WHITE], structureEval.allPieces);
    structureEval.kingAttackers[BLACK] = getAllAttackers<BLACK>(structureEval.posKing[BLACK], structureEval.allPieces);
//    if (phase == END) {
//
//        structureEval.pinned[BLACK] = getPinned<BLACK>(structureEval.allPieces, structureEval.allPiecesSide[BLACK],
//                                                       structureEval.posKing[BLACK]);
//
//        structureEval.pinned[WHITE] = getPinned<WHITE>(structureEval.allPieces, structureEval.allPiecesSide[WHITE],
//                                                       structureEval.posKing[WHITE]);
//    } else {
//        structureEval.pinned[BLACK] = structureEval.pinned[WHITE] = 0;
//    }
    openFile<WHITE>();
    openFile<BLACK>();

//    if (phase != OPEN) {
    short bonus_attack_king_blackEG = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[WHITE])];
    short bonus_attack_king_whiteEG = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[BLACK])];
//    }
    _Tresult tresult[2];
    getScores(tresult);
//    switch (phase) {
//        case OPEN :
//            getRes < OPEN > (tresult);
//            break;
//        case END :
//            getRes < END > (tresult);
//            break;
//        case MIDDLE:
//            getRes < MIDDLE > (tresult);
//            break;
//        default:
//            break;
//    }

    ASSERT(getMobilityCastle(WHITE, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[MG]) / sizeof(int)));
    ASSERT(getMobilityCastle(BLACK, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[MG]) / sizeof(int)));
    short x = getMobilityCastle(WHITE, structureEval.allPieces);
    int mobWhite_mg = MOB_CASTLE[MG][x];
    int mobWhite_eg = MOB_CASTLE[EG][x];
    x = getMobilityCastle(BLACK, structureEval.allPieces);
    int mobBlack_mg = MOB_CASTLE[MG][x];
    int mobBlack_eg = MOB_CASTLE[EG][x];
    int attack_king_white = ATTACK_KING * bitCount(structureEval.kingAttackers[BLACK]);
    int attack_king_black = ATTACK_KING * bitCount(structureEval.kingAttackers[WHITE]);
    side == WHITE ? lazyscore_black -= 5 : lazyscore_white += 5;

    int result_mg = (mobBlack_mg + attack_king_black + lazyscore_black + tresult[MG].pawns[BLACK] +
        tresult[MG].knights[BLACK] + tresult[MG].bishop[BLACK] + tresult[MG].rooks[BLACK] + tresult[MG].queens[BLACK] +
        tresult[MG].kings[BLACK]) -
        (mobWhite_mg + attack_king_white + lazyscore_white + tresult[MG].pawns[WHITE] +
            tresult[MG].knights[WHITE] + tresult[MG].bishop[WHITE] + tresult[MG].rooks[WHITE]
            + tresult[MG].queens[WHITE] +
            tresult[MG].kings[WHITE]);

    int result_eg =
        (mobBlack_eg + attack_king_black + bonus_attack_king_blackEG + lazyscore_black + tresult[EG].pawns[BLACK] +
            tresult[EG].knights[BLACK] + tresult[EG].bishop[BLACK] + tresult[EG].rooks[BLACK]
            + tresult[EG].queens[BLACK] +
            tresult[EG].kings[BLACK]) -
            (mobWhite_eg + attack_king_white + bonus_attack_king_whiteEG + lazyscore_white + tresult[EG].pawns[WHITE] +
                tresult[EG].knights[WHITE] + tresult[EG].bishop[WHITE] + tresult[EG].rooks[WHITE]
                + tresult[EG].queens[WHITE] +
                tresult[EG].kings[WHITE]);

    const int phase = min(MAX_VALUE, lazyscore_white + lazyscore_black);
    const short score = ((result_mg * phase) + (result_eg * (MAX_VALUE - phase))) / MAX_VALUE;

    storeHashValue(key, score);
    auto r = side ? -score : score;

#ifdef DEBUG_MODE
    if (trace) {
//        const string HEADER = "\n";//|\t\t\t\t\tWHITE\t\tBLACK\t\t  WHITE\t\tBLACK\t\t  WHITE\t\tBLACK\n";

        cout << "|OPEN FILE: ";
        if (!structureEval.openFile)cout << "none";
        else
            for (int i = 0; i < 8; i++) if (POW2[i] & structureEval.openFile)cout << (char) (65 + i) << " ";
        cout << "\n";


        cout << "|VALUES:";
        cout << "\tPAWN: " << (double) _board::VALUEPAWN / 100.0;
        cout << " ROOK: " << (double) _board::VALUEROOK / 100.0;
        cout << " BISHOP: " << (double) _board::VALUEBISHOP / 100.0;
        cout << " KNIGHT: " << (double) _board::VALUEKNIGHT / 100.0;
        cout << " QUEEN: " << (double) _board::VALUEQUEEN / 100.0 << "\n\n";


//        cout << HEADER;
        cout << "|Material:         " << setw(10) << (double) (lazyscore_white - lazyscore_black) / 100.0 << setw(15)
            << (double) (lazyscore_white) / 100.0 << setw(10) << (double) (lazyscore_black) / 100.0 << "\n";
        cout << "|Mobility:         " << setw(10) << (double) (mobWhite_mg - mobBlack_mg) / 100.0 << setw(15)
            << (double) (mobWhite_mg) / 100.0 << setw(10) << (double) (mobBlack_mg) / 100.0 << "\n";
        cout << "|Bonus attack king (EG):" << setw(5)
            << (double) (bonus_attack_king_whiteEG - bonus_attack_king_blackEG) / 100.0 << setw(15)
            << (double) (bonus_attack_king_whiteEG) / 100.0 << setw(10) << (double) (bonus_attack_king_blackEG) / 100.0
            << endl;
        cout << endl;
        cout << "|\t\tEval term\t|\t   Total\t\t|\t   White\t\t|\t   Black\t\t|\n"
            "|\t\t\t\t\t|\tMG\t\tEG\t\t|\tMG\t\tEG\t\t|\tMG\t\tEG\t\t|\n"
            "|-------------------+-------------------+-------------------+-------------------+\n";
        cout << "|PAWN\t\t\t\t|";

        p((tresult[MG].pawns[WHITE] - tresult[MG].pawns[BLACK]), (tresult[EG].pawns[WHITE] - tresult[EG].pawns[BLACK]));

        p(tresult[MG].pawns[WHITE], tresult[MG].pawns[BLACK]);
        p(tresult[EG].pawns[WHITE], tresult[EG].pawns[BLACK]);


        cout << "\n|attack king\t\t";
        p();
        p(SCORE_DEBUG[MG].ATTACK_KING_PAWN[WHITE], SCORE_DEBUG[MG].ATTACK_KING_PAWN[BLACK]);
        p(SCORE_DEBUG[EG].ATTACK_KING_PAWN[WHITE], SCORE_DEBUG[EG].ATTACK_KING_PAWN[BLACK]);

        cout << "\n|1 bishop pawn s/c\t";
        p();
        p(SCORE_DEBUG[MG].BISHOP_PAWN_ON_SAME_COLOR[WHITE], SCORE_DEBUG[MG].BISHOP_PAWN_ON_SAME_COLOR[BLACK]);
        p(SCORE_DEBUG[EG].BISHOP_PAWN_ON_SAME_COLOR[WHITE], SCORE_DEBUG[EG].BISHOP_PAWN_ON_SAME_COLOR[BLACK]);


        cout << "\n|center\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_CENTER[WHITE], SCORE_DEBUG[MG].PAWN_CENTER[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_CENTER[WHITE], SCORE_DEBUG[EG].PAWN_CENTER[BLACK]);

        cout << "\n|in 7th\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_7H[WHITE], SCORE_DEBUG[MG].PAWN_7H[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_7H[WHITE], SCORE_DEBUG[EG].PAWN_7H[BLACK]);
        cout << "\n|in 8th\t\t\t\t";

        p();
        p(SCORE_DEBUG[MG].PAWN_IN_8TH[WHITE], SCORE_DEBUG[MG].PAWN_IN_8TH[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_IN_8TH[WHITE], SCORE_DEBUG[EG].PAWN_IN_8TH[BLACK]);

        cout << "\n|blocked\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_BLOCKED[WHITE], SCORE_DEBUG[MG].PAWN_BLOCKED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_BLOCKED[WHITE], SCORE_DEBUG[EG].PAWN_BLOCKED[BLACK]);

        cout << "\n|unprotected\t\t";
        p();
        p(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[WHITE], SCORE_DEBUG[MG].UNPROTECTED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].UNPROTECTED_PAWNS[WHITE], SCORE_DEBUG[EG].UNPROTECTED_PAWNS[BLACK]);

        cout << "\n|isolated\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_ISOLATED[WHITE], SCORE_DEBUG[MG].PAWN_ISOLATED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_ISOLATED[WHITE], SCORE_DEBUG[EG].PAWN_ISOLATED[BLACK]);

        cout << "\n|double\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].DOUBLED_PAWNS[WHITE], SCORE_DEBUG[MG].DOUBLED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].DOUBLED_PAWNS[WHITE], SCORE_DEBUG[EG].DOUBLED_PAWNS[BLACK]);

        cout << "\n|double isolated\t";
        p();
        p(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[WHITE], SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[WHITE], SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[BLACK]);

        cout << "\n|backward\t\t\t";
        p();
        p(SCORE_DEBUG[MG].BACKWARD_PAWN[WHITE], SCORE_DEBUG[MG].BACKWARD_PAWN[BLACK]);
        p(SCORE_DEBUG[EG].BACKWARD_PAWN[WHITE], SCORE_DEBUG[EG].BACKWARD_PAWN[BLACK]);

        cout << "\n|fork\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].FORK_SCORE[WHITE], SCORE_DEBUG[MG].FORK_SCORE[BLACK]);
        p(SCORE_DEBUG[EG].FORK_SCORE[WHITE], SCORE_DEBUG[EG].FORK_SCORE[BLACK]);

        cout << "\n|passed\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].PAWN_PASSED[WHITE], SCORE_DEBUG[MG].PAWN_PASSED[BLACK]);
        p(SCORE_DEBUG[EG].PAWN_PASSED[WHITE], SCORE_DEBUG[EG].PAWN_PASSED[BLACK]);

        cout << "\n|all enemies\t\t";
        p();
        p(SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[WHITE], SCORE_DEBUG[MG].ENEMIES_PAWNS_ALL[BLACK]);
        p(SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[WHITE], SCORE_DEBUG[EG].ENEMIES_PAWNS_ALL[BLACK]);

        cout << "\n|none\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].NO_PAWNS[WHITE], SCORE_DEBUG[MG].NO_PAWNS[BLACK]);
        p(SCORE_DEBUG[EG].NO_PAWNS[WHITE], SCORE_DEBUG[EG].NO_PAWNS[BLACK]);

//        cout << HEADER;
        cout << "\n|KNIGHT\t\t\t\t|";
        p((tresult[MG].knights[WHITE] - tresult[MG].knights[BLACK]),
          (tresult[EG].knights[WHITE] - tresult[EG].knights[BLACK]));

        p(tresult[MG].knights[WHITE], tresult[MG].knights[BLACK]);
        p(tresult[EG].knights[WHITE], tresult[EG].knights[BLACK]);

        cout << "\n|undevelop\t\t\t";
        p();
        p(SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[WHITE], SCORE_DEBUG[MG].UNDEVELOPED_KNIGHT[BLACK]);
        p(SCORE_DEBUG[EG].UNDEVELOPED_KNIGHT[WHITE], SCORE_DEBUG[EG].UNDEVELOPED_KNIGHT[BLACK]);

        cout << "\n|trapped\t\t\t";
        p();
        p(SCORE_DEBUG[MG].KNIGHT_TRAPPED[WHITE], SCORE_DEBUG[MG].KNIGHT_TRAPPED[BLACK]);
        p(SCORE_DEBUG[EG].KNIGHT_TRAPPED[WHITE], SCORE_DEBUG[EG].KNIGHT_TRAPPED[BLACK]);

        cout << "\n|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_KNIGHT[WHITE], SCORE_DEBUG[MG].MOB_KNIGHT[BLACK]);
        p(SCORE_DEBUG[EG].MOB_KNIGHT[WHITE], SCORE_DEBUG[EG].MOB_KNIGHT[BLACK]);

//        cout << HEADER;
        cout << "\n|BISHOP\t\t\t\t|";
        p((tresult[MG].bishop[WHITE] - tresult[MG].bishop[BLACK]),
          (tresult[EG].bishop[WHITE] - tresult[EG].bishop[BLACK]));

        p(tresult[MG].bishop[WHITE], tresult[MG].bishop[BLACK]);
        p(tresult[EG].bishop[WHITE], tresult[EG].bishop[BLACK]);

        cout << "\n|bad\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].BAD_BISHOP[WHITE], SCORE_DEBUG[MG].BAD_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].BAD_BISHOP[WHITE], SCORE_DEBUG[EG].BAD_BISHOP[BLACK]);

        cout << "\n|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_BISHOP[WHITE], SCORE_DEBUG[MG].MOB_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].MOB_BISHOP[WHITE], SCORE_DEBUG[EG].MOB_BISHOP[BLACK]);

        cout << "\n|undevelop\t\t\t";
        p();
        p(SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[WHITE], SCORE_DEBUG[MG].UNDEVELOPED_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].UNDEVELOPED_BISHOP[WHITE], SCORE_DEBUG[EG].UNDEVELOPED_BISHOP[BLACK]);

        cout << "\n|open diag\t\t\t";
        p();
        p(SCORE_DEBUG[MG].OPEN_DIAG_BISHOP[WHITE], SCORE_DEBUG[MG].OPEN_DIAG_BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[WHITE], SCORE_DEBUG[EG].OPEN_DIAG_BISHOP[BLACK]);

        cout << "\n|bonus 2 bishops\t";
        p();
        p(SCORE_DEBUG[MG].BONUS2BISHOP[WHITE], SCORE_DEBUG[MG].BONUS2BISHOP[BLACK]);
        p(SCORE_DEBUG[EG].BONUS2BISHOP[WHITE], SCORE_DEBUG[EG].BONUS2BISHOP[BLACK]);

        cout << "\n|ROOK\t\t\t\t|";
        p((tresult[MG].rooks[WHITE] - tresult[MG].rooks[BLACK]), (tresult[EG].rooks[WHITE] - tresult[EG].rooks[BLACK]));
        p(tresult[MG].rooks[WHITE], tresult[EG].rooks[BLACK]);
        p(tresult[EG].rooks[BLACK], tresult[EG].rooks[BLACK]);

        cout << "\n|7th\t\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_7TH_RANK[WHITE], SCORE_DEBUG[MG].ROOK_7TH_RANK[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_7TH_RANK[WHITE], SCORE_DEBUG[EG].ROOK_7TH_RANK[BLACK]);

        cout << "\n|trapped\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_TRAPPED[WHITE], SCORE_DEBUG[MG].ROOK_TRAPPED[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_TRAPPED[WHITE], SCORE_DEBUG[EG].ROOK_TRAPPED[BLACK]);

        cout << "\n|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_ROOK[WHITE], SCORE_DEBUG[MG].MOB_ROOK[BLACK]);
        p(SCORE_DEBUG[EG].MOB_ROOK[WHITE], SCORE_DEBUG[EG].MOB_ROOK[BLACK]);

        cout << "\n|blocked\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_BLOCKED[WHITE], SCORE_DEBUG[MG].ROOK_BLOCKED[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_BLOCKED[WHITE], SCORE_DEBUG[EG].ROOK_BLOCKED[BLACK]);

        cout << "\n|open file\t\t\t";
        p();
        p(SCORE_DEBUG[MG].ROOK_OPEN_FILE[WHITE], SCORE_DEBUG[MG].ROOK_OPEN_FILE[BLACK]);
        p(SCORE_DEBUG[EG].ROOK_OPEN_FILE[WHITE], SCORE_DEBUG[EG].ROOK_OPEN_FILE[BLACK]);

        cout << "\n|connected\t\t\t";
        p();
        p(SCORE_DEBUG[MG].CONNECTED_ROOKS[WHITE], SCORE_DEBUG[MG].CONNECTED_ROOKS[BLACK]);
        p(SCORE_DEBUG[EG].CONNECTED_ROOKS[WHITE], SCORE_DEBUG[EG].CONNECTED_ROOKS[BLACK]);

        cout << "\n|QUEEN\t\t\t\t|";
        p((tresult[MG].queens[WHITE] - tresult[MG].queens[BLACK]),
          (tresult[EG].queens[WHITE] - tresult[EG].queens[BLACK]));
        p(tresult[MG].queens[WHITE], tresult[MG].queens[BLACK]);
        p(tresult[EG].queens[WHITE], tresult[EG].queens[BLACK]);


        cout << "\n|mobility\t\t\t";
        p();
        p(SCORE_DEBUG[MG].MOB_QUEEN[WHITE], SCORE_DEBUG[MG].MOB_QUEEN[BLACK]);
        p(SCORE_DEBUG[MG].MOB_QUEEN[WHITE], SCORE_DEBUG[MG].MOB_QUEEN[BLACK]);

        cout << "\n|bishop on queen";
        p();
        p(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[WHITE], SCORE_DEBUG[MG].BISHOP_ON_QUEEN[BLACK]);
        p(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[WHITE], SCORE_DEBUG[MG].BISHOP_ON_QUEEN[BLACK]);
        
        cout << "\n|King:             " << setw(10) << (tresult[MG].kings[WHITE] - tresult[MG].kings[BLACK]) / 100.0
            << setw(15) << (tresult[MG].kings[WHITE]) / 100.0 << setw(10) << (tresult[MG].kings[BLACK]) / 100.0 <<
            setw(10) << (tresult[MG].kings[WHITE] - tresult[MG].kings[BLACK]) / 100.0 << setw(15)
            << (tresult[MG].kings[WHITE]) / 100.0 << setw(10) << (tresult[MG].kings[BLACK]) / 100.0 << endl;
        cout << "\n|       distance:                 " << setw(10) << (SCORE_DEBUG[MG].DISTANCE_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].DISTANCE_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].DISTANCE_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].DISTANCE_KING[BLACK]) / 100.0 << endl;
        cout << "\n|       open file:                " << setw(10) << (SCORE_DEBUG[MG].END_OPENING_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].END_OPENING_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].END_OPENING_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].END_OPENING_KING[BLACK]) / 100.0 << endl;

        cout << "\n|       pawn near:                " << setw(10) << (SCORE_DEBUG[MG].PAWN_NEAR_KING[WHITE]) / 100.0
            << setw(10) << (SCORE_DEBUG[MG].PAWN_NEAR_KING[BLACK]) / 100.0 <<
            setw(10) << (SCORE_DEBUG[MG].PAWN_NEAR_KING[WHITE]) / 100.0 << setw(10)
            << (SCORE_DEBUG[MG].PAWN_NEAR_KING[BLACK]) / 100.0 << endl;
//      cout << "|       mobility:                 " << setw(10) <<  (SCORE_DEBUG[MG].MOB_KING[WHITE]) / 100.0 << setw(10) <<  (SCORE_DEBUG[MG].MOB_KING[BLACK]) / 100.0 << "\n";
        cout << endl;
        cout << "\n|Total (white)..........   " << (double) (side ? r / 100.0 : -r / 100.0) << "\n";
    }
#endif

    return r;
}

