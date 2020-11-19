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

#pragma once

#include "ChessBoard.h"
#include "Hash.h"
#include "util/Bitboard.h"
#include <vector>
#include "namespaces/board.h"


class GenMoves : public ChessBoard {

public:
    static const int MAX_MOVE = 130;

    GenMoves();

    virtual ~GenMoves();

    void setPerft(const bool b);

    bool generateCaptures(const int side, u64, u64);

    void generateMoves(const int side, const u64);

    template<int side>
    void generateMoves(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        tryAllCastle<side>(allpieces);
        performDiagShift<side>(BISHOP_BLACK + side, allpieces);
        performRankFileShift<side>(ROOK_BLACK + side, allpieces);
        performRankFileShift<side>(QUEEN_BLACK + side, allpieces);
        performDiagShift<side>(QUEEN_BLACK + side, allpieces);
        performPawnShift<side>(~allpieces);
        performKnightShiftCapture<side>(KNIGHT_BLACK + side, ~allpieces, false);
        performKingShiftCapture<side>(~allpieces, false);
    }

    template<int side>
    bool generateCaptures(const u64 enemies, const u64 friends) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        const u64 allpieces = enemies | friends;

        if (perftMode) {
            int kingPosition = BITScanForward(chessboard[KING_BLACK + side]);
            pinned = board::getPinned<side>(allpieces, friends, kingPosition, chessboard);
            isInCheck = board::isAttacked<side>(kingPosition, allpieces, chessboard);
        }

