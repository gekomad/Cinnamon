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

#ifdef BENCH_MODE
Time Eval::evalTime;
Time Eval::bishopTime;
Time Eval::pawnTime;
Time Eval::kingTime;
Time Eval::rookTime;
Time Eval::knightTime;
Time Eval::queenTime;
#endif

Eval::Eval() {
    if (evalHash == nullptr)
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
 * 7. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING * each pawn near to king and substracts ENEMY_NEAR_KING * each enemy pawn near to king
 * 8. pawn in 8th - if pawn is in 7' add PAWN_7H. If pawn can go forward add PAWN_IN_8TH for each pawn
 * 9. unprotected - no friends pawn protect it
 * 10. blocked - pawn can't go on
 * 11. isolated - there aren't friend pawns on the two sides - subtracts PAWN_ISOLATED for each pawn
 * 12. doubled - there aren't friend pawns on the two sides - subtracts DOUBLED_PAWNS for each pawn. If it is isolated too substracts DOUBLED_ISOLATED_PAWNS
 * 13. backward - if there isn't friend pawns on sides or on sides in 1 rank below subtracts BACKWARD_PAWN
 * 14. passed - if there isn't friend pawns forward and forward on sides until 8' rank add PAWN_PASSED[side][pos]

 */
template<int side, Eval::_Tphase phase>
int Eval::evaluatePawn() {
    INC(evaluationCount[side]);
    int result = 0;
    constexpr int xside = side ^1;

    const u64 ped_friends = chessboard[side];

    // 5. space
    if (phase == OPEN) {
        result += PAWN_CENTER * bitCount(ped_friends & CENTER_MASK);
        ADD(SCORE_DEBUG.PAWN_CENTER[side], PAWN_CENTER * bitCount(ped_friends & CENTER_MASK));
    }

    // 7.
    if (phase != OPEN) {
        structureEval.kingSecurity[side] +=
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & ped_friends);

        structureEval.kingSecurity[side] -=
            ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & ped_friends);
    }

    // 8.  pawn in 8th
    if (phase != OPEN) {
        const u64 pawnsIn7 = PAWNS_7_2[side] & ped_friends;
        result += PAWN_IN_7TH * bitCount(pawnsIn7);
        ADD(SCORE_DEBUG.PAWN_7H[side], PAWN_IN_7TH * bitCount(pawnsIn7));

        const u64 pawnsIn8 = (shiftForward<side, 8>(pawnsIn7) & (~structureEval.allPieces)) |
            (structureEval.allPiecesSide[xside] & (shiftForward<side, 7>(pawnsIn7) | shiftForward<side, 9>(pawnsIn7)));

        result += PAWN_IN_8TH * bitCount(pawnsIn8); //try to decrease PAWN_IN_8TH
        ADD(SCORE_DEBUG.PAWN_IN_8TH[side], PAWN_IN_8TH * (bitCount(pawnsIn8)));

    }

    for (u64 p = ped_friends; p; RESET_LSB(p)) {
        bool isolated = false;
        const int o = BITScanForward(p);
        u64 pos = POW2[o];

        // 4. attack king
        if (structureEval.posKingBit[xside] & PAWN_FORK_MASK[side][o]) {
            structureEval.kingAttackers[xside] |= pos;
            result += ATTACK_KING;
        }

        /// blocked
        result -= (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED : 0;
        ADD(SCORE_DEBUG.PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
            (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? -PAWN_BLOCKED
                                                                     : 0);
        /// unprotected
        if (!(ped_friends & PAWN_PROTECTED_MASK[side][o])) {
            result -= UNPROTECTED_PAWNS;
            ADD(SCORE_DEBUG.UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS);
        }
        /// isolated
        if (!(ped_friends & PAWN_ISOLATED_MASK[o])) {
            result -= PAWN_ISOLATED;
            ADD(SCORE_DEBUG.PAWN_ISOLATED[side], -PAWN_ISOLATED);
            isolated = true;
        }
        /// doubled
        if (NOTPOW2[o] & FILE_[o] & ped_friends) {
            result -= DOUBLED_PAWNS;
            ADD(SCORE_DEBUG.DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            /// doubled and isolated
            if (isolated) {
                ADD(SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS);
                result -= DOUBLED_ISOLATED_PAWNS;
            }
        }
        /// backward
        if (!(ped_friends & PAWN_BACKWARD_MASK[side][o])) {
            ADD(SCORE_DEBUG.BACKWARD_PAWN[side], -BACKWARD_PAWN);
            result -= BACKWARD_PAWN;
        }
        /// passed
        if (!(chessboard[xside] & PAWN_PASSED_MASK[side][o])) {
            ADD(SCORE_DEBUG.PAWN_PASSED[side], PAWN_PASSED[side][o]);
            result += PAWN_PASSED[side][o];
        }
    }
    return result;
}

/**
 * evaluate bishop for color at phase
 * 1. if no bishops returns 0
 * 2. if two bishops add BONUS2BISHOP
 * 3 *king security* - in OPEN phase substracts at kingSecurity ENEMY_NEAR_KING for each bishop close to enmey king
 * 4. undevelop - substracts UNDEVELOPED_BISHOP for each undeveloped bishop
 * 5. mobility add MOB_BISHOP[phase][???]
 * 6. if only one bishop and pawns on same square color substracts n_pawns * BISHOP_PAWN_ON_SAME_COLOR
 * 7. outposts
 * 8. bishop on big diagonal
 */
template<int side, Eval::_Tphase phase>
int Eval::evaluateBishop(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 bishop = chessboard[BISHOP_BLACK + side];

    // 1.
    if (!bishop) return 0;

    int result = 0;
    const int nBishop = bitCount(bishop);

    // 2.
    if (nBishop == 1) {
        result -= BISHOP_PAWN_ON_SAME_COLOR * bitCount(chessboard[side] & ChessBoard::colors(BITScanForward(bishop)));
    } else {
        // 2.
        if (phase != OPEN && nBishop > 1) {
            result += BONUS2BISHOP;
            ADD(SCORE_DEBUG.BONUS2BISHOP[side], BONUS2BISHOP);
        }
    }

    // 3. *king security*
    if (phase != OPEN) {
        structureEval.kingSecurity[side] -=
            ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & bishop);
        ADD(SCORE_DEBUG.KING_SECURITY_BISHOP[side],
            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & bishop));
    }

    // 4. undevelop
    if (phase != END) {
        result -= UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop);
        ADD(SCORE_DEBUG.UNDEVELOPED_BISHOP[side], UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop));
    }

    for (; bishop; RESET_LSB(bishop)) {
        const int o = BITScanForward(bishop);
        // 5. mobility

        u64 captured = getDiagCapture(o, structureEval.allPieces, enemies);
        ASSERT(bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces) <
            (int) (sizeof(MOB_BISHOP) / sizeof(int)));

        if (captured & structureEval.posKingBit[side ^ 1]) {
            structureEval.kingAttackers[side ^ 1] |= POW2[o];
        }

        result += MOB_BISHOP[phase][bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces)];
        ADD(SCORE_DEBUG.MOB_BISHOP[side],
            MOB_BISHOP[phase][bitCount(captured) + getDiagShiftCount(o, structureEval.allPieces)]);

        // 6.
        if (phase != OPEN) {
            if ((BIG_DIAGONAL & structureEval.allPieces) == POW2[o]) {
                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
                result += OPEN_FILE;
            }
            if ((BIG_ANTIDIAGONAL & structureEval.allPieces) == POW2[o]) {
                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
                result += OPEN_FILE;
            }
        }

        // 7. outposts
        auto p = BISHOP_OUTPOST[side][o];
        constexpr int xside = side ^1;
        //enemy pawn doesn't attack bishop
        if (p && !(PAWN_FORK_MASK[side ^ 1][o] & chessboard[side ^ 1])) {
            //friend paws defends bishop
            if (PAWN_FORK_MASK[side ^ 1][o] & chessboard[side]) {
                result += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(o))) {
                    result += p;
                }
            }
        }
    }
    return result;
}

