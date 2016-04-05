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

#include "GenMoves.h"
#include "Eval.h"
#include "util/Bitboard.h"

bool GenMoves::forceCheck = false;

GenMoves::GenMoves() : perftMode(false), listId(-1) {
    currentPly = 0;
    gen_list = (_TmoveP *) calloc(MAX_PLY, sizeof(_TmoveP));
    _assert(gen_list);
    for (int i = 0; i < MAX_PLY; i++) {
        gen_list[i].moveList = (_Tmove *) calloc(MAX_MOVE, sizeof(_Tmove));
        _assert(gen_list[i].moveList);
    }
    repetitionMap = (u64 *) malloc(sizeof(u64) * MAX_REP_COUNT);
    _assert(repetitionMap);
    repetitionMapCount = 0;
}

bool GenMoves::performRankFileCapture(const int piece, const u64 enemies, const int side, const u64 allpieces) {
    ASSERT_RANGE(piece, 0, 11);
    ASSERT_RANGE(side, 0, 1);
    u64 x2 = chessboard[piece];
    while (x2) {
        int position = BITScanForward(x2);
        u64 rankFile = getRankFile(position, allpieces) & enemies;;
        while (rankFile) {
            if (pushmove<STANDARD_MOVE_MASK>(position, BITScanForward(rankFile), side, NO_PROMOTION, piece)) {
                return true;
            }
            RESET_LSB(rankFile);
        }
        RESET_LSB(x2);
    }
    return false;
}

int GenMoves::performRankFileCaptureAndShiftCount(const int position, const u64 enemies, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63);
    u64 rankFile = getRankFile(position, allpieces);
    rankFile = (rankFile & enemies) | (rankFile & ~allpieces);
    return bitCount(rankFile);
}

bool GenMoves::performDiagCapture(const int piece, const u64 enemies, const int side, const u64 allpieces) {
    ASSERT_RANGE(piece, 0, 11);
    ASSERT_RANGE(side, 0, 1);
    u64 x2 = chessboard[piece];
    while (x2) {
        int position = BITScanForward(x2);
        u64 diag = getDiagonalAntiDiagonal(position, allpieces) & enemies;;
        while (diag) {
            if (pushmove<STANDARD_MOVE_MASK>(position, BITScanForward(diag), side, NO_PROMOTION, piece)) {
                return true;
            }
            RESET_LSB(diag);
        }
        RESET_LSB(x2);
    }
    return false;
}

void GenMoves::performRankFileShift(const int piece, const int side, const u64 allpieces) {
    ASSERT_RANGE(piece, 0, 11);
    ASSERT_RANGE(side, 0, 1);
    u64 x2 = chessboard[piece];
    while (x2) {
        int position = BITScanForward(x2);
        u64 rankFile = getRankFile(position, allpieces) & ~allpieces;
        while (rankFile) {
            pushmove<STANDARD_MOVE_MASK>(position, BITScanForward(rankFile), side, NO_PROMOTION, piece);
            RESET_LSB(rankFile);
        }
        RESET_LSB(x2);
    }
}

void GenMoves::performDiagShift(const int piece, const int side, const u64 allpieces) {
    ASSERT_RANGE(piece, 0, 11);
    ASSERT_RANGE(side, 0, 1);
    u64 x2 = chessboard[piece];
    while (x2) {
        int position = BITScanForward(x2);
        u64 diag = getDiagonalAntiDiagonal(position, allpieces) & ~allpieces;
        while (diag) {
            pushmove<STANDARD_MOVE_MASK>(position, BITScanForward(diag), side, NO_PROMOTION, piece);
            RESET_LSB(diag);
        }
        RESET_LSB(x2);
    }
}

void GenMoves::generateMoves(const int side, const u64 allpieces) {
    ASSERT_RANGE(side, 0, 1);
    side ? generateMoves<WHITE>(allpieces) : generateMoves<BLACK>(allpieces);
}

bool GenMoves::generateCaptures(const int side, const u64 enemies, const u64 friends) {
    ASSERT_RANGE(side, 0, 1);
    return side ? generateCaptures<WHITE>(enemies, friends) : generateCaptures<BLACK>(enemies, friends);
}