        if (performPawnCapture<side>(enemies)) {
            return true;
        }
        if (performKingShiftCapture<side>(enemies, true)) {
            return true;
        }
        if (performKnightShiftCapture<side>(KNIGHT_BLACK + side, enemies, true)) {
            return true;
        }
        if (performDiagCapture<side>(BISHOP_BLACK + side, enemies, allpieces)) {
            return true;
        }
        if (performRankFileCapture<side>(ROOK_BLACK + side, enemies, allpieces)) {
            return true;
        }
        if (performRankFileCapture<side>(QUEEN_BLACK + side, enemies, allpieces)) {
            return true;
        }
        if (performDiagCapture<side>(QUEEN_BLACK + side, enemies, allpieces)) {
            return true;
        }
        return false;
    }

    bool getForceCheck() const {
        return forceCheck;
    }

    void setForceCheck(bool b) const {
        forceCheck = b;
    }

    int getMoveFromSan(const string fenStr, _Tmove *move);

    void init();

    int loadFen(string fen = "");

    inline u64 getDiagCapture(const int position, const u64 allpieces, const u64 enemies) const {
        ASSERT_RANGE(position, 0, 63);
        return Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
    }

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setRepetitionMapCount(const int i);

    template<int side>
    bool performKingShiftCapture(const u64 enemies, const bool isCapture) {
        BENCH(times->start("kingShiftCapture"))
        ASSERT_RANGE(side, 0, 1)
        const int pos = BITScanForward(chessboard[KING_BLACK + side]);
        ASSERT(pos != -1)

        for (u64 x1 = enemies & NEAR_MASK1[pos]; x1; RESET_LSB(x1)) {
            BENCH(times->subProcess("kingShiftCapture", "pushmove"))
            if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, KING_BLACK + side,
                                                   isCapture)) {
                BENCH(times->stop("kingShiftCapture"))
                return true;
            }
        }
        BENCH(times->stop("kingShiftCapture"))
        return false;
    }


    template<int side>
    bool performKnightShiftCapture(const int piece, const u64 enemies, const bool isCapture) {
        BENCH(times->start("knightShiftCapture"))
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)
        for (u64 x = chessboard[piece]; x; RESET_LSB(x)) {
            const int pos = BITScanForward(x);
            for (u64 x1 = enemies & KNIGHT_MASK[pos]; x1; RESET_LSB(x1)) {
                BENCH(times->subProcess("knightShiftCapture", "pushmove"))
                if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, piece, isCapture)) {
                    BENCH(times->stop("knightShiftCapture"))
                    return true;
                }
            }
        }
        BENCH(times->stop("knightShiftCapture"))
        return false;
    }

    template<int side>
    bool performDiagCapture(const int piece, const u64 enemies, const u64 allpieces) {
        BENCH(times->start("diagCapture"))
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)
        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 diag = getDiagonalAntiDiagonal(position, allpieces) & enemies;
            for (; diag; RESET_LSB(diag)) {
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece, true)) {
                    BENCH(times->stop("diagCapture"))
                    return true;
                }
            }
        }
        BENCH(times->stop("diagCapture"))
        return false;
    }

    u64 getTotMoves() const;

    template<int side>
    bool performRankFileCapture(const int piece, const u64 enemies, const u64 allpieces) {
        BENCH(times->start("rankFileCapture"))
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)

        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 rankFile = getRankFile(position, allpieces) & enemies;
            for (; rankFile; RESET_LSB(rankFile)) {
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece, true)) {
                    BENCH(times->stop("rankFileCapture"))
                    return true;
                }
            }
        }
        BENCH(times->stop("rankFileCapture"))
        return false;
    }

    template<int side>
    bool performPawnCapture(const u64 enemies) {
        BENCH(times->start("pawnCapture"))
        if (!chessboard[side]) {
            if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
                updateZobristKey(13, chessboard[ENPASSANT_IDX]);
            }
            chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
            BENCH(times->stop("pawnCapture"))
            return false;
        }
        constexpr int sh = side ? -7 : 7;

        u64 x = shiftForward<side, 7>(chessboard[side]) & enemies;
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {//PROMOTION
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, QUEEN_BLACK + side, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;        //queen
                }
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, KNIGHT_BLACK + side, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;        //knight
                }
                if (perftMode) {
                    BENCH(times->subProcess("pawnCapture", "pushmove"))
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, ROOK_BLACK + side, side, true)) {
                        BENCH(times->stop("pawnCapture"))
                        return true;        //rock
                    }
                    BENCH(times->subProcess("pawnCapture", "pushmove"))
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, BISHOP_BLACK + side, side, true)) {
                        BENCH(times->stop("pawnCapture"))
                        return true;        //bishop
                    }
                }
            } else {
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<STANDARD_MOVE_MASK, side>(o + sh, o, NO_PROMOTION, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;
                }
            }
        }
        constexpr int sh2 = side ? -9 : 9;
        x = shiftForward<side, 9>(chessboard[side]) & enemies;

        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {    //PROMOTION
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, QUEEN_BLACK + side, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;        //queen
                }
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, KNIGHT_BLACK + side, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;        //knight
                }
                if (perftMode) {
                    BENCH(times->subProcess("pawnCapture", "pushmove"))
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, BISHOP_BLACK + side, side, true)) {
                        BENCH(times->stop("pawnCapture"))
                        return true;        //bishop
                    }
                    BENCH(times->subProcess("pawnCapture", "pushmove"))
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, ROOK_BLACK + side, side, true)) {
                        BENCH(times->stop("pawnCapture"))
                        return true;        //rock
                    }
                }
            } else {
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                if (pushmove<STANDARD_MOVE_MASK, side>(o + sh2, o, NO_PROMOTION, side, true)) {
                    BENCH(times->stop("pawnCapture"))
                    return true;
                }
            }
        }
        //ENPASSANT
        if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
            x = ENPASSANT_MASK[side ^ 1][chessboard[ENPASSANT_IDX]] & chessboard[side];
            for (; x; RESET_LSB(x)) {
                const int o = BITScanForward(x);
                BENCH(times->subProcess("pawnCapture", "pushmove"))
                pushmove<ENPASSANT_MOVE_MASK, side>(o,
                                                    (side ? chessboard[ENPASSANT_IDX] + 8 : chessboard[ENPASSANT_IDX] -
                                                                                            8),
                                                    NO_PROMOTION, side, true);

            }
            updateZobristKey(13, chessboard[ENPASSANT_IDX]);
            chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
        }
        BENCH(times->stop("pawnCapture"))
        return false;
    }

    template<int side>
    void performPawnShift(const u64 xallpieces) {
        u64 x = chessboard[side];
        checkJumpPawn<side>(x, xallpieces);
        BENCH(times->start("pawnShift"))
        constexpr int sh = side ? -8 : 8;
        x = side ? x << 8 : x >> 8;

        x &= xallpieces;
        BENCH(times->stop("pawnShift"))
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            ASSERT(board::getPieceAt<side>(POW2[o + sh], chessboard) != SQUARE_EMPTY);
            ASSERT(board::board::getBitmap(side, chessboard) & POW2[o + sh]);
            if (o > A7 || o < H2) {
                pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, QUEEN_BLACK + side, side, false);
                pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, KNIGHT_BLACK + side, side, false);
                if (perftMode) {
                    pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, BISHOP_BLACK + side, side, false);
                    pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, ROOK_BLACK + side, side, false);
                }
            } else {
                pushmove<STANDARD_MOVE_MASK, side>(o + sh, o, NO_PROMOTION, side, false);
            }
        }
    }

    void clearHeuristic();

    template<int side>
    void performDiagShift(const int piece, const u64 allpieces) {
        BENCH(times->start("diagShift"))
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)
        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 diag = getDiagonalAntiDiagonal(position, allpieces) & ~allpieces;
            for (; diag; RESET_LSB(diag)) {
                BENCH(times->subProcess("diagShift", "pushmove"))
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece, false);
            }
        }
        BENCH(times->stop("diagShift"))
    }

    template<int side>
    void performRankFileShift(const int piece, const u64 allpieces) {
        BENCH(times->start("rankFileShift"))
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)

        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 rankFile = getRankFile(position, allpieces) & ~allpieces;
            for (; rankFile; RESET_LSB(rankFile)) {
                BENCH(times->subProcess("rankFileShift", "pushmove"))
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece, false);
            }
        }
        BENCH(times->stop("rankFileShift"))
    }

    bool makemove(const _Tmove *move, const bool rep, const bool);

    void incListId() {
        listId++;
        ASSERT(listId >= 0);
        ASSERT(listId < MAX_PLY);
    }

    void decListId() {
        ASSERT(listId > -1);
        gen_list[listId--].size = 0;
    }

    int getListSize() const {
        return gen_list[listId].size;
    }

    void pushStackMove() {
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }

    void resetList() {
        gen_list[listId].size = 0;
    }

    bool generatePuzzle(const string type);

    void incHistoryHeuristic(const int from, const int to, const int value) {
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        ASSERT(historyHeuristic[from][to] <= historyHeuristic[from][to] + value);
        historyHeuristic[from][to] += value;
    }