/**
 * evaluate queen for color at phase
 * 2. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each queen near to king and substracts ENEMY_NEAR_KING for each queen near to enemy king
 * 3. mobility - MOB_QUEEN[phase][position]
 * 4. half open file - if there is a enemy pawn on same file add HALF_OPEN_FILE_Q
 * 5. open file - if there is any pieces on same file add OPEN_FILE_Q
 * 6. 5. bishop on queen - if there is a bishop on same diagonal add BISHOP_ON_QUEEN
 */
template<int side, Eval::_Tphase phase>
int Eval::evaluateQueen(const u64 enemies) {
    INC(evaluationCount[side]);
    u64 queen = chessboard[QUEEN_BLACK + side];
    int result = 0;

    // 2. *king security*
    if (phase != OPEN) {
        structureEval.kingSecurity[side] +=
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen);
        ADD(SCORE_DEBUG.KING_SECURITY_QUEEN[side],
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen));

        structureEval.kingSecurity[side] -=
            ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & queen);
        ADD(SCORE_DEBUG.KING_SECURITY_QUEEN[side ^ 1],
            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & queen));
    }

    for (; queen; RESET_LSB(queen)) {
        const int o = BITScanForward(queen);
        // 3. mobility
        u64 x = getMobilityQueen(o, enemies, structureEval.allPieces);
        result += MOB_QUEEN[phase][bitCount(x)];
        ADD(SCORE_DEBUG.MOB_QUEEN[side], MOB_QUEEN[phase][bitCount(x)]);

        if (x & structureEval.posKingBit[side ^ 1])
            structureEval.kingAttackers[side ^ 1] |= POW2[o];
        // 4. half open file
        if ((chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG.HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
            result += HALF_OPEN_FILE_Q;
        }

        // 5. open file
        if ((FILE_[o] & structureEval.allPieces) == POW2[o]) {
            ADD(SCORE_DEBUG.OPEN_FILE_Q[side], OPEN_FILE_Q);
            result += OPEN_FILE_Q;
        }

        // 6. bishop on queen
        if (DIAGONAL_ANTIDIAGONAL[o] & chessboard[BISHOP_BLACK + side]) {
            ADD(SCORE_DEBUG.BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN);
            result += BISHOP_ON_QUEEN;
        }
    }
    return result;
}