int GenMoves::getMobilityPawns(const int side, const int ep, const u64 ped_friends, const u64 enemies, const u64 xallpieces) {
    ASSERT_RANGE(side, 0, 1);
    return ep == NO_ENPASSANT ? 0 : bitCount(ENPASSANT_MASK[side ^ 1][ep] & chessboard[side]) + side == WHITE ? bitCount((ped_friends << 8) & xallpieces) + bitCount(((((ped_friends & TABJUMPPAWN) << 8) & xallpieces) << 8) & xallpieces) + bitCount((chessboard[side] << 7) & TABCAPTUREPAWN_LEFT & enemies) + bitCount((chessboard[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies) : bitCount((ped_friends >> 8) & xallpieces) + bitCount(((((ped_friends & TABJUMPPAWN) >> 8) & xallpieces) >> 8) & xallpieces) + bitCount((chessboard[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies) + bitCount((chessboard[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies);
}

int GenMoves::getMobilityQueen(const int position, const u64 enemies, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63);
    return performRankFileCaptureAndShiftCount(position, enemies, allpieces) +
           bitCount(getDiagShiftAndCapture(position, enemies, allpieces));
}

int GenMoves::getMobilityRook(const int position, const u64 enemies, const u64 friends) {
    ASSERT_RANGE(position, 0, 63);
    return performRankFileCaptureAndShiftCount(position, enemies, enemies | friends);
}

void GenMoves::setPerft(const bool b) {
    perftMode = b;
}

void GenMoves::clearKillerHeuristic() {
    memset(killerHeuristic, 0, sizeof(killerHeuristic));
}

_Tmove *GenMoves::getNextMove(_TmoveP *list) {
    _Tmove *gen_list1 = list->moveList;
    ASSERT(gen_list1);
    int listcount = list->size;
    int bestId = -1;
    int j, bestScore;
    for (j = 0; j < listcount; j++) {
        if (!gen_list1[j].used) {
            bestId = j;
            bestScore = gen_list1[bestId].score;
            break;
        }
    }
    if (bestId == -1) {
        return nullptr;
    }
    for (int i = j + 1; i < listcount; i++) {
        if (!gen_list1[i].used && gen_list1[i].score > bestScore) {
            bestId = i;
            bestScore = gen_list1[bestId].score;
        }
    }
    gen_list1[bestId].used = true;
    return &gen_list1[bestId];
}

GenMoves::~GenMoves() {
    for (int i = 0; i < MAX_PLY; i++) {
        free(gen_list[i].moveList);
    }
    free(gen_list);
    free(repetitionMap);
}

//bool GenMoves::isPinned(const int side, const uchar position, const uchar piece) {
//    u64 king = chessboard[KING_BLACK + side];
//    int posKing = BITScanForward(king);
//    u64 pow2position = POW2[position];
//    if(!(LEFT_RIGHT_RANK_FILE[posKing] & pow2position)) {
//        return false;
//    }
//    int xside = side ^ 1;
//    chessboard[piece] &= NOTPOW2[position];
//    u64 allpieces = getBitmap<WHITE>() | getBitmap<BLACK>();
//    u64 qr = chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside];
//    u64 qb = chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside];
//    if(king & RANK[position] && RANK[position] & qr) {
//        //rank
//        for(int n = position + 1; n <= ORIZ_LEFT[position]; n++) {
//            if(qr & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//        for(int n = position - 1; n >= ORIZ_RIGHT[position]; n--) {
//            if(qr & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//    } else if(king & FILE_[position] && FILE_[position] & qr) {
//        for(int n = posKing + 8; n <= VERT_UPPER[posKing]; n += 8) {
//            if(qr & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(POW2[n]&allpieces) {
//                break;
//            }
//        }
//        for(int n = posKing - 8; n >= VERT_LOWER[posKing]; n -= 8) {
//            if(qr & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(POW2[n]&allpieces) {
//                break;
//            }
//        }
//    } else if(king & LEFT_DIAG[position] && LEFT_DIAG[position] & qb) {
//        for(int n = position + 7; n <= LEFT_UPPER[position]; n += 7) {
//            if(qb & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//        for(int n = position - 7; n >= LEFT_LOWER[position]; n -= 7) {
//            if(qb & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//    } else if(king & RIGHT_DIAG[position] && RIGHT_DIAG[position] & qb) {
//        for(int n = position + 9; n <= RIGHT_UPPER[position]; n += 9) {
//            if(qb & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//        for(int n = position - 9; n >= RIGHT_LOWER[position]; n -= 9) {
//            if(qb & POW2[n]) {
//                chessboard[piece] |= pow2position;
//                return true;
//            }
//            if(allpieces & POW2[n]) {
//                break;
//            }
//        }
//    }
//    chessboard[piece] |= pow2position;
//    return false;
//}

void GenMoves::performCastle(const int side, const uchar type) {
    ASSERT_RANGE(side, 0, 1);
    if (side == WHITE) {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            ASSERT(getPieceAt(side, POW2_3) == KING_WHITE);
            ASSERT(getPieceAt(side, POW2_1) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_2) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_0) == ROOK_WHITE);
            updateZobristKey(KING_WHITE, 3);
            updateZobristKey(KING_WHITE, 1);
            chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_1) & NOTPOW2_3;
            updateZobristKey(ROOK_WHITE, 2);
            updateZobristKey(ROOK_WHITE, 0);
            chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_2) & NOTPOW2_0;
        } else {
            ASSERT(type & QUEEN_SIDE_CASTLE_MOVE_MASK);
            ASSERT(getPieceAt(side, POW2_3) == KING_WHITE);
            ASSERT(getPieceAt(side, POW2_4) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_5) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_6) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_7) == ROOK_WHITE);
            chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_5) & NOTPOW2_3;
            updateZobristKey(KING_WHITE, 5);
            updateZobristKey(KING_WHITE, 3);
            chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_4) & NOTPOW2_7;
            updateZobristKey(ROOK_WHITE, 4);
            updateZobristKey(ROOK_WHITE, 7);
        }
    } else {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            ASSERT(getPieceAt(side, POW2_59) == KING_BLACK);
            ASSERT(getPieceAt(side, POW2_58) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_57) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_56) == ROOK_BLACK);
            chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_57) & NOTPOW2_59;
            updateZobristKey(KING_BLACK, 57);
            updateZobristKey(KING_BLACK, 59);
            chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_58) & NOTPOW2_56;
            updateZobristKey(ROOK_BLACK, 58);
            updateZobristKey(ROOK_BLACK, 56);
        } else {
            ASSERT(type & QUEEN_SIDE_CASTLE_MOVE_MASK);
            ASSERT(getPieceAt(side, POW2_59) == KING_BLACK);
            ASSERT(getPieceAt(side, POW2_60) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_61) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_62) == SQUARE_FREE);
            ASSERT(getPieceAt(side, POW2_63) == ROOK_BLACK);
            chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_61) & NOTPOW2_59;
            updateZobristKey(KING_BLACK, 61);
            updateZobristKey(KING_BLACK, 59);
            chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_60) & NOTPOW2_63;
            updateZobristKey(ROOK_BLACK, 60);
            updateZobristKey(ROOK_BLACK, 63);
        }
    }
}