#ifdef DEBUG_MODE
    unsigned nCutAB, nNullMoveCut, nCutFp, nCutRazor;
    double betaEfficiency;
#endif


    static constexpr int NO_PROMOTION = -1;
protected:

    u64 pinned;
    bool perftMode;
    int listId;
    _TmoveP *gen_list;

    static constexpr u64 RANK_2 = 0xff00ULL;
    static constexpr u64 RANK_3 = 0xff000000ULL;
    static constexpr u64 RANK_5 = 0xff00000000ULL;
    static constexpr u64 RANK_7 = 0xff000000000000ULL;
    static constexpr uchar STANDARD_MOVE_MASK = 0x3;
    static constexpr uchar ENPASSANT_MOVE_MASK = 0x1;
    static constexpr uchar PROMOTION_MOVE_MASK = 0x2;
    static constexpr int MAX_REP_COUNT = 1024;

    int repetitionMapCount;

    u64 *repetitionMap;
    int currentPly;

    u64 numMoves = 0;
    u64 numMovesq = 0;

    _Tmove *getNextMove(decltype(gen_list), const int depth, const Hash::_ThashData *c, const int first);

    template<int side>
    int getMobilityCastle(const u64 allpieces) const {
        ASSERT_RANGE(side, 0, 1)
        if (chess960) return 0;
        int count = 0;
        if (side == WHITE) {
            if (allowCastleWhiteKing(allpieces)) count++;
            if (allowCastleWhiteQueen(allpieces)) count++;
        } else {
            if (allowCastleBlackKing(allpieces)) count++;
            if (allowCastleBlackQueen(allpieces)) count++;
        }
        return count;
    }

    int historyHeuristic[64][64];
    unsigned short killer[3][MAX_PLY];

