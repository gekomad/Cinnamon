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
#include "util/bench/Bench.h"
#include "util/Bitboard.h"
#include <vector>
#include "namespaces/board.h"

class GenMoves : public ChessBoard {

public:
    static const int MAX_MOVE = 130;

    GenMoves();

    virtual ~GenMoves();

    void setPerft(const bool b);

    bool generateCaptures(const uchar side, const u64, const u64);

    void generateMoves(const uchar side, const u64);

    template<uchar side>
    __attribute__((always_inline)) void generateMoves(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1)
        assert(chessboard[KING_BLACK]);
        assert(chessboard[KING_WHITE]);
        tryAllCastle<side>(allpieces);
        performDiagShift<side>(BISHOP_BLACK + side, allpieces);
        performRankFileShift<side>(ROOK_BLACK + side, allpieces);
        performRankFileShift<side>(QUEEN_BLACK + side, allpieces);
        performDiagShift<side>(QUEEN_BLACK + side, allpieces);
        performPawnShift<side>(~allpieces);
        performKnightShiftCapture<side>(~allpieces, false);
        performKingShiftCapture<side>(~allpieces, false);
    }

#ifndef NDEBUG

    static void verifyPV(string fen, string pv) {
        cout << flush;
        std::string delimiter = " ";
        size_t pos;
        std::string token;
        GenMoves g;
        g.loadFen(fen);
        while ((pos = pv.find(delimiter)) != std::string::npos) {
            token = pv.substr(0, pos);
            //std::cout << token << std::endl;
            _Tmove move;
            int x = !g.getMoveFromSan(token, &move);
            g.setSide(x);
            g.makemove(&move, false);
            pv.erase(0, pos + delimiter.length());
        }
    }