int GenMoves::getMobilityCastle(const int side, const u64 allpieces) {
    ASSERT_RANGE(side, 0, 1);
    int count = 0;
    if (side == WHITE) {
        if (POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x6ULL) && chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_0 && !isAttacked<WHITE>(1, allpieces) && !isAttacked<WHITE>(2, allpieces) && !isAttacked<WHITE>(3, allpieces)) {
            count++;
        }
        if (POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x70ULL) && chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_7 && !isAttacked<WHITE>(3, allpieces) && !isAttacked<WHITE>(4, allpieces) && !isAttacked<WHITE>(5, allpieces)) {
            count++;
        }
    } else {
        if (POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK && !(allpieces & 0x600000000000000ULL) && chessboard[ROOK_BLACK] & POW2_56 && !isAttacked<BLACK>(57, allpieces) && !isAttacked<BLACK>(58, allpieces) && !isAttacked<BLACK>(59, allpieces)) {
            count++;
        }
        if (POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK && !(allpieces & 0x7000000000000000ULL) && chessboard[ROOK_BLACK] & POW2_63 && !isAttacked<BLACK>(59, allpieces) && !isAttacked<BLACK>(60, allpieces) && !isAttacked<BLACK>(61, allpieces)) {
            count++;
        }
    }
    return count;
}