/**
 * evaluate knight for color at phase
 * 1. // pinned
 * 2. undevelop - substracts UNDEVELOPED_KNIGHT for each undeveloped knight
 * 4. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each knight near to king and substracts ENEMY_NEAR_KING for each knight near to enemy king
 * 5. mobility
 * 6. outposts
*/

template<int side, Eval::_Tphase phase>
int Eval::evaluateKnight(const u64 enemiesPawns, const u64 notMyBits) {
    INC(evaluationCount[side]);
    u64 knight = chessboard[KNIGHT_BLACK + side];
    if (!knight) return 0;

    // 1. pinned
    int result = 0;

    // 2. undevelop
    if (phase == OPEN) {
        result -= bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT;
        ADD(SCORE_DEBUG.UNDEVELOPED_KNIGHT[side],
            bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT);
    }

    // 4. king security
    if (phase != OPEN) {
        structureEval.kingSecurity[side] +=
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight);
        ADD(SCORE_DEBUG.KING_SECURITY_KNIGHT[side],
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight));

        structureEval.kingSecurity[side] -=
            ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & knight);
        ADD(SCORE_DEBUG.KING_SECURITY_KNIGHT[side ^ 1],
            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side ^ 1]] & knight));
    }
    for (; knight; RESET_LSB(knight)) {
        const int pos = BITScanForward(knight);

        // 5. mobility
        ASSERT(bitCount(notMyBits & KNIGHT_MASK[pos]) < (int) (sizeof(MOB_KNIGHT) / sizeof(int)));
        u64 mob = notMyBits & KNIGHT_MASK[pos];
        result += MOB_KNIGHT[bitCount(mob)];
        if (mob & structureEval.posKingBit[side ^ 1]) structureEval.kingAttackers[side ^ 1] |= POW2[pos];
        ADD(SCORE_DEBUG.MOB_KNIGHT[side], MOB_KNIGHT[bitCount(mob)]);

        // 6. outposts
        auto p = KNIGHT_OUTPOST[side][pos];
        constexpr int xside = side ^1;
        //enemy pawn doesn't attack knight
        if (p && !(PAWN_FORK_MASK[side ^ 1][pos] & chessboard[side ^ 1])) {
            //friend paws defends knight
            if (PAWN_FORK_MASK[side ^ 1][pos] & chessboard[side]) {
                result += p;
                if (!(chessboard[KNIGHT_BLACK + xside]) &&
                    !(chessboard[BISHOP_BLACK + xside] & ChessBoard::colors(pos))) {
                    result += p;
                }
            }
        }
    }
    return result;
}