#endif

    __attribute__((always_inline)) static bool isAttacked(const _Tmove &move, const _Tchessboard &chessboard, const u64 allpieces) {
        assert(allpieces == (board::getBitmap<WHITE>(chessboard) | board::getBitmap<BLACK>(chessboard)));
        return board::isAttacked(move.side, move.to, allpieces, chessboard);
    }

    template<uchar side>
    __attribute__((always_inline)) bool generateCaptures(const u64 enemies, const u64 friends) {
        ASSERT_RANGE(side, 0, 1)
        assert(chessboard[KING_BLACK]);
        assert(chessboard[KING_WHITE]);
        const u64 allpieces = enemies | friends;

        if (perftMode) {
            int kingPosition = BITScanForward(chessboard[KING_BLACK + side]);
            pinned = board::getPinned<side>(allpieces, friends, kingPosition, chessboard);
            isInCheck = board::isAttacked<side>(kingPosition, allpieces, chessboard);
        }

        if (performPawnCapture<side>(enemies)) return true;
        if (performKingShiftCapture<side>(enemies, true)) return true;
        if (performKnightShiftCapture<side>(enemies, true)) return true;
        if (performDiagCapture<side>(BISHOP_BLACK + side, enemies, allpieces)) return true;
        if (performRankFileCapture<side>(ROOK_BLACK + side, enemies, allpieces)) return true;
        if (performRankFileCapture<side>(QUEEN_BLACK + side, enemies, allpieces)) return true;
        if (performDiagCapture<side>(QUEEN_BLACK + side, enemies, allpieces)) return true;
        return false;
    }

    bool getForceCheck() const {
        return forceCheck;
    }

    void setForceCheck(const bool b) {
        forceCheck = b;
    }

    int getMoveFromSan(const string &fenStr, _Tmove *move);

    void init();

    void takeback(const _Tmove *move, const u64 oldkey, const uchar oldEnpassant, const bool rep);

    void setRepetitionMapCount(const int i);

    template<uchar side>
    __attribute__((always_inline)) bool performKingShiftCapture(const u64 enemies, const bool isCapture) {
        BENCH_AUTO_CLOSE("kingShiftCapture")
        ASSERT_RANGE(side, 0, 1)
        const int pos = BITScanForward(chessboard[KING_BLACK + side]);
        assert(pos != -1);

        for (u64 x1 = enemies & NEAR_MASK1[pos]; x1; RESET_LSB(x1)) {
            BENCH_SUBPROCESS("kingShiftCapture", "pushmove")
            if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, KING_BLACK + side,
                                                   isCapture)) {
                return true;
            }
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) bool performKnightShiftCapture(const u64 enemies, const bool isCapture) {
        BENCH_AUTO_CLOSE("knightShiftCapture")
        ASSERT_RANGE(side, 0, 1)
        for (u64 x = chessboard[KNIGHT_BLACK + side]; x; RESET_LSB(x)) {
            const int pos = BITScanForward(x);
            for (u64 x1 = enemies & KNIGHT_MASK[pos]; x1; RESET_LSB(x1)) {
                BENCH_SUBPROCESS("knightShiftCapture", "pushmove")
                if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, KNIGHT_BLACK + side,
                                                       isCapture
                ))
                    return true;

            }
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) bool performDiagCapture(const uchar piece, const u64 enemies, const u64 allpieces) {
        BENCH_AUTO_CLOSE("diagCapture")
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)
        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 diag = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
            for (; diag; RESET_LSB(diag)) {
                BENCH_SUBPROCESS("diagCapture", "pushmove")
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece, true))
                    return true;

            }
        }
        return false;
    }

    u64 getTotMoves() const;

    template<uchar side>
    __attribute__((always_inline)) bool performRankFileCapture(const uchar piece, const u64 enemies, const u64 allpieces) {
        BENCH_AUTO_CLOSE("rankFileCapture")
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)

        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 rankFile = Bitboard::getRankFile(position, allpieces) & enemies;
            for (; rankFile; RESET_LSB(rankFile)) {
                BENCH_SUBPROCESS("rankFileCapture", "pushmove")
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece, true))
                    return true;

            }
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) bool performPawnCapture(const u64 enemies) {
        BENCH_AUTO_CLOSE("pawnCapture")
        if (!chessboard[side]) {
            if (enPassant != NO_ENPASSANT) {
                updateZobristKey(ENPASSANT_RAND, enPassant);
            }
            enPassant = NO_ENPASSANT;
            return false;
        }
        constexpr int sh = side ? -7 : 7;

        u64 x = shiftForward<side, 7>(chessboard[side]) & enemies;
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            if ((side && o > A7) || (!side && o < H2)) {//PROMOTION
                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, QUEEN_BLACK + side, side, true)) return true;

                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, KNIGHT_BLACK + side, side, true)) return true;

                if (perftMode) {
                    BENCH_SUBPROCESS("pawnCapture", "pushmove")
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, ROOK_BLACK + side, side, true)) return true;

                    BENCH_SUBPROCESS("pawnCapture", "pushmove")
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, BISHOP_BLACK + side, side, true)) return true;
                }
            } else {
                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<STANDARD_MOVE_MASK, side>(o + sh, o, NO_PROMOTION, side, true)) return true;

            }
        }
        constexpr int sh2 = side ? -9 : 9;
        x = shiftForward<side, 9>(chessboard[side]) & enemies;

        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            if ((side && o > A7) || (!side && o < H2)) {    //PROMOTION
                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, QUEEN_BLACK + side, side, true)) return true;

                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, KNIGHT_BLACK + side, side, true)) return true;

                if (perftMode) {
                    BENCH_SUBPROCESS("pawnCapture", "pushmove")
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, BISHOP_BLACK + side, side, true)) return true;

                    BENCH_SUBPROCESS("pawnCapture", "pushmove")
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + sh2, o, ROOK_BLACK + side, side, true)) return true;
                }
            } else {
                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                if (pushmove<STANDARD_MOVE_MASK, side>(o + sh2, o, NO_PROMOTION, side, true)) return true;

            }
        }
        //enPassant
        if (enPassant != NO_ENPASSANT) {
            x = ENPASSANT_MASK[X(side)][enPassant] & chessboard[side];
            for (; x; RESET_LSB(x)) {
                const int o = BITScanForward(x);
                BENCH_SUBPROCESS("pawnCapture", "pushmove")
                pushmove<ENPASSANT_MOVE_MASK, side>(o, (side ? enPassant + 8 : enPassant - 8), NO_PROMOTION, side,
                                                    true);

            }
            updateZobristKey(ENPASSANT_RAND, enPassant);
            enPassant = NO_ENPASSANT;
        }
        return false;
    }

    template<uchar side>
    __attribute__((always_inline)) void performPawnShift(const u64 xallpieces) {
        u64 x = chessboard[side];
        performJumpPawn<side>(x, xallpieces);
        BENCH_AUTO_CLOSE("pawnShift")
        constexpr int sh = side ? -8 : 8;
        x = side ? x << 8 : x >> 8;

        x &= xallpieces;
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            assert(board::getPieceAt<side>(POW2(o + sh), chessboard) != SQUARE_EMPTY);
            assert(board::getBitmap(side, chessboard) & POW2(o + sh));
            if (o > A7 || o < H2) {
                BENCH_SUBPROCESS("pawnShift", "pushmove")
                pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, QUEEN_BLACK + side, side, false);
                BENCH_SUBPROCESS("pawnShift", "pushmove")
                pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, KNIGHT_BLACK + side, side, false);
                if (perftMode) {
                    BENCH_SUBPROCESS("pawnShift", "pushmove")
                    pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, BISHOP_BLACK + side, side, false);
                    BENCH_SUBPROCESS("pawnShift", "pushmove")
                    pushmove<PROMOTION_MOVE_MASK, side>(o + sh, o, ROOK_BLACK + side, side, false);
                }
            } else {
                BENCH_SUBPROCESS("pawnShift", "pushmove")
                pushmove<STANDARD_MOVE_MASK, side>(o + sh, o, NO_PROMOTION, side, false);
            }
        }
    }

    void clearHeuristic();

    template<uchar side>
    __attribute__((always_inline)) void performDiagShift(const uchar piece, const u64 allpieces) {
        BENCH_AUTO_CLOSE("diagShift")
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)
        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 diag = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & ~allpieces;
            for (; diag; RESET_LSB(diag)) {
                BENCH_SUBPROCESS("diagShift", "pushmove")
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece, false);
            }
        }
    }

    template<uchar side>
    __attribute__((always_inline)) void performRankFileShift(const uchar piece, const u64 allpieces) {
        BENCH_AUTO_CLOSE("rankFileShift")
        ASSERT_RANGE(piece, 0, 11)
        ASSERT_RANGE(side, 0, 1)

        for (u64 x2 = chessboard[piece]; x2; RESET_LSB(x2)) {
            const int position = BITScanForward(x2);
            u64 rankFile = Bitboard::getRankFile(position, allpieces) & ~allpieces;
            for (; rankFile; RESET_LSB(rankFile)) {
                BENCH_SUBPROCESS("rankFileShift", "pushmove")
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece, false);
            }
        }
    }

    bool makemove(const _Tmove *move, const bool rep);

    void incListId() {
        listId++;
        assert(listId >= 0);
        assert(listId < MAX_PLY);
    }

    void decListId() {
        assert(listId > -1);
        genList[listId--].size = 0;
    }

    int getListSize() const {
        return genList[listId].size;
    }

    void pushStackMove() {
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }

    void resetList() {
        genList[listId].size = 0;
    }

    bool generatePuzzle(const string type);

    __attribute__((always_inline))void incHistoryHeuristic(const int from, const int to, const int value) {
        ASSERT_RANGE(from, 0, 63)
        ASSERT_RANGE(to, 0, 63)
        assert(historyHeuristic[from][to] <= historyHeuristic[from][to] + value);
        historyHeuristic[from][to] += value;
    }

    bool perftMode;