void GenMoves::tryAllCastle(const int side, const u64 allpieces) {
    ASSERT_RANGE(side, 0, 1);
    if (side == WHITE) {
        if (POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x6ULL) && chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_0 && !isAttacked<WHITE>(1, allpieces) && !isAttacked<WHITE>(2, allpieces) && !isAttacked<WHITE>(3, allpieces)) {
            pushmove<KING_SIDE_CASTLE_MOVE_MASK>(-1, -1, WHITE, NO_PROMOTION, -1);
        }
        if (POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x70ULL) && chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_7 && !isAttacked<WHITE>(3, allpieces) && !isAttacked<WHITE>(4, allpieces) && !isAttacked<WHITE>(5, allpieces)) {
            pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK>(-1, -1, WHITE, NO_PROMOTION, -1);
        }
    } else {
        if (POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK && !(allpieces & 0x600000000000000ULL) && chessboard[ROOK_BLACK] & POW2_56 && !isAttacked<BLACK>(57, allpieces) && !isAttacked<BLACK>(58, allpieces) && !isAttacked<BLACK>(59, allpieces)) {
            pushmove<KING_SIDE_CASTLE_MOVE_MASK>(-1, -1, BLACK, NO_PROMOTION, -1);
        }
        if (POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK && !(allpieces & 0x7000000000000000ULL) && chessboard[ROOK_BLACK] & POW2_63 && !isAttacked<BLACK>(59, allpieces) && !isAttacked<BLACK>(60, allpieces) && !isAttacked<BLACK>(61, allpieces)) {
            pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK>(-1, -1, BLACK, NO_PROMOTION, -1);
        }
    }
}

bool GenMoves::performKnightShiftCapture(const int piece, const u64 enemies, const int side) {
    ASSERT_RANGE(piece, 0, 11);
    ASSERT_RANGE(side, 0, 1);
    u64 x = chessboard[piece];
    while (x) {
        int pos = BITScanForward(x);
        u64 x1 = enemies & KNIGHT_MASK[pos];
        while (x1) {
            if (pushmove<STANDARD_MOVE_MASK>(pos, BITScanForward(x1), side, NO_PROMOTION, piece)) {
                return true;
            }
            RESET_LSB(x1);
        };
        RESET_LSB(x);
    }
    return false;
}

bool GenMoves::performKingShiftCapture(int side, const u64 enemies) {
    ASSERT_RANGE(side, 0, 1);
    int pos = BITScanForward(chessboard[KING_BLACK + side]);
    ASSERT(pos != -1);
    u64 x1 = enemies & NEAR_MASK1[pos];
    while (x1) {
        if (pushmove<STANDARD_MOVE_MASK>(pos, BITScanForward(x1), side, NO_PROMOTION, KING_BLACK + side)) {
            return true;
        }
        RESET_LSB(x1);
    };
    return false;
}


