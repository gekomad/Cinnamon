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
    memset(&structureEval, 0, sizeof(_Tboard));
    if (evalHash == nullptr)
        evalHash = (u64 *) calloc(hashSize, sizeof(u64));
}

Eval::~Eval() {
    free(evalHash);
    evalHash = nullptr;
}

template<uchar side>
pair<int, int> Eval::evaluatePawn(const _Tchessboard &chessboard) {
    INC(evaluationCount[side]);
    int result[2] = {0, 0};
    constexpr int xside = X(side);

    const u64 ped_friends = chessboard[side];

    // 5. space
//    if (phase == OPEN) {
//        result += PAWN_CENTER * bitCount(ped_friends & CENTER_MASK);
//        ADD(SCORE_DEBUG.PAWN_CENTER[side], PAWN_CENTER * bitCount(ped_friends & CENTER_MASK));
//    }

    // 7.

//        if (structureEval.pinned[side] & ped_friends) result -= PAWN_PINNED;
//        ADD(SCORE_DEBUG.PAWN_PINNED[side], -PAWN_PINNED);
//    structureEval.kingSecurity[MG][side] +=
    //           FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & ped_friends);
    //   structureEval.kingSecurity[EG][side] +=
    //           FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & ped_friends);
//
//        structureEval.kingSecurity[side] -=
//                ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & ped_friends);


    // 8.  pawn in 8th

    const u64 pawnsIn7 = PAWNS_7_2[side] & ped_friends;
    const auto count7 = bitCount(pawnsIn7);
    result[MG] += PAWN_IN_7TH[MG] * count7;
    result[EG] += PAWN_IN_7TH[EG] * count7;
    ADD(SCORE_DEBUG[MG].PAWN_7H[side], PAWN_IN_7TH[MG] * count7);
    ADD(SCORE_DEBUG[EG].PAWN_7H[side], PAWN_IN_7TH[EG] * count7);

    const u64 pawnsIn8 = (shiftForward<side, 8>(pawnsIn7) & (~structureEval.allPieces)) |
                         (structureEval.allPiecesSide[xside] &
                          (shiftForward<side, 7>(pawnsIn7) | shiftForward<side, 9>(pawnsIn7)));
    const auto count8 = bitCount(pawnsIn8);
    result[MG] += PAWN_IN_PROMOTION[MG] * count8; //try to decrease PAWN_IN_PROMOTION
    result[EG] += PAWN_IN_PROMOTION[EG] * count8; //try to decrease PAWN_IN_PROMOTION
    ADD(SCORE_DEBUG[MG].PAWN_IN_PROMOTION[side], PAWN_IN_PROMOTION[MG] * (bitCount(pawnsIn8)));
    ADD(SCORE_DEBUG[EG].PAWN_IN_PROMOTION[side], PAWN_IN_PROMOTION[EG] * (bitCount(pawnsIn8)));


    for (u64 p = ped_friends; p; RESET_LSB(p)) {
        bool isolated = false;
        const int o = BITScanForward(p);
        const u64 pos = POW2(o);

        // 4. attack king
//        if (structureEval.posKingBit[xside] & PAWN_FORK_MASK[side][o]) {
//            structureEval.kingAttackers[xside] |= pos;
//            result[EG] += ATTACK_KING[EG];
//            result[MG] += ATTACK_KING[MG];
//        }

        /// blocked
        result[EG] -= (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
                      (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED[EG] : 0;
        result[MG] -= (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
                      (structureEval.allPieces & (shiftForward<side, 8>(pos))) ? PAWN_BLOCKED[MG] : 0;
        ADD(SCORE_DEBUG[MG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
                                                (structureEval.allPieces & (shiftForward<side, 8>(pos)))
                                                ? -PAWN_BLOCKED[MG] : 0);
        ADD(SCORE_DEBUG[EG].PAWN_BLOCKED[side], (!(PAWN_FORK_MASK[side][o] & structureEval.allPiecesSide[xside])) &&
                                                (structureEval.allPieces & (shiftForward<side, 8>(pos)))
                                                ? -PAWN_BLOCKED[EG] : 0);
        /// unprotected
//        if (!(ped_friends & PAWN_PROTECTED_MASK[side][o])) {
//            result[EG] -= UNPROTECTED_PAWNS[EG];
//            result[MG] -= UNPROTECTED_PAWNS[MG];
//            ADD(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS[MG]);
//            ADD(SCORE_DEBUG[EG].UNPROTECTED_PAWNS[side], -UNPROTECTED_PAWNS[EG]);
//        }
        /// isolated
        if (!(ped_friends & PAWN_ISOLATED_MASK[o])) {
//            result -= PAWN_ISOLATED;
//            ADD(SCORE_DEBUG.PAWN_ISOLATED[side], -PAWN_ISOLATED);
            isolated = true;
        }
        /// doubled
        if (NOTPOW2(o) & FILE_[o] & ped_friends) {
//            result -= DOUBLED_PAWNS;
//            ADD(SCORE_DEBUG.DOUBLED_PAWNS[side], -DOUBLED_PAWNS);
            /// doubled and isolated
            if (isolated) {
                ADD(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS[MG]);
                ADD(SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[side], -DOUBLED_ISOLATED_PAWNS[EG]);
                result[EG] -= DOUBLED_ISOLATED_PAWNS[EG];
                result[MG] -= DOUBLED_ISOLATED_PAWNS[MG];
            }
        }
        /// backward
        if (!(ped_friends & PAWN_BACKWARD_MASK[side][o])) {
            ADD(SCORE_DEBUG[MG].BACKWARD_PAWN[side], -BACKWARD_PAWN[MG]);
            ADD(SCORE_DEBUG[EG].BACKWARD_PAWN[side], -BACKWARD_PAWN[EG]);
            result[EG] -= BACKWARD_PAWN[EG];
            result[MG] -= BACKWARD_PAWN[MG];
        }
        /// passed
        if (!(chessboard[xside] & PAWN_PASSED_MASK[side][o])) {
            ADD(SCORE_DEBUG[MG].PAWN_PASSED[side], PAWN_PASSED[side][o]);
            ADD(SCORE_DEBUG[EG].PAWN_PASSED[side], PAWN_PASSED[side][o]);
            result[EG] += PAWN_PASSED[side][o];
            result[MG] += PAWN_PASSED[side][o];
        }
    }
    return pair<int, int>(result[MG], result[EG]);
}

template<uchar side>
pair<int, int> Eval::evaluateBishop(const _Tchessboard &chessboard, const u64 enemies) {
    INC(evaluationCount[side]);
    constexpr int xside = X(side);
    u64 bishop = chessboard[BISHOP_BLACK + side];

    // 1.
    if (!bishop) return pair<int, int>(0, 0);

    int result[2] = {0, 0};

    // 2.
    if (bitCount(bishop) == 1) {
        const auto x = bitCount(chessboard[side] & board::colors(BITScanForward(bishop)));
        result[MG] -= BISHOP_PAWN_ON_SAME_COLOR[MG] * x;
        result[EG] -= BISHOP_PAWN_ON_SAME_COLOR[EG] * x;
    } else {
        // 2.

        result[MG] += BONUS2BISHOP[MG];
        result[EG] += BONUS2BISHOP[EG];
        ADD(SCORE_DEBUG[MG].BONUS2BISHOP[side], BONUS2BISHOP[MG]);
        ADD(SCORE_DEBUG[EG].BONUS2BISHOP[side], BONUS2BISHOP[EG]);

    }

    // 3. *king security*

    // 9. pinned
    if (structureEval.pinned[side] & bishop) result[MG] -= BISHOP_PINNED[MG];
    if (structureEval.pinned[side] & bishop) result[EG] -= BISHOP_PINNED[EG];
    ADD(SCORE_DEBUG[MG].BISHOP_PINNED[side], -BISHOP_PINNED[MG]);
    ADD(SCORE_DEBUG[EG].BISHOP_PINNED[side], -BISHOP_PINNED[EG]);
//        structureEval.kingSecurity[side] -=
//                ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & bishop);
//        ADD(SCORE_DEBUG.KING_SECURITY_BISHOP[side],
//            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & bishop));


    // 4. undevelop
//    if (phase != END) {
//        result -= UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop);
//        ADD(SCORE_DEBUG.UNDEVELOPED_BISHOP[side], UNDEVELOPED_BISHOP * bitCount(BISHOP_HOME[side] & bishop));
//    }

    for (; bishop; RESET_LSB(bishop)) {
        const int o = BITScanForward(bishop);
        // 5. mobility

        const u64 x = Bitboard::getDiagonalAntiDiagonal(o, structureEval.allPieces);
        const u64 captured = x & enemies;
        assert(bitCount(captured) + bitCount(x & ~structureEval.allPieces) < (int) (sizeof(MOB_BISHOP) / sizeof(int)));
        if (captured & structureEval.posKingBit[xside]) structureEval.kingAttackers[xside] |= POW2(o);
        const auto x1 = bitCount(captured) + bitCount(x & ~structureEval.allPieces);
        result[MG] += MOB_BISHOP[MG][x1];
        result[EG] += MOB_BISHOP[EG][x1];

        ADD(SCORE_DEBUG[MG].MOB_BISHOP[side], MOB_BISHOP[MG][x1]);
        ADD(SCORE_DEBUG[EG].MOB_BISHOP[side], MOB_BISHOP[EG][x1]);

        // 6.
//        if (phase != OPEN) {
//            if ((BIG_DIAGONAL & structureEval.allPieces) == POW2(o)) {
//                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
//                result += OPEN_FILE;
//            }
//            if ((BIG_ANTIDIAGONAL & structureEval.allPieces) == POW2(o)) {
//                ADD(SCORE_DEBUG.OPEN_DIAG_BISHOP[side], OPEN_FILE);
//                result += OPEN_FILE;
//            }
//        }

        // 7. outposts
//           const auto p = BISHOP_OUTPOST[side][o];
//
        // //enemy pawn doesn't attack bishop
//           if (p && !(PAWN_FORK_MASK[xside][o] & chessboard[xside])) {
        // //    friend paws defends bishop
//               if (PAWN_FORK_MASK[X(side)][o] & chessboard[side]) {
//                   result[MG] += p;
//                   result[EG] += p;
//                   if (!(chessboard[KNIGHT_BLACK + xside]) && !(chessboard[BISHOP_BLACK + xside] & board::colors(o))) {
//                       result[MG] += p;
//                       result[EG] += p;
//                   }
//               }
//           }
    }
    return pair<int, int>(result[MG], result[EG]);
}

template<uchar side>
pair<int, int> Eval::evaluateQueen(const _Tchessboard &chessboard, const u64 enemies) {
    INC(evaluationCount[side]);
    u64 queen = chessboard[QUEEN_BLACK + side];
    int result[2] = {0, 0};
    constexpr int xside = X(side);
    // 2. *king security*

//    if (structureEval.pinned[side] & queen) result[MG] -= QUEEN_PINNED[MG];
//    if (structureEval.pinned[side] & queen) result[EG] -= QUEEN_PINNED[EG];
//    ADD(SCORE_DEBUG[MG].QUEEN_PINNED[side], -QUEEN_PINNED[MG]);
//    ADD(SCORE_DEBUG[EG].QUEEN_PINNED[side], -QUEEN_PINNED[EG]);
//    structureEval.kingSecurity[MG][side] +=
//            FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen);
//    structureEval.kingSecurity[EG][side] +=
//            FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen);
//    ADD(SCORE_DEBUG[MG].KING_SECURITY_QUEEN[side],
//        FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen));
//    ADD(SCORE_DEBUG[EG].KING_SECURITY_QUEEN[side],
//        FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & queen));

//        structureEval.kingSecurity[side] -=
//                ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & queen);
//        ADD(SCORE_DEBUG.KING_SECURITY_QUEEN[xside],
//            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & queen));


    for (; queen; RESET_LSB(queen)) {
        const int o = BITScanForward(queen);
        // 3. mobility
        const u64 x = board::performRankFileCaptureAndShift(o, enemies, structureEval.allPieces) |
                      board::getDiagShiftAndCapture(o, enemies, structureEval.allPieces);

        result[MG] += MOB_QUEEN[MG][bitCount(x)] * MOB_QUEEN_[MG] / 100;
        result[EG] += MOB_QUEEN[EG][bitCount(x)] * MOB_QUEEN_[EG] / 100;
        ADD(SCORE_DEBUG[MG].MOB_QUEEN[side], MOB_QUEEN[MG][bitCount(x)] * MOB_QUEEN_[MG] / 100);
        ADD(SCORE_DEBUG[EG].MOB_QUEEN[side], MOB_QUEEN[EG][bitCount(x)] * MOB_QUEEN_[EG] / 100);

        if (x & structureEval.posKingBit[xside]) structureEval.kingAttackers[xside] |= POW2(o);
        // 4. half open file
//        if ((chessboard[xside] & FILE_[o])) {
//            ADD(SCORE_DEBUG.HALF_OPEN_FILE_Q[side], HALF_OPEN_FILE_Q);
//            result += HALF_OPEN_FILE_Q;
//        }

        // 5. open file
        if ((FILE_[o] & structureEval.allPieces) == POW2(o)) {
            ADD(SCORE_DEBUG[MG].OPEN_FILE_Q[side], OPEN_FILE_Q[MG]);
            ADD(SCORE_DEBUG[EG].OPEN_FILE_Q[side], OPEN_FILE_Q[EG]);
            result[MG] += OPEN_FILE_Q[MG];
            result[EG] += OPEN_FILE_Q[EG];
        }

        // 6. bishop on queen
//         if (DIAGONAL_ANTIDIAGONAL[o] & chessboard[BISHOP_BLACK + side]) {
//             ADD(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN[MG]);
//             ADD(SCORE_DEBUG[EG].BISHOP_ON_QUEEN[side], BISHOP_ON_QUEEN[EG]);
//             result[MG] += BISHOP_ON_QUEEN[MG];
//             result[EG] += BISHOP_ON_QUEEN[EG];
//         }
    }
    return pair<int, int>(result[MG], result[EG]);
}

template<uchar side>
pair<int, int> Eval::evaluateKnight(const _Tchessboard &chessboard, const u64 notMyBits) {
    INC(evaluationCount[side]);
    u64 knight = chessboard[KNIGHT_BLACK + side];
    if (!knight) return pair<int, int>(0, 0);
    constexpr int xside = X(side);
    // 1. pinned
    int result[2] = {0, 0};

    // 2. undevelop
//    if (phase == OPEN) {
//        result -= bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT;
//        ADD(SCORE_DEBUG.UNDEVELOPED_KNIGHT[side],
//            bitCount(knight & KNIGHT_HOME[side]) * UNDEVELOPED_KNIGHT);
//    }

    // 4. king security

    if (structureEval.pinned[side] & knight) result[MG] -= KNIGHT_PINNED[MG];
    if (structureEval.pinned[side] & knight) result[EG] -= KNIGHT_PINNED[EG];
    ADD(SCORE_DEBUG[MG].KNIGHT_PINNED[side], -KNIGHT_PINNED[MG]);
    ADD(SCORE_DEBUG[EG].KNIGHT_PINNED[side], -KNIGHT_PINNED[EG]);
//    structureEval.kingSecurity[MG][side] +=
//            FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight);
//    structureEval.kingSecurity[EG][side] +=
//            FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight);
//    ADD(SCORE_DEBUG[MG].KING_SECURITY_KNIGHT[side],
//        FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight));
//    ADD(SCORE_DEBUG[EG].KING_SECURITY_KNIGHT[side],
//        FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & knight));

//        structureEval.kingSecurity[side] -=
//                ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & knight);
//        ADD(SCORE_DEBUG.KING_SECURITY_KNIGHT[xside],
//            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & knight));

    for (; knight; RESET_LSB(knight)) {
        const int pos = BITScanForward(knight);

        // 5. mobility
        assert(bitCount(notMyBits & KNIGHT_MASK[pos]) < (int) (sizeof(MOB_KNIGHT) / sizeof(int)));
        const u64 mob = notMyBits & KNIGHT_MASK[pos];
        result[MG] += MOB_KNIGHT[bitCount(mob)] * MOB_KNIGHT_[MG] / 100;
        result[EG] += MOB_KNIGHT[bitCount(mob)] * MOB_KNIGHT_[EG] / 100;
        if (mob & structureEval.posKingBit[xside]) structureEval.kingAttackers[xside] |= POW2(pos);
        ADD(SCORE_DEBUG[MG].MOB_KNIGHT[side], MOB_KNIGHT[bitCount(mob)] * MOB_KNIGHT_[MG] / 100);
        ADD(SCORE_DEBUG[EG].MOB_KNIGHT[side], MOB_KNIGHT[bitCount(mob)] * MOB_KNIGHT_[EG] / 100);

        // 6. outposts
//          auto p = KNIGHT_OUTPOST[side][pos];
//
//          //enemy pawn doesn't attack knight
//          if (p && !(PAWN_FORK_MASK[xside][pos] & chessboard[xside])) {
//              //friend paws defends knight
//              if (PAWN_FORK_MASK[xside][pos] & chessboard[side]) {
//                  result[MG] += p;
//                  result[EG] += p;
//                  if (!(chessboard[KNIGHT_BLACK + xside]) &&
//                      !(chessboard[BISHOP_BLACK + xside] & board::colors(pos))) {
//                      result[MG] += p;
//                      result[EG] += p;
//                  }
//              }
//          }
    }
    return pair<int, int>(result[MG], result[EG]);
}


template<uchar side>
pair<int, int> Eval::evaluateRook(const _Tchessboard &chessboard, const u64 enemies, const u64 friends) {
    INC(evaluationCount[side]);

    u64 rook = chessboard[ROOK_BLACK + side];
    if (!rook) return pair<int, int>(0, 0);

//    const int nRooks = bitCount(rook);
    // 2.
    int result[2] = {0, 0};
    constexpr int xside = X(side);
    // 3. in 7th
    const auto x = bitCount(rook & RANK_2_7[xside]);
    result[MG] += ROOK_7TH_RANK[MG] * x;
    result[EG] += ROOK_7TH_RANK[EG] * x;
    ADD(SCORE_DEBUG[MG].ROOK_7TH_RANK[side], ROOK_7TH_RANK[MG] * x);
    ADD(SCORE_DEBUG[EG].ROOK_7TH_RANK[side], ROOK_7TH_RANK[EG] * x);

    // 4. king security
    if (structureEval.pinned[side] & rook) {
        ADD(SCORE_DEBUG[MG].ROOK_PINNED[side], -ROOK_PINNED[MG]);
        result[MG] -= ROOK_PINNED[MG];
    }
    if (structureEval.pinned[side] & rook) {
        ADD(SCORE_DEBUG[EG].ROOK_PINNED[side], -ROOK_PINNED[EG]);
        result[EG] -= ROOK_PINNED[EG];
    }
    if (RANK_2_7[xside] & rook && POW2(structureEval.posKing[xside]) & RANK_1_8[xside]) {
        result[MG] += ROOK_IN_7_KING_IN_8[MG];
        result[EG] += ROOK_IN_7_KING_IN_8[EG];
        ADD(SCORE_DEBUG[MG].ROOK_IN_7_KING_IN_8[side], ROOK_IN_7_KING_IN_8[MG]);
        ADD(SCORE_DEBUG[EG].ROOK_IN_7_KING_IN_8[side], ROOK_IN_7_KING_IN_8[EG]);
    }

//     structureEval.kingSecurity[MG][side] +=
//             FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook);
//     structureEval.kingSecurity[EG][side] +=
//             FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook);
//     ADD(SCORE_DEBUG[MG].KING_SECURITY_ROOK[side],
//         FRIEND_NEAR_KING[MG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook));
//     ADD(SCORE_DEBUG[EG].KING_SECURITY_ROOK[side],
//         FRIEND_NEAR_KING[EG] * bitCount(NEAR_MASK2[structureEval.posKing[side]] & rook));

//        structureEval.kingSecurity[side] -=
//                ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook);
//        ADD(SCORE_DEBUG.KING_SECURITY_ROOK[xside],
//            -ENEMY_NEAR_KING * bitCount(NEAR_MASK2[structureEval.posKing[xside]] & rook));


    // .6
//    if (((F1G1bit[side] & king) && (H1H2G1bit[side] & rook)) || ((C1B1bit[side] & king) && (A1A2B1bit[side] & rook))) {
//        ADD(SCORE_DEBUG.ROOK_TRAPPED[side], -ROOK_TRAPPED);
//        result -= ROOK_TRAPPED;
//    }

    // .7
//    if (nRooks == 2) {
//        const int firstRook = BITScanForward(rook);
//        const int secondRook = BITScanReverse(rook);
//        if ((!(LINK_ROOKS[firstRook][secondRook] & structureEval.allPieces))) {
//            ADD(SCORE_DEBUG.CONNECTED_ROOKS[side], CONNECTED_ROOKS);
//            result += CONNECTED_ROOKS;
//        }
//    }

    for (; rook; RESET_LSB(rook)) {
        const int o = BITScanForward(rook);
        //mobility
        u64 mob = board::getMobilityRook(o, enemies, friends);
        if (mob & structureEval.posKingBit[xside]) structureEval.kingAttackers[xside] |= POW2(o);

        assert(bitCount(mob) < (int) (sizeof(MOB_ROOK[MG]) / sizeof(int)));
        const auto x = bitCount(mob);
        result[MG] += MOB_ROOK[MG][x];
        result[EG] += MOB_ROOK[EG][x];
        ADD(SCORE_DEBUG[MG].MOB_ROOK[side], MOB_ROOK[MG][x]);
        ADD(SCORE_DEBUG[EG].MOB_ROOK[side], MOB_ROOK[EG][x]);

//        if (phase != OPEN) {
//            // .8 Penalise if Rook is Blocked Horizontally
//            if ((RANK_BOUND[o] & structureEval.allPieces) == RANK_BOUND[o]) {
//                ADD(SCORE_DEBUG.ROOK_BLOCKED[side], -ROOK_BLOCKED);
//                result -= ROOK_BLOCKED;
//            }
//        }

        // .5
//        if (!(chessboard[side] & FILE_[o])) {
//            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
//            result += OPEN_FILE;
//        }
//        if (!(chessboard[xside] & FILE_[o])) {
//            ADD(SCORE_DEBUG.ROOK_OPEN_FILE[side], OPEN_FILE);
//            result += OPEN_FILE;
//        }
    }
    return pair<int, int>(result[MG], result[EG]);
}

pair<int, int> Eval::evaluateKing(const _Tchessboard &chessboard, const uchar side, const u64 squares) {
    assert(evaluationCount[side] == 5);
    int result[2] = {0, 0};
    uchar pos_king = structureEval.posKing[side];

    ADD(SCORE_DEBUG[EG].DISTANCE_KING[side], DISTANCE_KING_ENDING[pos_king]);
    ADD(SCORE_DEBUG[MG].DISTANCE_KING[side], DISTANCE_KING_OPENING[pos_king]);
    result[EG] = DISTANCE_KING_ENDING[pos_king];
    result[MG] = DISTANCE_KING_OPENING[pos_king];

    //mobility
    const auto x = bitCount(squares & NEAR_MASK1[pos_king]);
    assert(bitCount(x) < (int) (sizeof(MOB_KING[MG]) / sizeof(int)));
    result[MG] += MOB_KING[MG][x];
    result[EG] += MOB_KING[EG][x];
    ADD(SCORE_DEBUG[MG].MOB_KING[side], MOB_KING[MG][x]);
    ADD(SCORE_DEBUG[EG].MOB_KING[side], MOB_KING[EG][x]);

    assert(pos_king < 64);
    if (!(NEAR_MASK1[pos_king] & chessboard[side])) {
        ADD(SCORE_DEBUG[MG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING[MG]);
        ADD(SCORE_DEBUG[EG].PAWN_NEAR_KING[side], -PAWN_NEAR_KING[EG]);
        result[MG] -= PAWN_NEAR_KING[MG];
        result[EG] -= PAWN_NEAR_KING[EG];
    }
    // result[MG] += structureEval.kingSecurity[MG][side];
    //result[EG] += structureEval.kingSecurity[EG][side];
    return pair<int, int>(result[MG], result[EG]);
}

void Eval::storeHashValue(const u64 key, const short value) {
    evalHash[key % hashSize] = (key & keyMask) | (value & valueMask);
    assert(value == getHashValue(key));
}

short Eval::getHashValue(const u64 key) {
    const u64 kv = evalHash[key % hashSize];
    if ((kv & keyMask) == (key & keyMask))
        return (short) (kv & valueMask);

    return noHashValue;
}

short Eval::getScore(const _Tchessboard &chessboard, const u64 key, const uchar side, const int alpha, const int beta,
                     const bool trace) {
    BENCH_START("eval total")
#ifndef TUNING
    const short hashValue = getHashValue(key);
    if (hashValue != noHashValue && !trace) {
        auto a = side ? -hashValue : hashValue;
        BENCH_STOP("eval total")
        return a;
    }
#endif
    const int lazyscore_white = lazyEvalSide<WHITE>(chessboard);
    const int lazyscore_black = lazyEvalSide<BLACK>(chessboard);

#ifndef TUNING
    const int lazyscore = side ? lazyscore_white - lazyscore_black : lazyscore_black - lazyscore_white;
    if (lazyscore > (beta + FUTIL_MARGIN) || lazyscore < (alpha - FUTIL_MARGIN)) {
        INC(lazyEvalCuts);
        BENCH_STOP("eval total")
        return lazyscore;
    }
#endif
    DEBUG(evaluationCount[WHITE] = evaluationCount[BLACK] = 0)
    DEBUG(memset(&SCORE_DEBUG, 0, sizeof(_TSCORE_DEBUG)))

    const auto w = board::getBitmapNoPawnsNoKing<WHITE>(chessboard);
    const auto b = board::getBitmapNoPawnsNoKing<BLACK>(chessboard);

    //memset(&structureEval.kingSecurity, 0, sizeof(structureEval.kingSecurity));
    structureEval.allPiecesNoPawns[BLACK] = b | chessboard[KING_BLACK];
    structureEval.allPiecesNoPawns[WHITE] = w | chessboard[KING_WHITE];
    structureEval.allPiecesSide[BLACK] = structureEval.allPiecesNoPawns[BLACK] | chessboard[PAWN_BLACK];
    structureEval.allPiecesSide[WHITE] = structureEval.allPiecesNoPawns[WHITE] | chessboard[PAWN_WHITE];
    structureEval.allPieces = structureEval.allPiecesSide[BLACK] | structureEval.allPiecesSide[WHITE];
    structureEval.posKing[BLACK] = (uchar) BITScanForward(chessboard[KING_BLACK]);
    structureEval.posKing[WHITE] = (uchar) BITScanForward(chessboard[KING_WHITE]);
    structureEval.posKingBit[BLACK] = POW2(structureEval.posKing[BLACK]);
    structureEval.posKingBit[WHITE] = POW2(structureEval.posKing[WHITE]);
    structureEval.kingAttackers[WHITE] = structureEval.kingAttackers[BLACK] = 0;

    structureEval.pinned[WHITE] = board::getPinned<WHITE>(structureEval.allPieces,
                                                          structureEval.allPiecesSide[WHITE],
                                                          structureEval.posKing[WHITE], chessboard);
    structureEval.pinned[BLACK] = board::getPinned<BLACK>(structureEval.allPieces,
                                                          structureEval.allPiecesSide[BLACK],
                                                          structureEval.posKing[BLACK], chessboard);

    _Tresult tResult[2];

    getRes(chessboard, tResult);

    const int bonus_attack_king_black_eg = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[WHITE])];
    const int bonus_attack_king_white_eg = BONUS_ATTACK_KING[bitCount(structureEval.kingAttackers[BLACK])];

//    const int attack_king_white[2] = {ATTACK_KING[MG] * bitCount(structureEval.kingAttackers[WHITE]),
//                                      ATTACK_KING[EG] * bitCount(structureEval.kingAttackers[WHITE])};
//
//    const int attack_king_black[2] = {ATTACK_KING[MG] * bitCount(structureEval.kingAttackers[BLACK]),
//                                      ATTACK_KING[EG] * bitCount(structureEval.kingAttackers[BLACK])};
    const int firstMoveWhite = side == WHITE ? 5 : 0;
    const int firstMoveBlack = side == BLACK ? 5 : 0;

    const int result_mg =
            (lazyscore_black + tResult[MG].pawns[BLACK] +// attack_king_black[MG] +
             tResult[MG].knights[BLACK] + tResult[MG].bishop[BLACK] + tResult[MG].rooks[BLACK] +
             tResult[MG].queens[BLACK] + tResult[MG].kings[BLACK] + firstMoveBlack) -
            (lazyscore_white + tResult[MG].pawns[WHITE] + //attack_king_white[MG] +
             tResult[MG].knights[WHITE] + tResult[MG].bishop[WHITE] + tResult[MG].rooks[WHITE] +
             tResult[MG].queens[WHITE] + tResult[MG].kings[WHITE] + firstMoveWhite);
    const int result_eg =
            (bonus_attack_king_black_eg + lazyscore_black + tResult[EG].pawns[BLACK] + //attack_king_black[EG] +
             tResult[EG].knights[BLACK] + tResult[EG].bishop[BLACK] + tResult[EG].rooks[BLACK] +
             tResult[EG].queens[BLACK] + tResult[EG].kings[BLACK] + firstMoveBlack) -
            (bonus_attack_king_white_eg + lazyscore_white + tResult[EG].pawns[WHITE] + //attack_king_white[EG] +
             tResult[EG].knights[WHITE] + tResult[EG].bishop[WHITE] + tResult[EG].rooks[WHITE] +
             tResult[EG].queens[WHITE] + tResult[EG].kings[WHITE] + firstMoveWhite);
//    const int PawnPhase = 0;
//    const int KnightPhase = 1;
//    const int BishopPhase = 1;
//    const int RookPhase = 2;
//    const int QueenPhase = 4;
//    const int TotalPhase = PawnPhase * 16 + KnightPhase * 4 + BishopPhase * 4 + RookPhase * 4 + QueenPhase * 2;;
//
//    const int wp = bitCount(chessboard[PAWN_WHITE]);
//    const int bp = bitCount(chessboard[PAWN_BLACK]);
//
//    const int wn = bitCount(chessboard[KNIGHT_WHITE]);
//    const int bn = bitCount(chessboard[KNIGHT_BLACK]);
//
//    const int wb = bitCount(chessboard[BISHOP_WHITE]);
//    const int bb = bitCount(chessboard[BISHOP_BLACK]);
//
//    const int wq = bitCount(chessboard[QUEEN_WHITE]);
//    const int bq = bitCount(chessboard[QUEEN_BLACK]);
//
//    const int wr = bitCount(chessboard[ROOK_WHITE]);
//    const int br = bitCount(chessboard[ROOK_BLACK]);
//    int phase = TotalPhase;
//    phase -= wp * PawnPhase;
//    phase -= bp * PawnPhase;
//
//    phase -= wn * KnightPhase;
//    phase -= bn * KnightPhase;
//
//    phase -= wb * BishopPhase;
//    phase -= bb * BishopPhase;
//
//    phase -= wr * RookPhase;
//    phase -= br * RookPhase;
//
//    phase -= wq * QueenPhase;
//    phase -= bq * QueenPhase;

//    phase = (phase * 256 + (TotalPhase / 2)) / TotalPhase;


    int phase =
            ((MAX_VALUE_TAPERED - (lazyscore_white + lazyscore_black)) * 256 + (MAX_VALUE_TAPERED / 2)) /
            MAX_VALUE_TAPERED;

    const short finalScore = ((result_mg * (256 - phase)) + (result_eg * phase)) / 256;

#ifndef NDEBUG
    if (trace) {

        cout << "========================================================================================" << endl;
        cout << "phase 0 total opening, phase 256 total ending" << endl;
        cout << "current phase: " << phase << " result_mg: " << -result_mg << " result_eg: " << -result_eg << endl;
        cout << "white to move " << firstMoveWhite / 100.0 << endl;
        cout << "VALUES:";
        cout << "\tPAWN: " << (float) constants::VALUEPAWN / 100.0;
        cout << " ROOK: " << (float) constants::VALUEROOK / 100.0;
        cout << " BISHOP: " << (float) constants::VALUEBISHOP / 100.0;
        cout << " KNIGHT: " << (float) constants::VALUEKNIGHT / 100.0;
        cout << " QUEEN: " << (float) constants::VALUEQUEEN / 100.0 << endl << endl;

        cout << setprecision(2) << "Material:         " << setw(10)
             << (float) (lazyscore_white - lazyscore_black) / 100.0 << setw(15)
             << (float) (lazyscore_white) / 10000.0 << setw(10) << (float) (lazyscore_black) / 10000.0 << endl;

        cout << "Bonus attack king (EG):" << setw(5)
             << (float) (bonus_attack_king_white_eg - bonus_attack_king_black_eg) / 100.0 << setw(15)
             << (float) (bonus_attack_king_white_eg) / 100.0 << setw(10) << (float) (bonus_attack_king_black_eg) / 100.0
             << endl;
        cout << endl;
        cout << "  Eval term\t|\t   Total\t|\t   White\t|\t   Black\t|\n"
                "\t\t|  MG\t  EG\t  TOT\t|  MG\t    EG\t   TOT\t|  MG\t   EG\t TOT\t|\n"
                "----------------+-----------------------+-----------------------+-----------------------+" << endl;
        cout << "\tPAWN\t|";

        p2((tResult[MG].pawns[WHITE] - tResult[MG].pawns[BLACK]), (tResult[EG].pawns[WHITE] - tResult[EG].pawns[BLACK]),
           phase);

        p2(tResult[MG].pawns[WHITE], tResult[EG].pawns[WHITE], phase);
        p2(tResult[MG].pawns[BLACK], tResult[EG].pawns[BLACK], phase);

        cout << endl << "in 7th\t\t";
        p();
        p2(SCORE_DEBUG[MG].PAWN_7H[WHITE], SCORE_DEBUG[EG].PAWN_7H[WHITE], phase);
        p2(SCORE_DEBUG[MG].PAWN_7H[BLACK], SCORE_DEBUG[EG].PAWN_7H[BLACK], phase);
        cout << endl << "in promotion\t";
        p();
        p2(SCORE_DEBUG[MG].PAWN_IN_PROMOTION[WHITE], SCORE_DEBUG[EG].PAWN_IN_PROMOTION[WHITE], phase);
        p2(SCORE_DEBUG[MG].PAWN_IN_PROMOTION[BLACK], SCORE_DEBUG[EG].PAWN_IN_PROMOTION[BLACK], phase);
        cout << endl << "blocked\t\t";
        p();
        p2(SCORE_DEBUG[MG].PAWN_BLOCKED[WHITE], SCORE_DEBUG[EG].PAWN_BLOCKED[WHITE], phase);
        p2(SCORE_DEBUG[MG].PAWN_BLOCKED[BLACK], SCORE_DEBUG[EG].PAWN_BLOCKED[BLACK], phase);

        cout << endl << "unprotected\t";
        p();
        p2(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[WHITE], SCORE_DEBUG[EG].UNPROTECTED_PAWNS[WHITE], phase);
        p2(SCORE_DEBUG[MG].UNPROTECTED_PAWNS[BLACK], SCORE_DEBUG[EG].UNPROTECTED_PAWNS[BLACK], phase);

        cout << endl << "double isolated\t";
        p();
        p2(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[WHITE], SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[WHITE], phase);
        p2(SCORE_DEBUG[MG].DOUBLED_ISOLATED_PAWNS[BLACK], SCORE_DEBUG[EG].DOUBLED_ISOLATED_PAWNS[BLACK], phase);

        cout << endl << "backward\t";
        p();
        p2(SCORE_DEBUG[MG].BACKWARD_PAWN[WHITE], SCORE_DEBUG[EG].BACKWARD_PAWN[WHITE], phase);
        p2(SCORE_DEBUG[MG].BACKWARD_PAWN[BLACK], SCORE_DEBUG[EG].BACKWARD_PAWN[BLACK], phase);

        cout << endl << "passed\t\t";
        p();
        p2(SCORE_DEBUG[MG].PAWN_PASSED[WHITE], SCORE_DEBUG[EG].PAWN_PASSED[WHITE], phase);
        p2(SCORE_DEBUG[MG].PAWN_PASSED[BLACK], SCORE_DEBUG[EG].PAWN_PASSED[BLACK], phase);

        cout << endl << "\tKNIGHT\t|";
        p2((tResult[MG].knights[WHITE] - tResult[MG].knights[BLACK]),
           (tResult[EG].knights[WHITE] - tResult[EG].knights[BLACK]), phase);

        p2(tResult[MG].knights[WHITE], tResult[EG].knights[WHITE], phase);
        p2(tResult[MG].knights[BLACK], tResult[EG].knights[BLACK], phase);

        cout << endl << "mobility\t";
        p();
        p2(SCORE_DEBUG[MG].MOB_KNIGHT[WHITE], SCORE_DEBUG[EG].MOB_KNIGHT[WHITE], phase);
        p2(SCORE_DEBUG[MG].MOB_KNIGHT[BLACK], SCORE_DEBUG[EG].MOB_KNIGHT[BLACK], phase);
        cout << endl << "pinned\t\t";
        p();
        p2(SCORE_DEBUG[MG].KNIGHT_PINNED[WHITE], SCORE_DEBUG[EG].KNIGHT_PINNED[WHITE], phase);
        p2(SCORE_DEBUG[MG].KNIGHT_PINNED[BLACK], SCORE_DEBUG[EG].KNIGHT_PINNED[BLACK], phase);
        cout << endl << "\tBISHOP\t|";
        p2((tResult[MG].bishop[WHITE] - tResult[MG].bishop[BLACK]),
           (tResult[EG].bishop[WHITE] - tResult[EG].bishop[BLACK]), phase);

        p2(tResult[MG].bishop[WHITE], tResult[EG].bishop[WHITE], phase);
        p2(tResult[MG].bishop[BLACK], tResult[EG].bishop[BLACK], phase);

        cout << endl << "mobility\t";
        p();
        p2(SCORE_DEBUG[MG].MOB_BISHOP[WHITE], SCORE_DEBUG[EG].MOB_BISHOP[WHITE], phase);
        p2(SCORE_DEBUG[MG].MOB_BISHOP[BLACK], SCORE_DEBUG[EG].MOB_BISHOP[BLACK], phase);

        cout << endl << "bonus 2 bishops\t";
        p();
        p2(SCORE_DEBUG[MG].BONUS2BISHOP[WHITE], SCORE_DEBUG[EG].BONUS2BISHOP[WHITE], phase);
        p2(SCORE_DEBUG[MG].BONUS2BISHOP[BLACK], SCORE_DEBUG[EG].BONUS2BISHOP[BLACK], phase);
        cout << endl << "pinned\t\t";
        p();
        p2(SCORE_DEBUG[MG].BISHOP_PINNED[WHITE], SCORE_DEBUG[EG].BISHOP_PINNED[WHITE], phase);
        p2(SCORE_DEBUG[MG].BISHOP_PINNED[BLACK], SCORE_DEBUG[EG].BISHOP_PINNED[BLACK], phase);
        cout << endl << "\tROOK\t|";
        p2((tResult[MG].rooks[WHITE] - tResult[MG].rooks[BLACK]), (tResult[EG].rooks[WHITE] - tResult[EG].rooks[BLACK]),
           phase);
        p2(tResult[MG].rooks[WHITE], tResult[EG].rooks[WHITE], phase);
        p2(tResult[MG].rooks[BLACK], tResult[EG].rooks[BLACK], phase);

        cout << endl << "7th\t\t";
        p();
        p2(SCORE_DEBUG[MG].ROOK_7TH_RANK[WHITE], SCORE_DEBUG[EG].ROOK_7TH_RANK[WHITE], phase);
        p2(SCORE_DEBUG[MG].ROOK_7TH_RANK[BLACK], SCORE_DEBUG[EG].ROOK_7TH_RANK[BLACK], phase);

        cout << endl << "7th king in 8th\t";
        p();
        p2(SCORE_DEBUG[MG].ROOK_IN_7_KING_IN_8[WHITE], SCORE_DEBUG[EG].ROOK_IN_7_KING_IN_8[WHITE], phase);
        p2(SCORE_DEBUG[MG].ROOK_IN_7_KING_IN_8[BLACK], SCORE_DEBUG[EG].ROOK_IN_7_KING_IN_8[BLACK], phase);

        cout << endl << "mobility\t";
        p();
        p2(SCORE_DEBUG[MG].MOB_ROOK[WHITE], SCORE_DEBUG[EG].MOB_ROOK[WHITE], phase);
        p2(SCORE_DEBUG[MG].MOB_ROOK[BLACK], SCORE_DEBUG[EG].MOB_ROOK[BLACK], phase);
        cout << endl << "pinned\t\t";
        p();
        p2(SCORE_DEBUG[MG].ROOK_PINNED[WHITE], SCORE_DEBUG[EG].ROOK_PINNED[WHITE], phase);
        p2(SCORE_DEBUG[MG].ROOK_PINNED[BLACK], SCORE_DEBUG[EG].ROOK_PINNED[BLACK], phase);
        cout << endl << "\tQUEEN\t|";
        p2((tResult[MG].queens[WHITE] - tResult[MG].queens[BLACK]),
           (tResult[EG].queens[WHITE] - tResult[EG].queens[BLACK]), phase);
        p2(tResult[MG].queens[WHITE], tResult[EG].queens[WHITE], phase);
        p2(tResult[MG].queens[BLACK], tResult[EG].queens[BLACK], phase);

        cout << endl << "mobility\t";
        p();
        p2(SCORE_DEBUG[MG].MOB_QUEEN[WHITE], SCORE_DEBUG[EG].MOB_QUEEN[WHITE], phase);
        p2(SCORE_DEBUG[MG].MOB_QUEEN[BLACK], SCORE_DEBUG[EG].MOB_QUEEN[BLACK], phase);
        cout << endl << "open file\t";
        p();
        p2(SCORE_DEBUG[MG].OPEN_FILE_Q[WHITE], SCORE_DEBUG[EG].OPEN_FILE_Q[WHITE], phase);
        p2(SCORE_DEBUG[MG].OPEN_FILE_Q[BLACK], SCORE_DEBUG[EG].OPEN_FILE_Q[BLACK], phase);

//        cout << endl << "bishop on queen\t";
//        p();
//        p2(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[WHITE], SCORE_DEBUG[EG].BISHOP_ON_QUEEN[WHITE],phase);
//        p2(SCORE_DEBUG[MG].BISHOP_ON_QUEEN[BLACK], SCORE_DEBUG[EG].BISHOP_ON_QUEEN[BLACK],phase);
//        cout << endl << "pinned\t\t";
//        p();
//        p2(SCORE_DEBUG[MG].QUEEN_PINNED[WHITE], SCORE_DEBUG[EG].QUEEN_PINNED[WHITE],phase);
//        p2(SCORE_DEBUG[MG].QUEEN_PINNED[BLACK], SCORE_DEBUG[EG].QUEEN_PINNED[BLACK],phase);
        cout << endl << "\tKING\t|";
        p2((tResult[MG].kings[WHITE] - tResult[MG].kings[BLACK]), (tResult[EG].kings[WHITE] - tResult[EG].kings[BLACK]),
           phase);
        p2(tResult[MG].kings[WHITE], tResult[EG].kings[WHITE], phase);
        p2(tResult[MG].kings[BLACK], tResult[EG].kings[BLACK], phase);
        cout << endl << "distance\t";
        p();
        p2(SCORE_DEBUG[MG].DISTANCE_KING[WHITE], SCORE_DEBUG[EG].DISTANCE_KING[WHITE], phase);
        p2(SCORE_DEBUG[MG].DISTANCE_KING[BLACK], SCORE_DEBUG[EG].DISTANCE_KING[BLACK], phase);

        cout << endl << "pawn near\t";
        p();
        p2(SCORE_DEBUG[MG].PAWN_NEAR_KING[WHITE], SCORE_DEBUG[EG].PAWN_NEAR_KING[WHITE], phase);
        p2(SCORE_DEBUG[MG].PAWN_NEAR_KING[BLACK], SCORE_DEBUG[EG].PAWN_NEAR_KING[BLACK], phase);

        cout << endl << "mobility\t";
        p();
        p2(SCORE_DEBUG[MG].MOB_KING[WHITE], SCORE_DEBUG[EG].MOB_KING[WHITE], phase);
        p2(SCORE_DEBUG[MG].MOB_KING[BLACK], SCORE_DEBUG[EG].MOB_KING[BLACK], phase);

    }
#endif
#ifndef TUNING
    storeHashValue(key, finalScore);
#endif
    BENCH_STOP("eval total")
    return side ? -finalScore : finalScore;

}