#ifndef NDEBUG
    unsigned nCutAB, nNullMoveCut, nCutFp, nCutRazor, nCutBadCaputure;
    double betaEfficiency = 0.0;
    unsigned betaEfficiencyCount = 0;
#endif
    static constexpr uchar STANDARD_MOVE_MASK = 0x3;
protected:
    typedef struct {
        _Tmove *moveList;
        int size;
    } _TmoveP;
    u64 pinned;

    static constexpr u64 RANK_2 = 0xff00ULL;
    static constexpr u64 RANK_3 = 0xff000000ULL;
    static constexpr u64 RANK_5 = 0xff00000000ULL;
    static constexpr u64 RANK_7 = 0xff000000000000ULL;

    static constexpr uchar ENPASSANT_MOVE_MASK = 0x1;
    static constexpr uchar PROMOTION_MOVE_MASK = 0x2;
    static constexpr int MAX_REP_COUNT = 1024;

    int repetitionMapCount;

    u64 *repetitionMap;
    int currentPly;

    u64 numMoves = 0;
    u64 numMovesq = 0;

    _Tmove *getNextMoveQ(_TmoveP *list, const int first);

    _Tmove *getNextMove(_TmoveP *list, const int depth, const u64 &, const int first);

    template<uchar side>
    __attribute__((always_inline)) int getMobilityCastle(const u64 allpieces) const {
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
    unsigned short killer[2][MAX_PLY];

#ifndef NDEBUG

    bool verifyMove(const _Tmove *move);

    template<uchar side, uchar type>
    bool
    __attribute__((always_inline)) inCheckSlow(const int from, const int to, const uchar pieceFrom, const uchar pieceTo, const uchar promotionPiece) {
        bool result;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                assert(pieceFrom != SQUARE_EMPTY);
                assert(pieceTo != KING_BLACK);
                assert(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2(to);
                }
                chessboard[pieceFrom] &= NOTPOW2(from);
                chessboard[pieceFrom] |= POW2(to);
                assert(chessboard[KING_BLACK]);
                assert(chessboard[KING_WHITE]);

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
                chessboard[pieceFrom] &= NOTPOW2(from);
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] &= NOTPOW2(to);
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2(to);
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
                u64 to1 = chessboard[X(side)];
                u64 from1 = chessboard[side];
                chessboard[side] &= NOTPOW2(from);
                chessboard[side] |= POW2(to);
                if (side) {
                    chessboard[X(side)] &= NOTPOW2(to - 8);
                } else {
                    chessboard[X(side)] &= NOTPOW2(to + 8);
                }
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[X(side)] = to1;
                chessboard[side] = from1;
                break;
            }
            default:
                _assert(0)
        }

        return result;
    }