//template<int side>
//bool GenMoves::attackSquare(const uchar position, u64 allpieces) {
//    ASSERT_RANGE(position, 0, 63);
//    ASSERT_RANGE(side, 0, 1);
//    if (KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + (side ^ 1)]) {
//        return true;
//    }
//    if (NEAR_MASK1[position] & chessboard[KING_BLACK + (side ^ 1)]) {
//        return true;
//    }
//    //enpassant
//    if (PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + (side ^ 1)]) {
//        return true;
//    }
//    allpieces |= POW2[position];
//    u64 enemies = chessboard[QUEEN_BLACK + (side ^ 1)] | chessboard[BISHOP_BLACK + (side ^ 1)];
//    if (LEFT_RIGHT_DIAG[position] & enemies) {
//        ///LEFT
//        u64 q = allpieces & MASK_BIT_UNSET_LEFT_UP[position];
//        if (q && enemies & POW2[BITScanReverse(q)]) {
//            return true;
//        }
//        q = allpieces & MASK_BIT_UNSET_LEFT_DOWN[position];
//        if (q && enemies & POW2[BITScanForward(q)]) {
//            return true;
//        }
//        ///RIGHT
//        q = allpieces & MASK_BIT_UNSET_RIGHT_UP[position];
//        if (q && enemies & POW2[BITScanReverse(q)]) {
//            return true;
//        }
//        q = allpieces & MASK_BIT_UNSET_RIGHT_DOWN[position];
//        if (q && enemies & POW2[BITScanForward(q)]) {
//            return true;
//        }
//    }
//    ///
//    u64 x = allpieces & FILE_[position];
//    enemies = chessboard[QUEEN_BLACK + (side ^ 1)] | chessboard[ROOK_BLACK + (side ^ 1)];
//    if (x & enemies) {
//        u64 q = x & MASK_BIT_UNSET_UP[position];
//        if (q && enemies & POW2[BITScanReverse(q)]) {
//            return true;
//        }
//        q = x & MASK_BIT_UNSET_DOWN[position];
//        if (q && enemies & POW2[BITScanForward(q)]) {
//            return true;
//        }
//    }
//    x = allpieces & RANK[position];
//    if (x & enemies) {
//        u64 q = x & MASK_BIT_UNSET_RIGHT[position];
//        if (q && enemies & POW2[BITScanForward(q)]) {
//            return true;
//        }
//        q = x & MASK_BIT_UNSET_LEFT[position];
//        if (q && enemies & POW2[BITScanReverse(q)]) {
//            return true;
//        }
//    }
//    return false;
//}

void GenMoves::unPerformCastle(const int side, const uchar type) {
    ASSERT_RANGE(side, 0, 1);
    if (side == WHITE) {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            ASSERT(getPieceAt(side, POW2_1) == KING_WHITE);
            ASSERT(getPieceAt(side, POW2_0) == 12);
            ASSERT(getPieceAt(side, POW2_3) == 12);
            ASSERT(getPieceAt(side, POW2_2) == ROOK_WHITE);
            chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_3) & NOTPOW2_1;
            chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_0) & NOTPOW2_2;
        } else {
            chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_3) & NOTPOW2_5;
            chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_7) & NOTPOW2_4;
        }
    } else {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_59) & NOTPOW2_57;
            chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_56) & NOTPOW2_58;
        } else {
            chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_59) & NOTPOW2_61;
            chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_63) & NOTPOW2_60;
        }
    }
}

void GenMoves::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    if (rep) {
        popStackMove();
    }
    chessboard[ZOBRISTKEY_IDX] = oldkey;
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    int pieceFrom, posTo, posFrom, movecapture;
    chessboard[RIGHT_CASTLE_IDX] = move->type & 0xf0;
    if ((move->type & 0x3) == STANDARD_MOVE_MASK || (move->type & 0x3) == ENPASSANT_MOVE_MASK) {
        posTo = move->to;
        posFrom = move->from;
        movecapture = move->capturedPiece;
        ASSERT_RANGE(posFrom, 0, 63);
        ASSERT_RANGE(posTo, 0, 63);
        pieceFrom = move->pieceFrom;
        chessboard[pieceFrom] = (chessboard[pieceFrom] & NOTPOW2[posTo]) | POW2[posFrom];
        if (movecapture != SQUARE_FREE) {
            if (((move->type & 0x3) != ENPASSANT_MOVE_MASK)) {
                chessboard[movecapture] |= POW2[posTo];
            } else {
                ASSERT(movecapture == (move->side ^ 1));
                if (move->side) {
                    chessboard[movecapture] |= POW2[posTo - 8];
                } else {
                    chessboard[movecapture] |= POW2[posTo + 8];
                }
            }
        }
    } else if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
        posTo = move->to;
        posFrom = move->from;
        movecapture = move->capturedPiece;
        ASSERT(posTo >= 0 && move->side >= 0 && move->promotionPiece >= 0);
        chessboard[(uchar) move->side] |= POW2[posFrom];
        chessboard[(uchar) move->promotionPiece] &= NOTPOW2[posTo];
        if (movecapture != SQUARE_FREE) {
            chessboard[movecapture] |= POW2[posTo];
        }
    } else if (move->type & 0xc) { //castle
        unPerformCastle(move->side, move->type);
    }
}