#ifdef DEBUG_MODE

    template<int side, uchar type>
    bool inCheckSlow(const int from, const int to, const int pieceFrom, const int pieceTo, const int promotionPiece) {
        bool result;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_EMPTY);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2[to];
                };
                chessboard[pieceFrom] &= NOTPOW2[from];
                chessboard[pieceFrom] |= POW2[to];
                ASSERT(chessboard[KING_BLACK]);
                ASSERT(chessboard[KING_WHITE]);

                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[pieceFrom] = from1;
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] = to1;
                }
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                }
                u64 from1 = chessboard[pieceFrom];
                u64 p1 = chessboard[promotionPiece];
                chessboard[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] &= NOTPOW2[to];
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2[to];
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] = to1;
                }
                chessboard[pieceFrom] = from1;
                chessboard[promotionPiece] = p1;
                break;
            }
            case ENPASSANT_MOVE_MASK: {
                u64 to1 = chessboard[side ^ 1];
                u64 from1 = chessboard[side];
                chessboard[side] &= NOTPOW2[from];
                chessboard[side] |= POW2[to];
                if (side) {
                    chessboard[side ^ 1] &= NOTPOW2[to - 8];
                } else {
                    chessboard[side ^ 1] &= NOTPOW2[to + 8];
                }
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[side ^ 1] = to1;
                chessboard[side] = from1;;
                break;
            }
            default:
                _assert(0);
        }

        return result;
    }