/**
 * evaluate rook for color at phase
 * 1. if no rooks returns 0
 * 3. in middle if in 7th - add ROOK_7TH_RANK for each rook in 7th
 * 4. *king security* - in OPEN phase add at kingSecurity FRIEND_NEAR_KING for each rook near to king and substracts ENEMY_NEAR_KING for each rook near to enemy king
 * 5. add OPEN_FILE/HALF_OPEN_FILE if the rook is on open/semiopen file
 * 6. trapped
 * 7. 2 linked towers
 * 8. Penalise if Rook is Blocked Horizontally
*/
template<int side, Eval::_Tphase phase>
int Eval::evaluateRook(const u64 king, const u64 enemies, const u64 friends) {
    INC(evaluationCount[side]);

    u64 rook = chessboard[ROOK_BLACK + side];
    if (!rook) return 0;

    const int nRooks = bitCount(rook);
    // 2.
    int result = 0;
    constexpr int xside = side ^1;
    // 3. in 7th
    if (phase == MIDDLE) {
        result += ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]);
        ADD(SCORE_DEBUG.ROOK_7TH_RANK[side], ROOK_7TH_RANK * bitCount(rook & RANK_1_7[side]));
    }

    // 4. king security
    if (phase != OPEN) {
        structureEval.kingSecurity[side] +=
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook);
        ADD(SCORE_DEBUG.KING_SECURITY_ROOK[side],
            FRIEND_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook));

        structureEval.kingSecurity[side] -=
            ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook);
        ADD(SCORE_DEBUG.KING_SECURITY_ROOK[xside],
            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook));

    }

    // .6
    if (((F1G1bit[side] & king) && (H1H2G1bit[side] & rook)) || ((C1B1bit[side] & king) && (A1A2B1bit[side] & rook))) {
        ADD(SCORE_DEBUG.ROOK_TRAPPED[side], -ROOK_TRAPPED);
        result -= ROOK_TRAPPED;
    }

    // .7
    if (nRooks == 2) {
        const int firstRook = BITScanForward(rook);
        const int secondRook = BITScanReverse(rook);
        if ((!(LINK_ROOKS[firstRook][secondRook] & structureEval.allPieces))) {
            ADD(SCORE_DEBUG.CONNECTED_ROOKS[side], CONNECTED_ROOKS);
            result += CONNECTED_ROOKS;
        }
    }

    for (; rook; RESET_LSB(rook)) {
        const int o = BITScanForward(rook);
        //mobility
        u64 mob = getMobilityRook(o, enemies, friends);
        if (mob & structureEval.posKingBit[side ^ 1]) structureEval.kingAttackers[side ^ 1] |= POW2[o];

        ASSERT(bitCount(mob) < (int) (sizeof(MOB_ROOK[phase]) / sizeof(int)));
        result += MOB_ROOK[phase][bitCount(mob)];
        ADD(SCORE_DEBUG.MOB_ROOK[side], MOB_ROOK[phase][bitCount(mob)]);

        if (phase != OPEN) {
            // .8 Penalise if Rook is Blocked Horizontally
            if ((RANK_BOUND[o] & structureEval.allPieces) == RANK_BOUND[o]) {
                ADD(SCORE_DEBUG.ROOK_BLOCKED[side], -ROOK_BLOCKED);
                result -= ROOK_BLOCKED;
            }
        }

        // .5
        if (!(chessboard[side] & FILE_[o])) {
            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
            result += OPEN_FILE;
        }
        if (!(chessboard[side ^ 1] & FILE_[o])) {
            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
            result += OPEN_FILE;
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
    if (!(NEAR_MASK1[pos_king] & chessboard[side])) {
        ADD(SCORE_DEBUG.PAWN_NEAR_KING[side], -PAWN_NEAR_KING);
        result -= PAWN_NEAR_KING;
    }
    result += structureEval.kingSecurity[side];
    return result;
}

void Eval::storeHashValue(const u64 key, const short value) {
    evalHash[key % hashSize] = (key & keyMask) | (value & valueMask);
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
    BENCH(evalTime.start());
    const short hashValue = getHashValue(key);
    if (hashValue != noHashValue) {
        BENCH(evalTime.stop());
        return side ? -hashValue : hashValue;
    }
    int lazyscore_white = lazyEvalSide<WHITE>();
    int lazyscore_black = lazyEvalSide<BLACK>();
    int lazyscore = lazyscore_black - lazyscore_white;
    if (side) {
        lazyscore = -lazyscore;
    }
    if (lazyscore > (beta + FUTIL_MARGIN) || lazyscore < (alpha - FUTIL_MARGIN)) {
        INC(lazyEvalCuts);
        BENCH(evalTime.stop());
        return lazyscore;
    }

#ifdef DEBUG_MODE
    evaluationCount[WHITE] = evaluationCount[BLACK] = 0;
    memset(&SCORE_DEBUG, 0, sizeof(_TSCORE_DEBUG));
#endif
    memset(structureEval.kingSecurity, 0, sizeof(structureEval.kingSecurity));
    int npieces = getNpiecesNoPawnNoKing<WHITE>() + getNpiecesNoPawnNoKing<BLACK>();
    _Tphase phase;
    if (npieces < 4) {
        phase = END;
    } else if (npieces < 11) {
        phase = MIDDLE;
    } else {
        phase = OPEN;
    }
    structureEval.allPiecesNoPawns[BLACK] = getBitmapNoPawns<BLACK>();
    structureEval.allPiecesNoPawns[WHITE] = getBitmapNoPawns<WHITE>();
    structureEval.allPiecesSide[BLACK] = structureEval.allPiecesNoPawns[BLACK] | chessboard[PAWN_BLACK];
    structureEval.allPiecesSide[WHITE] = structureEval.allPiecesNoPawns[WHITE] | chessboard[PAWN_WHITE];
    structureEval.allPieces = structureEval.allPiecesSide[BLACK] | structureEval.allPiecesSide[WHITE];
    structureEval.posKing[BLACK] = (uchar) BITScanForward(chessboard[KING_BLACK]);
    structureEval.posKing[WHITE] = (uchar) BITScanForward(chessboard[KING_WHITE]);
    structureEval.posKingBit[BLACK] = POW2[structureEval.posKing[BLACK]];
    structureEval.posKingBit[WHITE] = POW2[structureEval.posKing[WHITE]];
    structureEval.kingAttackers[WHITE] = structureEval.kingAttackers[BLACK] = 0;

    openFile<WHITE>();
    openFile<BLACK>();

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
    int bonus_attack_king_black = 0;
    int bonus_attack_king_white = 0;
    if (phase != OPEN) {
        bonus_attack_king_black = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[WHITE])];
        bonus_attack_king_white = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[BLACK])];
    }

    ASSERT(getMobilityCastle(WHITE, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[phase]) / sizeof(int)));
    ASSERT(getMobilityCastle(BLACK, structureEval.allPieces) < (int) (sizeof(MOB_CASTLE[phase]) / sizeof(int)));
    int mobWhite = MOB_CASTLE[phase][getMobilityCastle(WHITE, structureEval.allPieces)];
    int mobBlack = MOB_CASTLE[phase][getMobilityCastle(BLACK, structureEval.allPieces)];
    int attack_king_white = ATTACK_KING * bitCount(structureEval.kingAttackers[BLACK]);
    int attack_king_black = ATTACK_KING * bitCount(structureEval.kingAttackers[WHITE]);
    side == WHITE ? lazyscore_black -= 5 : lazyscore_white += 5;
    int result = (mobBlack + attack_king_black + bonus_attack_king_black + lazyscore_black + Tresult.pawns[BLACK] +
        Tresult.knights[BLACK] + Tresult.bishop[BLACK] + Tresult.rooks[BLACK] + Tresult.queens[BLACK] +
        Tresult.kings[BLACK]) -
        (mobWhite + attack_king_white + bonus_attack_king_white + lazyscore_white + Tresult.pawns[WHITE] +
            Tresult.knights[WHITE] + Tresult.bishop[WHITE] + Tresult.rooks[WHITE] + Tresult.queens[WHITE] +
            Tresult.kings[WHITE]);