bool GenMoves::makemove(_Tmove *move, bool rep, bool checkInCheck) {
    ASSERT(move);
    ASSERT(bitCount(chessboard[KING_WHITE]) == 1 && bitCount(chessboard[KING_BLACK]) == 1);
    int pieceFrom = SQUARE_FREE, posTo, posFrom, movecapture = SQUARE_FREE;
    uchar rightCastleOld = chessboard[RIGHT_CASTLE_IDX];
    if (!(move->type & 0xc)) { //no castle
        posTo = move->to;
        posFrom = move->from;
        movecapture = move->capturedPiece;
        ASSERT_RANGE(posFrom, 0, 63);
        ASSERT_RANGE(posTo, 0, 63);
        pieceFrom = move->pieceFrom;
        if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
            chessboard[pieceFrom] &= NOTPOW2[posFrom];
            updateZobristKey(pieceFrom, posFrom);
            ASSERT(move->promotionPiece >= 0);
            chessboard[(uchar) move->promotionPiece] |= POW2[posTo];
            updateZobristKey((uchar) move->promotionPiece, posTo);
        } else {
            chessboard[pieceFrom] = (chessboard[pieceFrom] | POW2[posTo]) & NOTPOW2[posFrom];
            updateZobristKey(pieceFrom, posFrom);
            updateZobristKey(pieceFrom, posTo);
        }
        if (movecapture != SQUARE_FREE) {
            if ((move->type & 0x3) != ENPASSANT_MOVE_MASK) {
                chessboard[movecapture] &= NOTPOW2[posTo];
                updateZobristKey(movecapture, posTo);
            } else { //en passant
                ASSERT(movecapture == (move->side ^ 1));
                if (move->side) {
                    chessboard[movecapture] &= NOTPOW2[posTo - 8];
                    updateZobristKey(movecapture, posTo - 8);
                } else {
                    chessboard[movecapture] &= NOTPOW2[posTo + 8];
                    updateZobristKey(movecapture, posTo + 8);
                }
            }
        }
        //lost castle right
        switch (pieceFrom) {
            case KING_WHITE: {
                chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
            }
                break;
            case KING_BLACK: {
                chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
            }
                break;
            case ROOK_WHITE:
                if (posFrom == 0) {
                    chessboard[RIGHT_CASTLE_IDX] &= 0xef;
                } else if (posFrom == 7) {
                    chessboard[RIGHT_CASTLE_IDX] &= 0xdf;
                }
                break;
            case ROOK_BLACK:
                if (posFrom == 56) {
                    chessboard[RIGHT_CASTLE_IDX] &= 0xbf;
                } else if (posFrom == 63) {
                    chessboard[RIGHT_CASTLE_IDX] &= 0x7f;
                }
                break;
                //en passant
            case PAWN_WHITE:
                if ((RANK_1 & POW2[posFrom]) && (RANK_3 & POW2[posTo])) {
                    chessboard[ENPASSANT_IDX] = posTo;
                    updateZobristKey(13, chessboard[ENPASSANT_IDX]);
                }
                break;
            case PAWN_BLACK:
                if ((RANK_6 & POW2[posFrom]) && (RANK_4 & POW2[posTo])) {
                    chessboard[ENPASSANT_IDX] = posTo;
                    updateZobristKey(13, chessboard[ENPASSANT_IDX]);
                }
                break;
            default:;
        }
    } else { //castle
        performCastle(move->side, move->type);
        if (move->side == WHITE) {
            chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
        } else {
            chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
        }
    }
    u64 x2 = rightCastleOld ^chessboard[RIGHT_CASTLE_IDX];
    while (x2) {
        int position = BITScanForward(x2);
        updateZobristKey(14, position);
        RESET_LSB(x2);
    }
    if (rep) {
        if (movecapture != SQUARE_FREE || pieceFrom == WHITE || pieceFrom == BLACK || move->type & 0xc) {
            pushStackMove(0);
        }
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }
    if ((forceCheck || (checkInCheck && !perftMode)) && ((move->side == WHITE && inCheck<WHITE>()) || (move->side == BLACK && inCheck<BLACK>()))) {
        return false;
    }
    return true;
}