#endif

    template<int type, uchar side>
    __attribute__((always_inline)) bool inCheck(const uchar from, const uchar to, const uchar pieceFrom, const uchar pieceTo, uchar promotionPiece) {
        BENCH_AUTO_CLOSE("inCheck")
#ifndef NDEBUG
        _Tchessboard a;
        memcpy(&a, chessboard, sizeof(_Tchessboard));
        ASSERT_RANGE(from, 0, 63)
        ASSERT_RANGE(to, 0, 63)
        ASSERT_RANGE(side, 0, 1)
        ASSERT_RANGE(pieceFrom, 0, 12)
        ASSERT_RANGE(pieceTo, 0, 12)
        assert(perftMode || forceCheck);
        assert(!(type & 0xc));
#endif
        if (pieceTo == KING_BLACK || pieceTo == KING_WHITE) return false;
        if (perftMode) {
            if ((KING_BLACK + side) != pieceFrom && !isInCheck) {
                if (!(pinned & POW2(from)) || (LINES[from][to] & chessboard[KING_BLACK + side])) {
                    assert(!(inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
                    return false;
                } else {
                    assert((inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
                    return true;
                }
            }
        }

        bool result = 0;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                assert(pieceFrom != SQUARE_EMPTY);
                assert(pieceTo != KING_BLACK);
                assert(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_EMPTY) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2(to);
                }
                chessboard[pieceFrom] &= NOTPOW2(from);
                chessboard[pieceFrom] |= POW2(to);
                assert(chessboard[KING_BLACK]);
                assert(chessboard[KING_WHITE]);

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
                chessboard[pieceFrom] &= NOTPOW2(from);
                if (pieceTo != SQUARE_EMPTY) {
                    chessboard[pieceTo] &= NOTPOW2(to);
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2(to);
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
                u64 to1 = chessboard[X(side)];
                u64 from1 = chessboard[side];
                chessboard[side] &= NOTPOW2(from);
                chessboard[side] |= POW2(to);
                if (side) {
                    chessboard[X(side)] &= NOTPOW2(to - 8);
                } else {
                    chessboard[X(side)] &= NOTPOW2(to + 8);
                }
                result = board::isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                                 board::getBitmap<BLACK>(chessboard) |
                                                 board::getBitmap<WHITE>(chessboard), chessboard);
                chessboard[X(side)] = to1;
                chessboard[side] = from1;
                break;
            }
            default:
                _assert(0)
        }

        assert(!memcmp(&a, chessboard, sizeof(_Tchessboard)));
        return result;
    }

    void performCastle(const uchar side, const uchar type);

    void unPerformCastle(const uchar side, const uchar type);

    template<uchar side>
    __attribute__((always_inline))void tryAllCastle(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1)
        BENCH_AUTO_CLOSE("castle")
        if (chess960)tryAllCastle960<side>(allpieces);
        else tryAllCastleStandard<side>(allpieces);
    }

    __attribute__((always_inline)) bool allowKingSideBlack(const u64 allpieces) const {
        const auto a = board::isCastleRight_BlackKing(rightCastle) &&
                       board::isPieceAt(KING_BLACK, startPosBlackKing, chessboard) &&
                       board::isPieceAt(ROOK_BLACK, startPosBlackRookKingSide, chessboard) &&
                       (!board::isOccupied(G8, allpieces) || startPosBlackKing == G8 ||
                        startPosBlackRookKingSide == G8) &&
                       (!board::isOccupied(F8, allpieces) || startPosBlackKing == F8 ||
                        startPosBlackRookKingSide == F8) &&
                       !(startPosBlackKing == G8 && startPosBlackRookKingSide == F8);
        if (!a)return a;
        const u64 path = LINK_SQUARE[startPosBlackKing][G8];
        const u64 rookPath = LINK_SQUARE[startPosBlackRookKingSide][F8] & NOTPOW2(startPosBlackKing);
        const u64 kingPath = path | POW2(G8) | POW2(startPosBlackKing);
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2(startPosBlackRookKingSide))) &&
                !board::anyAttack<BLACK>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<BLACK>(kingPath, allpieces & NOTPOW2(startPosBlackRookKingSide), chessboard));
    }

    __attribute__((always_inline))bool allowQueenSideBlack(const u64 allpieces) const {
        auto a = board::isCastleRight_BlackQueen(rightCastle) &&
                 board::isPieceAt(KING_BLACK, startPosBlackKing, chessboard) &&
                 board::isPieceAt(ROOK_BLACK, startPosBlackRookQueenSide, chessboard) &&
                 (!board::isOccupied(C8, allpieces) || startPosBlackKing == C8 || startPosBlackRookQueenSide == C8) &&
                 (!board::isOccupied(D8, allpieces) || startPosBlackKing == D8 || startPosBlackRookQueenSide == D8) &&
                 !(startPosBlackKing == C8 && startPosBlackRookQueenSide == D8);
        if (!a)return false;
        const u64 rookPath = LINK_SQUARE[startPosBlackRookQueenSide][D8] & NOTPOW2(startPosBlackKing);
        const u64 path = LINK_SQUARE[startPosBlackKing][C8];
        const u64 kingPath = path | POW2(C8) | POW2(startPosBlackKing);
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2(startPosBlackRookQueenSide))) &&
                !board::anyAttack<BLACK>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<BLACK>(kingPath, allpieces & NOTPOW2(startPosBlackRookQueenSide),
                                         chessboard));
    }

    bool allowCastleBlackQueen(const u64 allpieces) const;

    bool allowCastleWhiteQueen(const u64 allpieces) const;

    bool allowCastleBlackKing(const u64 allpieces) const;

    bool allowCastleWhiteKing(const u64 allpieces) const;

    bool allowQueenSideWhite(const u64 allpieces) const {
        const auto a = board::isCastleRight_WhiteQueen(rightCastle) &&
                       board::isPieceAt(KING_WHITE, startPosWhiteKing, chessboard) &&
                       board::isPieceAt(ROOK_WHITE, startPosWhiteRookQueenSide, chessboard) &&
                       (!board::isOccupied(C1, allpieces) || startPosWhiteKing == C1 ||
                        startPosWhiteRookQueenSide == C1) &&
                       (!board::isOccupied(D1, allpieces) || startPosWhiteKing == D1 ||
                        startPosWhiteRookQueenSide == D1) &&
                       !(startPosWhiteKing == C1 && startPosWhiteRookQueenSide == D1);
        if (!a)return false;
        const u64 path = LINK_SQUARE[startPosWhiteKing][C1];
        const u64 rookPath = LINK_SQUARE[startPosWhiteRookQueenSide][D1] & NOTPOW2(startPosWhiteKing);
        const u64 kingPath = path | POW2(C1) | POW2(startPosWhiteKing);
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2(startPosWhiteRookQueenSide))) &&
                !board::anyAttack<WHITE>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<WHITE>(kingPath, allpieces & NOTPOW2(startPosWhiteRookQueenSide), chessboard));
    }

    __attribute__((always_inline))bool allowKingSideWhite(const u64 allpieces) const {
        const auto a = board::isCastleRight_WhiteKing(rightCastle) &&
                       board::isPieceAt(KING_WHITE, startPosWhiteKing, chessboard) &&
                       board::isPieceAt(ROOK_WHITE, startPosWhiteRookKingSide, chessboard) &&
                       (!board::isOccupied(G1, allpieces) || startPosWhiteKing == G1 ||
                        startPosWhiteRookKingSide == G1) &&
                       (!board::isOccupied(F1, allpieces) || startPosWhiteKing == F1 ||
                        startPosWhiteRookKingSide == F1) &&
                       !(startPosWhiteKing == G1 && startPosWhiteRookKingSide == F1);
        if (!a)return a;
        const u64 path = LINK_SQUARE[startPosWhiteKing][G1];
        const u64 rookPath = LINK_SQUARE[startPosWhiteRookKingSide][F1] & NOTPOW2(startPosWhiteKing);
        const u64 kingPath = path | POW2(G1) | POW2(startPosWhiteKing);
        return (!(allpieces & rookPath) && !(allpieces & (path & NOTPOW2(startPosWhiteRookKingSide))) &&
                !board::anyAttack<WHITE>(kingPath, allpieces, chessboard) &&
                !board::anyAttack<WHITE>(kingPath, allpieces & NOTPOW2(startPosWhiteRookKingSide), chessboard));
    }

    template<uchar side>
    __attribute__((always_inline)) void tryAllCastle960(const u64 allpieces) {
        if (side == WHITE) {
            if (allowKingSideWhite(allpieces)) {
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            }
            if (allowQueenSideWhite(allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            }

        } else {
            if (allowKingSideBlack(allpieces)) {
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            }
            if (allowQueenSideBlack(allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            }
        }
    }

    template<uchar side>
    __attribute__((always_inline)) void tryAllCastleStandard(const u64 allpieces) {
        if (side == WHITE) {
            if (allowCastleWhiteKing(allpieces))
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            if (allowCastleWhiteQueen(allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            }
        } else {
            if (allowCastleBlackKing(allpieces))
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
            if (allowCastleBlackQueen(allpieces))
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, side>(NO_POSITION, NO_POSITION, NO_PROMOTION, NO_PIECE, false);
        }
    }

    template<uchar type, uchar side>
    bool
    __attribute__((always_inline)) pushmove(const uchar from, const uchar to, const uchar promotionPiece, const uchar pieceFrom,
             const bool isCapture) {
        BENCH_AUTO_CLOSE("pushmove")
        assert(chessboard[KING_BLACK]);
        assert(chessboard[KING_WHITE]);
        uchar capturedPiece = SQUARE_EMPTY;
        bool res = false;
        if (((type & 0x3) != ENPASSANT_MOVE_MASK) && !(type & 0xc)) {
            if (isCapture) {
                capturedPiece = board::getPieceAt<X(side)>(POW2(to), chessboard);
                if (capturedPiece == KING_BLACK + (X(side))) {
                    res = true;
                }
            }
        } else if (!(type & 0xc)) {//en passant
            capturedPiece = X(side);
        }
        if (!(type & 0xc) && (forceCheck || perftMode)) {
            BENCH_SUBPROCESS("pushmove", "inCheck")
            if (inCheck<type, side>(from, to, pieceFrom, capturedPiece, promotionPiece)) return false;
        }
        ASSERT_RANGE(listId, 0, MAX_PLY - 1)
        assert(getListSize() < MAX_MOVE);
        auto move = &genList[listId].moveList[getListSize()];
        ++genList[listId].size;
        move->type = rightCastle | type;
        move->side = side;
        move->capturedPiece = capturedPiece;
        if (type & 0x3) {
            move->from = (uchar) from;
            move->to = (uchar) to;
            move->pieceFrom = pieceFrom;
            move->promotionPiece = (char) promotionPiece;
        }
        assert(getListSize() < MAX_MOVE);
        return res;
    }

    _Tmove *getMove(const int i) const {
        return &genList[listId].moveList[i];
    }

    virtual void setRunning(const int t) {
        running = t;
    }

    virtual int getRunning() const {
        return running;
    }

    __attribute__((always_inline)) void setHistoryHeuristic(const int from, const int to, const int depth) {
        ASSERT_RANGE(from, 0, 63)
        ASSERT_RANGE(to, 0, 63)
        if (depth < 0)return;
        const int value = (depth < 30) ? 2 << depth : 0x40000000;
        historyHeuristic[from][to] = value;
    }

    __attribute__((always_inline)) void setKiller(const int from, const int to, const int depth) {
        ASSERT_RANGE(from, 0, 63)
        ASSERT_RANGE(to, 0, 63)
        ASSERT_RANGE(depth, 0, MAX_PLY - 1)
        killer[1][depth] = killer[0][depth];
        killer[0][depth] = from | (to << 8);
    }

    bool isKiller(const int idx, const int from, const int to, const int depth) {
        ASSERT_RANGE(from, 0, 63)
        ASSERT_RANGE(to, 0, 63)
        ASSERT_RANGE(depth, 0, MAX_PLY - 1)
        const unsigned short v = from | (to << 8);
        return v == killer[idx][depth];
    }

    bool forceCheck;
public:
    _TmoveP *genList;
    int listId;

private:

    int running;
    bool isInCheck;
    static constexpr u64 TABJUMPPAWN = 0xFF00000000FF00ULL;

    void writeRandomFen(const vector<int>);

    _Tmove *swap(_TmoveP *list, const int i, const int j) {
        std::swap(list->moveList[i], list->moveList[j]);
        return &list->moveList[i];
    }

    template<uchar side>
    __attribute__((always_inline)) void performJumpPawn(u64 x, const u64 xallpieces) {
        BENCH_AUTO_CLOSE("performJumpPawn")
        x &= TABJUMPPAWN;
        if (!x) return;

        if (side) {
            x = (((x << 8) & xallpieces) << 8) & xallpieces;
        } else {
            x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
        }
        for (; x; RESET_LSB(x)) {
            const int o = BITScanForward(x);
            BENCH_SUBPROCESS("performJumpPawn", "pushmove")
            pushmove<STANDARD_MOVE_MASK, side>(o + (side ? -16 : 16), o, NO_PROMOTION, side, false);
        }
    }

    __attribute__((always_inline))  void popStackMove() {
        ASSERT_RANGE(repetitionMapCount, 1, MAX_REP_COUNT - 1)
        if (--repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0) {
            repetitionMapCount--;
        }
    }

    __attribute__((always_inline)) void pushStackMove(const u64 key) {
        assert(repetitionMapCount < MAX_REP_COUNT - 1);
        repetitionMap[repetitionMapCount++] = key;
    }

};