#ifdef DEBUG_MODE
    if (trace) {
        const string HEADER = "\n|\t\t\t\t\tTOT (white)\t\t  WHITE\t\tBLACK\n";
        cout << "|PHASE: ";
        if (phase == OPEN) {
            cout << " OPEN\n";
        } else if (phase == MIDDLE) {
            cout << " MIDDLE\n";
        } else {
            cout << " END\n";
        }
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

        cout << HEADER;
        cout << "|Material:         " << setw(10) << (double) (lazyscore_white - lazyscore_black) / 100.0 << setw(15) <<
            (double) (lazyscore_white) / 100.0 << setw(10) << (double) (lazyscore_black) / 100.0 << "\n";
        cout << "|Mobility:         " << setw(10) << (double) (mobWhite - mobBlack) / 100.0 << setw(15) <<
            (double) (mobWhite) / 100.0 << setw(10) << (double) (mobBlack) / 100.0 << "\n";
        cout << "|Bonus attack king:" << setw(10) <<
            (double) (bonus_attack_king_white - bonus_attack_king_black) / 100.0 << setw(15) <<
            (double) (bonus_attack_king_white) / 100.0 << setw(10) << (double) (bonus_attack_king_black) / 100.0
            << "\n";

        cout << HEADER;
        cout << "|Pawn:             " << setw(10) << (double) (Tresult.pawns[WHITE] - Tresult.pawns[BLACK]) / 100.0 <<
            setw(15) << (double) (Tresult.pawns[WHITE]) / 100.0 << setw(10) << (double) (Tresult.pawns[BLACK]) / 100.0
            <<
                "\n";
        cout << "|       attack king:              " << setw(10) <<
            (double) (SCORE_DEBUG.ATTACK_KING_PAWN[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.ATTACK_KING_PAWN[BLACK]) / 100.0 << "\n";
        cout << "|       center:                   " << setw(10) << (double) (SCORE_DEBUG.PAWN_CENTER[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.PAWN_CENTER[BLACK]) / 100.0 << "\n";
        cout << "|       in 7th:                   " << setw(10) << (double) (SCORE_DEBUG.PAWN_7H[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.PAWN_7H[BLACK]) / 100.0 << "\n";
        cout << "|       in 8th:                   " << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_IN_8TH[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.PAWN_IN_8TH[BLACK]) / 100.0 << "\n";
        cout << "|       blocked:                  " << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_BLOCKED[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.PAWN_BLOCKED[BLACK]) / 100.0 << "\n";
        cout << "|       unprotected:              " << setw(10) <<
            (double) (SCORE_DEBUG.UNPROTECTED_PAWNS[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.UNPROTECTED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "|       isolated                  " << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_ISOLATED[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_ISOLATED[BLACK]) / 100.0 << "\n";
        cout << "|       double                    " << setw(10) <<
            (double) (SCORE_DEBUG.DOUBLED_PAWNS[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.DOUBLED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "|       double isolated           " << setw(10) <<
            (double) (SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.DOUBLED_ISOLATED_PAWNS[BLACK]) / 100.0 << "\n";
        cout << "|       backward                  " << setw(10) <<
            (double) (SCORE_DEBUG.BACKWARD_PAWN[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.BACKWARD_PAWN[BLACK]) / 100.0 << "\n";
        cout << "|       fork:                     " << setw(10) << (double) (SCORE_DEBUG.FORK_SCORE[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.FORK_SCORE[BLACK]) / 100.0 << "\n";
        cout << "|       passed:                   " << setw(10) << (double) (SCORE_DEBUG.PAWN_PASSED[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.PAWN_PASSED[BLACK]) / 100.0 << "\n";
        cout << "|       all enemies:              " << setw(10) <<
            (double) (SCORE_DEBUG.ENEMIES_PAWNS_ALL[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.ENEMIES_PAWNS_ALL[BLACK]) / 100.0 << "\n";
        cout << "|       none:                     " << setw(10) << (double) (SCORE_DEBUG.NO_PAWNS[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.NO_PAWNS[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "|Knight:           " << setw(10) <<
            (double) (Tresult.knights[WHITE] - Tresult.knights[BLACK]) / 100.0 << setw(15) <<
            (double) (Tresult.knights[WHITE]) / 100.0 << setw(10) << (double) (Tresult.knights[BLACK]) / 100.0 << "\n";
        cout << "|       undevelop:                " << setw(10) <<
            (double) (SCORE_DEBUG.UNDEVELOPED_KNIGHT[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.UNDEVELOPED_KNIGHT[BLACK]) / 100.0 << "\n";
        cout << "|       trapped:                  " << setw(10) <<
            (double) (SCORE_DEBUG.KNIGHT_TRAPPED[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.KNIGHT_TRAPPED[BLACK]) / 100.0 << "\n";
        cout << "|       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_KNIGHT[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.MOB_KNIGHT[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "|Bishop:           " << setw(10) << (double) (Tresult.bishop[WHITE] - Tresult.bishop[BLACK]) / 100.0 <<
            setw(15) << (double) (Tresult.bishop[WHITE]) / 100.0 << setw(10)
            << (double) (Tresult.bishop[BLACK]) / 100.0 <<
            "\n";
        cout << "|       bad:                      " << setw(10) << (double) (SCORE_DEBUG.BAD_BISHOP[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.BAD_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "|       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_BISHOP[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.MOB_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "|       undevelop:                " << setw(10) <<
            (double) (SCORE_DEBUG.UNDEVELOPED_BISHOP[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.UNDEVELOPED_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "|       open diag:                " << setw(10) <<
            (double) (SCORE_DEBUG.OPEN_DIAG_BISHOP[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.OPEN_DIAG_BISHOP[BLACK]) / 100.0 << "\n";
        cout << "|       bonus 2 bishops:          " << setw(10) <<
            (double) (SCORE_DEBUG.BONUS2BISHOP[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.BONUS2BISHOP[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "|Rook:             " << setw(10) << (double) (Tresult.rooks[WHITE] - Tresult.rooks[BLACK]) / 100.0 <<
            setw(15) << (double) (Tresult.rooks[WHITE]) / 100.0 << setw(10) << (double) (Tresult.rooks[BLACK]) / 100.0
            <<
                "\n";
        cout << "|       7th:                      " << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_7TH_RANK[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_7TH_RANK[BLACK]) / 100.0 << "\n";
        cout << "|       trapped:                  " << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_TRAPPED[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.ROOK_TRAPPED[BLACK]) / 100.0 << "\n";
        cout << "|       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_ROOK[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.MOB_ROOK[BLACK]) / 100.0 << "\n";
        cout << "|       blocked:                  " << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_BLOCKED[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.ROOK_BLOCKED[BLACK]) / 100.0 << "\n";
        cout << "|       open file:                " << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_OPEN_FILE[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.ROOK_OPEN_FILE[BLACK]) / 100.0 << "\n";
        cout << "|       connected:                " << setw(10) <<
            (double) (SCORE_DEBUG.CONNECTED_ROOKS[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.CONNECTED_ROOKS[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "|Queen:            " << setw(10) << (double) (Tresult.queens[WHITE] - Tresult.queens[BLACK]) / 100.0 <<
            setw(15) << (double) (Tresult.queens[WHITE]) / 100.0 << setw(10)
            << (double) (Tresult.queens[BLACK]) / 100.0 <<
            "\n";
        cout << "|       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_QUEEN[WHITE]) / 100.0 <<
            setw(10) << (double) (SCORE_DEBUG.MOB_QUEEN[BLACK]) / 100.0 << "\n";
        cout << "|       bishop on queen:          " << setw(10) <<
            (double) (SCORE_DEBUG.BISHOP_ON_QUEEN[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.BISHOP_ON_QUEEN[BLACK]) / 100.0 << "\n";

        cout << HEADER;
        cout << "|King:             " << setw(10) << (double) (Tresult.kings[WHITE] - Tresult.kings[BLACK]) / 100.0 <<
            setw(15) << (double) (Tresult.kings[WHITE]) / 100.0 << setw(10) << (double) (Tresult.kings[BLACK]) / 100.0
            <<
                "\n";
        cout << "|       distance:                 " << setw(10) <<
            (double) (SCORE_DEBUG.DISTANCE_KING[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.DISTANCE_KING[BLACK]) / 100.0 << "\n";
        cout << "|       open file:                " << setw(10) <<
            (double) (SCORE_DEBUG.END_OPENING_KING[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.END_OPENING_KING[BLACK]) / 100.0 << "\n";
        cout << "|       pawn near:                " << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_NEAR_KING[WHITE]) / 100.0 << setw(10) <<
            (double) (SCORE_DEBUG.PAWN_NEAR_KING[BLACK]) / 100.0 << "\n";
//      cout << "|       mobility:                 " << setw(10) << (double) (SCORE_DEBUG.MOB_KING[WHITE]) / 100.0 << setw(10) << (double) (SCORE_DEBUG.MOB_KING[BLACK]) / 100.0 << "\n";
        cout << endl;
        cout << "\n|Total (white)..........   " << (double) (side ? result / 100.0 : -result / 100.0) << endl;
        fflush(stdout);
    }
#endif
    storeHashValue(key, result);
    BENCH(evalTime.stop());
    return side ? -result : result;
}