void GenMoves::init() {
    numMoves = numMovesq = listId = 0;
#ifdef DEBUG_MODE
    nCutFp = nCutRazor = 0;
    betaEfficiency = 0.0;
    nCutAB = 0;
    nNullMoveCut = 0;
#endif
}

u64 GenMoves::getTotMoves() {
    return numMoves + numMovesq;
}

void GenMoves::setRepetitionMapCount(int i) {
    repetitionMapCount = i;
}

int GenMoves::loadFen(string fen) {
    repetitionMapCount = 0;
    int side = ChessBoard::loadFen(fen);
    if (side == 2) {
        fatal("Bad FEN position format ", fen);
        std::_Exit(1);
    }
    return side;
}

int GenMoves::getMoveFromSan(const string fenStr, _Tmove *move) {
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    memset(move, 0, sizeof(_Tmove));
    static const string MATCH_QUEENSIDE = "O-O-O e1c1 e8c8";
    static const string MATCH_QUEENSIDE_WHITE = "O-O-O e1c1";
    static const string MATCH_KINGSIDE_WHITE = "O-O e1g1";
    static const string MATCH_QUEENSIDE_BLACK = "O-O-O e8c8";
    static const string MATCH_KINGSIDE_BLACK = "O-O e8g8";
    if (((MATCH_QUEENSIDE_WHITE.find(fenStr) != string::npos || MATCH_KINGSIDE_WHITE.find(fenStr) != string::npos) && getPieceAt<WHITE>(POW2[E1]) == KING_WHITE) || ((MATCH_QUEENSIDE_BLACK.find(fenStr) != string::npos || MATCH_KINGSIDE_BLACK.find(fenStr) != string::npos) && getPieceAt<BLACK>(POW2[E8]) == KING_BLACK)) {
        if (MATCH_QUEENSIDE.find(fenStr) != string::npos) {
            move->type = QUEEN_SIDE_CASTLE_MOVE_MASK;
            move->from = QUEEN_SIDE_CASTLE_MOVE_MASK;
        } else {
            move->from = KING_SIDE_CASTLE_MOVE_MASK;
            move->type = KING_SIDE_CASTLE_MOVE_MASK;
        }
        if (fenStr.find("1") != string::npos) {
            move->side = WHITE;
        } else if (fenStr.find("8") != string::npos) {
            move->side = BLACK;
        } else {
            _assert(0);
        }
        move->from = -1;
        move->capturedPiece = SQUARE_FREE;
        return move->side;
    }
    int from = -1;
    int to = -1;
    for (int i = 0; i < 64; i++) {
        if (!fenStr.compare(0, 2, BOARD[i])) {
            from = i;
            break;
        }
    }
    if (from == -1) {
        cout << fenStr << endl;
        _assert(0);
    }
    for (int i = 0; i < 64; i++) {
        if (!fenStr.compare(2, 2, BOARD[i])) {
            to = i;
            break;
        }
    }
    if (to == -1) {
        cout << fenStr << endl;
        _assert(0);
    }
    int pieceFrom;
    if ((pieceFrom = getPieceAt<WHITE>(POW2[from])) != 12) {
        move->side = WHITE;
    } else if ((pieceFrom = getPieceAt<BLACK>(POW2[from])) != 12) {
        move->side = BLACK;
    } else {
        cout << "fenStr: " << fenStr << " from: " << from << endl;
        _assert(0);
    }
    move->from = from;
    move->to = to;
    if (fenStr.length() == 4) {
        move->type = STANDARD_MOVE_MASK;
        if (pieceFrom == PAWN_WHITE || pieceFrom == PAWN_BLACK) {
            if (FILE_AT[from] != FILE_AT[to] && (move->side ^ 1 ? getPieceAt<WHITE>(POW2[to]) : getPieceAt<BLACK>(POW2[to])) == SQUARE_FREE) {
                move->type = ENPASSANT_MOVE_MASK;
            }
        }
    } else if (fenStr.length() == 5) {
        move->type = PROMOTION_MOVE_MASK;
        if (move->side == WHITE) {
            move->promotionPiece = INV_FEN[toupper(fenStr.at(4))];
        } else {
            move->promotionPiece = INV_FEN[(uchar) fenStr.at(4)];
        }
        ASSERT(move->promotionPiece != -1);
    }
    if (move->side == WHITE) {
        move->capturedPiece = getPieceAt<BLACK>(POW2[move->to]);
        move->pieceFrom = getPieceAt<WHITE>(POW2[move->from]);
    } else {
        move->capturedPiece = getPieceAt<WHITE>(POW2[move->to]);
        move->pieceFrom = getPieceAt<BLACK>(POW2[move->from]);
    }
    if (move->type == ENPASSANT_MOVE_MASK) {
        move->capturedPiece = !move->side;
    }
    return move->side;
}