#endif

    template<int type, uchar side>
    bool inCheck(const int from, const int to, const int pieceFrom, const int pieceTo, int promotionPiece) {
        BENCH(times->start("inCheck"))
#ifdef DEBUG_MODE
        _Tchessboard a;
        memcpy(&a, chessboard, sizeof(_Tchessboard));
#endif
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        ASSERT_RANGE(side, 0, 1);
        ASSERT_RANGE(pieceFrom, 0, 12);
        ASSERT_RANGE(pieceTo, 0, 12);
        ASSERT(perftMode || forceCheck);
        ASSERT(!(type & 0xc));
        if (perftMode) {

            if ((KING_BLACK + side) != pieceFrom && !isInCheck) {
                if (!(pinned & POW2[from]) || (LINES[from][to] & chessboard[KING_BLACK + side])) {
                    ASSERT(!(inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
                    BENCH(times->stop("inCheck"))
                    return false;
                } else {
                    ASSERT ((inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
                    BENCH(times->stop("inCheck"))
                    return true;
                }
            }

        }

        bool result = 0;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_EMPTY);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2[to];
                };
                chessboard[pieceFrom] &= NOTPOW2[from];
                chessboard[pieceFrom] |= POW2[to];
                ASSERT(chessboard[KING_BLACK]);
                ASSERT(chessboard[KING_WHITE]);

                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[pieceFrom] = from1;
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] = to1;
                }
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                }
                u64 from1 = chessboard[pieceFrom];
                u64 p1 = chessboard[promotionPiece];
                chessboard[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] &= NOTPOW2[to];
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2[to];
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] = to1;
                }
                chessboard[pieceFrom] = from1;
                chessboard[promotionPiece] = p1;
                break;
            }
            case ENPASSANT_MOVE_MASK: {
                u64 to1 = chessboard[side ^ 1];
                u64 from1 = chessboard[side];
                chessboard[side] &= NOTPOW2[from];
                chessboard[side] |= POW2[to];
                if (side) {
                    chessboard[side ^ 1] &= NOTPOW2[to - 8];
                } else {
                    chessboard[side ^ 1] &= NOTPOW2[to + 8];
                }
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[side ^ 1] = to1;
                chessboard[side] = from1;;
                break;
            }
            default:
                _assert(0);
        }

        ASSERT(!memcmp(&a, chessboard, sizeof(_Tchessboard)));
        BENCH(times->stop("inCheck"))
        return result;
    }

    void performCastle(const int side, const uchar type);

    void unPerformCastle(const int side, const uchar type);

    template<int side>
    void tryAllCastle(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1)
        BENCH(times->start("castle"))
        if (chess960)tryAllCastle960<side>(allpieces);
        else
            tryAllCastleStandard<side>(allpieces);
        BENCH(times->stop("castle"))
    }

    bool allowKingSideBlack(const u64 allpieces) const {
        const auto a = board::isCastleRight_BlackKing(chessboard) &&
                       board::isPieceAt(KING_BLACK, startPosBlackKing, chessboard) &&
                       board::isPieceAt(ROOK_BLACK, startPosBlackRookKingSide, chessboard) &&
                       (!board::isOccupied(G8, allpieces) || startPosBlackKing == G8 ||
                        startPosBlackRookKingSide == G8) &&
                       (!board::isOccupied(F8, allpieces) || startPosBlackKing == F8 ||
                        startPosBlackRookKingSide == F8) &&
                       !(startPosBlackKing == G8 && startPosBlackRookKingSide == F8);
        if (!a)return a;
        const u64 path = LINK_SQUARE[startPosBlackKing][G8];
        const u64 rookPath = LINK_SQUARE[startPosBlackRookKingSide][F8] & NOTPOW2[startPosBlackKing];
        const u64 kingPath = path | POW2[G8] | POW2[startPosBlackKing];
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2[startPosBlackRookKingSide])) &&
                !board::anyAttack<BLACK>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<BLACK>(kingPath, allpieces & NOTPOW2[startPosBlackRookKingSide], chessboard));
    }

    bool allowQueenSideBlack(const u64 allpieces) const {
        auto a = board::isCastleRight_BlackQueen(chessboard) &&
                 board::isPieceAt(KING_BLACK, startPosBlackKing, chessboard) &&
                 board::isPieceAt(ROOK_BLACK, startPosBlackRookQueenSide, chessboard) &&
                 (!board::isOccupied(C8, allpieces) || startPosBlackKing == C8 || startPosBlackRookQueenSide == C8) &&
                 (!board::isOccupied(D8, allpieces) || startPosBlackKing == D8 || startPosBlackRookQueenSide == D8) &&
                 !(startPosWhiteKing == C8 && startPosWhiteRookQueenSide == D8);
        if (!a)return false;
        const u64 rookPath = LINK_SQUARE[startPosBlackRookQueenSide][D8] & NOTPOW2[startPosBlackKing];
        const u64 path = LINK_SQUARE[startPosBlackKing][C8];
        const u64 kingPath = path | POW2[C8] | POW2[startPosBlackKing];
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2[startPosBlackRookQueenSide])) &&
                !board::anyAttack<BLACK>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<BLACK>(kingPath, allpieces & NOTPOW2[startPosBlackRookQueenSide],
                                         chessboard));
    }

    bool allowCastleBlackQueen(const u64 allpieces) const;

    bool allowCastleWhiteQueen(const u64 allpieces) const;

    bool allowCastleBlackKing(const u64 allpieces) const;

    bool allowCastleWhiteKing(const u64 allpieces) const;

    bool allowQueenSideWhite(const u64 allpieces) const {
        const auto a = board::isCastleRight_WhiteQueen(chessboard) &&
                       board::isPieceAt(KING_WHITE, startPosWhiteKing, chessboard) &&
                       board::isPieceAt(ROOK_WHITE, startPosWhiteRookQueenSide, chessboard) &&
                       (!board::isOccupied(C1, allpieces) || startPosWhiteKing == C1 ||
                        startPosWhiteRookQueenSide == C1) &&
                       (!board::isOccupied(D1, allpieces) || startPosWhiteKing == D1 ||
                        startPosWhiteRookQueenSide == D1) &&
                       !(startPosWhiteKing == C1 && startPosWhiteRookQueenSide == D1);
        if (!a)return false;
        const u64 path = LINK_SQUARE[startPosWhiteKing][C1];
        const u64 rookPath = LINK_SQUARE[startPosWhiteRookQueenSide][D1] & NOTPOW2[startPosWhiteKing];
        const u64 kingPath = path | POW2[C1] | POW2[startPosWhiteKing];
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2[startPosWhiteRookQueenSide])) &&
                !board::anyAttack<WHITE>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<WHITE>(kingPath, allpieces & NOTPOW2[startPosWhiteRookQueenSide], chessboard));
    }

    bool allowKingSideWhite(const u64 allpieces) const {
        const auto a = board::isCastleRight_WhiteKing(chessboard) &&
                       board::isPieceAt(KING_WHITE, startPosWhiteKing, chessboard) &&
                       board::isPieceAt(ROOK_WHITE, startPosWhiteRookKingSide, chessboard) &&
                       (!board::isOccupied(G1, allpieces) || startPosWhiteKing == G1 ||
                        startPosWhiteRookKingSide == G1) &&
                       (!board::isOccupied(F1, allpieces) || startPosWhiteKing == F1 ||
                        startPosWhiteRookKingSide == F1) &&
                       !(startPosWhiteKing == G1 && startPosWhiteRookKingSide == F1);
        if (!a)return a;
        const u64 path = LINK_SQUARE[startPosWhiteKing][G1];
        const u64 rookPath = LINK_SQUARE[startPosWhiteRookKingSide][F1] & NOTPOW2[startPosWhiteKing];
        const u64 kingPath = path | POW2[G1] | POW2[startPosWhiteKing];
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2[startPosWhiteRookKingSide])) &&
                !board::anyAttack<WHITE>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<WHITE>(kingPath, allpieces & NOTPOW2[startPosWhiteRookKingSide], chessboard));
    }

    template<int side>
    void tryAllCastle960(const u64 allpieces) {
        if (side == WHITE) {

            if (allowKingSideWhite(allpieces)) {

                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            }

            if (allowQueenSideWhite(allpieces)) {

                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            }

        } else {
            if (allowKingSideBlack(allpieces)) {

                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            }

            if (allowQueenSideBlack(allpieces)) {

                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            }
        }
    }

    template<int side>
    void tryAllCastleStandard(const u64 allpieces) {
        if (side == WHITE) {
            if (allowCastleWhiteKing(allpieces))
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            if (allowCastleWhiteQueen(allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            }
        } else {
            if (allowCastleBlackKing(allpieces))
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
            if (allowCastleBlackQueen(allpieces))
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(-1, -1, NO_PROMOTION, -1, false);
        }
    }
    template<uchar type, int side>
    bool pushmove(const int from, const int to, const int promotionPiece, const int pieceFrom, const bool isCapture) {
#ifdef DEBUG_MODE
        if (((type & 0x3) != ENPASSANT_MOVE_MASK) && !(type & 0xc)) {
            auto a = board::getPieceAt<side ^ 1>(POW2[to], chessboard);
            if (from != -1 && (isCapture && a == SQUARE_EMPTY || !isCapture && a != SQUARE_EMPTY)) {
                if (((type & 0x3) != ENPASSANT_MOVE_MASK)) {
                    display();
                    cout << isCapture << " " << BOARD[from] << " " << BOARD[to] << endl << flush;
                    cout << 1;
                }
            }
        }
#endif
        BENCH(times->start("pushmove"))
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        int piece_captured = SQUARE_EMPTY;
        bool res = false;
        if (((type & 0x3) != ENPASSANT_MOVE_MASK) && !(type & 0xc)) {
            if (isCapture) {
                piece_captured = board::getPieceAt<side ^ 1>(POW2[to], chessboard);
                if (piece_captured == KING_BLACK + (side ^ 1)) {
                    res = true;
                }
            }
        } else if (!(type & 0xc)) {//en passant
            piece_captured = side ^ 1;
        }
        if (!(type & 0xc) && (forceCheck || perftMode)) {
            BENCH(times->subProcess("pushmove", "inCheck"))
            if (inCheck<type, side>(from, to, pieceFrom, piece_captured, promotionPiece)) {
                BENCH(times->stop("pushmove"));
                return false;
            }
        }
        ASSERT_RANGE(listId, 0, MAX_PLY - 1);
        ASSERT(getListSize() < MAX_MOVE);
        auto mos = &gen_list[listId].moveList[getListSize()];
        ++gen_list[listId].size;
        mos->s.type = (uchar) chessboard[RIGHT_CASTLE_IDX] | type;
        mos->s.side = (char) side;
        mos->s.capturedPiece = piece_captured;
        if (type & 0x3) {
            mos->s.from = (uchar) from;
            mos->s.to = (uchar) to;
            mos->s.pieceFrom = pieceFrom;
            mos->s.promotionPiece = (char) promotionPiece;

        }
        ASSERT(getListSize() < MAX_MOVE);
        BENCH(times->stop("pushmove"));
        return res;
    }

    _Tmove *getMove(const int i) const {
        return &gen_list[listId].moveList[i];
    }

    void setRunning(const int t) {
        running = t;
    }

    int getRunning() const {
        return running;
    }

    void setHistoryHeuristic(const int from, const int to, const int value) {
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        historyHeuristic[from][to] = value;
    }

    void setKiller(const int from, const int to, const int ply, const bool isMate) {
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        if (isMate) killer[2][ply] = from | (to << 8);
        else {
            killer[1][ply] = killer[0][ply];
            killer[0][ply] = from | (to << 8);
        }
    }

    bool isKiller(const int idx, const int from, const int to, const int ply) {
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        unsigned short v = from | (to << 8);
        if (v == killer[idx][ply])return true;
        return false;
    }

    bool isKillerMate(const int from, const int to, const int ply) {
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        unsigned short v = from | (to << 8);
        if (v == killer[2][ply])return true;
        return false;
    }

private:
    int running;
    bool isInCheck;
    static bool forceCheck;
    static constexpr u64 TABJUMPPAWN = 0xFF00000000FF00ULL;

    void writeRandomFen(const vector<int>);

    template<int side>
    void checkJumpPawn(u64 x, const u64 xallpieces) {
        BENCH(times->start("checkJumpPawn"))

        x &= TABJUMPPAWN;
        if (!x) {
            BENCH(times->stop("checkJumpPawn"));
            return;
        }
        if (side) {
            x = (((x << 8) & xallpieces) << 8) & xallpieces;
        } else {
            x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
        }
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            BENCH(times->subProcess("checkJumpPawn", "pushmove"))
            pushmove<STANDARD_MOVE_MASK, side>(o + (side ? -16 : 16), o, NO_PROMOTION, side, false);
        }
        BENCH(times->stop("checkJumpPawn"))
    }

    void popStackMove() {
        ASSERT_RANGE(repetitionMapCount, 1, MAX_REP_COUNT - 1);
        if (--repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0) {
            repetitionMapCount--;
        }
    }

    void pushStackMove(const u64 key) {
        ASSERT(repetitionMapCount < MAX_REP_COUNT - 1);
        repetitionMap[repetitionMapCount++] = key;
    }

};