void GenMoves::writeRandomFen(const vector<int> pieces) {
    while (1) {
        memset(chessboard, 0, sizeof(_Tchessboard));
        chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
        chessboard[SIDETOMOVE_IDX] = rand() % 2;
        chessboard[KING_BLACK] = POW2[rand() % 64];
        chessboard[KING_WHITE] = POW2[rand() % 64];
        u64 check = chessboard[KING_BLACK] | chessboard[KING_WHITE];
        for (unsigned long i = 0; i < pieces.size(); i++) {
            chessboard[pieces[i]] |= POW2[rand() % 64];
            check |= chessboard[pieces[i]];
        }

        if (bitCount(check) == (2 + (int) pieces.size()) && !inCheck<WHITE>() && !inCheck<BLACK>()) {
            cout << boardToFen() << "\n";
            loadFen(boardToFen());
            return;
        }
    }
}

bool GenMoves::generatePuzzle(const string type) {
    std::unordered_map<char, int> PIECES;
    PIECES['R'] = ROOK_BLACK;
    PIECES['P'] = PAWN_BLACK;
    PIECES['Q'] = QUEEN_BLACK;
    PIECES['B'] = BISHOP_BLACK;
    PIECES['N'] = KNIGHT_BLACK;

    const int TOT = 5000;
    vector<int> pieces;

    int side = BLACK;
    for (unsigned k = 0; k < TOT; k++) {
        pieces.clear();
        char c = toupper(type.at(0));
        _assert(c == 'K');
        for (unsigned i = 1; i < type.size(); i++) {
            c = toupper(type.at(i));
            if (!(c == 'K' || c == 'R' || c == 'P' || c == 'Q' || c == 'B' || c == 'N')) {
                return false;
            };
            if (c == 'K') {
                side = WHITE;
            } else {
                pieces.push_back(PIECES[c] + side);
            }
        }

        /* if (type == "KRKP") {
             pieces.push_back(ROOK_BLACK);
             pieces.push_back(PAWN_WHITE);
         } else if (type == "KQKP") {
             pieces.push_back(QUEEN_BLACK);
             pieces.push_back(PAWN_WHITE);

         } else if (type == "KBBKN") {
             pieces.push_back(BISHOP_BLACK);
             pieces.push_back(BISHOP_BLACK);
             pieces.push_back(KNIGHT_WHITE);

         } else if (type == "KQKR") {
             pieces.push_back(QUEEN_BLACK);
             pieces.push_back(ROOK_WHITE);

         } else if (type == "KRKB") {
             pieces.push_back(ROOK_BLACK);
             pieces.push_back(BISHOP_WHITE);

         } else if (type == "KRKN") {
             pieces.push_back(ROOK_BLACK);
             pieces.push_back(KNIGHT_WHITE);

         } else {
             cout << "error type";
             return;
         }*/
        writeRandomFen(pieces);
    }
    return true;
}

