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
#include "util/Bitboard.h"
#include <vector>

class GenMoves : public ChessBoard {

public:
    static const int MAX_MOVE = 130;

    GenMoves();

    virtual ~GenMoves();

    void setPerft(const bool b);

    bool generateCaptures(const int side, u64, u64);

    bool generateCapturesMoves();

    void generateMoves(const int side, const u64);

    template<int side>
    void generateMoves(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        tryAllCastle(side, allpieces);
        performDiagShift(BISHOP_BLACK + side, side, allpieces);
        performRankFileShift(ROOK_BLACK + side, side, allpieces);
        performRankFileShift(QUEEN_BLACK + side, side, allpieces);
        performDiagShift(QUEEN_BLACK + side, side, allpieces);
        performPawnShift<side>(~allpieces);
        performKnightShiftCapture(KNIGHT_BLACK + side, ~allpieces, side);
        performKingShiftCapture(side, ~allpieces);
    }

    template<int side>
    bool generateCaptures(const u64 enemies, const u64 friends) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        const u64 allpieces = enemies | friends;

        if (perftMode) {
            int kingPosition = BITScanForward(chessboard[KING_BLACK + side]);
            pinned = getPinned<side>(allpieces, friends, kingPosition);
            isInCheck = isAttacked<side>(kingPosition, allpieces);
        }

        if (performPawnCapture<side>(enemies)) {
            return true;
        }
        if (performKingShiftCapture(side, enemies)) {
            return true;
        }
        if (performKnightShiftCapture(KNIGHT_BLACK + side, enemies, side)) {
            return true;
        }
        if (performDiagCapture(BISHOP_BLACK + side, enemies, side, allpieces)) {
            return true;
        }
        if (performRankFileCapture(ROOK_BLACK + side, enemies, side, allpieces)) {
            return true;
        }
        if (performRankFileCapture(QUEEN_BLACK + side, enemies, side, allpieces)) {
            return true;
        }
        if (performDiagCapture(QUEEN_BLACK + side, enemies, side, allpieces)) {
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

    u64 getDiagShiftAndCapture(const int position, const u64 enemies, const u64 allpieces) const {
        ASSERT_RANGE(position, 0, 63);
        u64 nuovo = Bitboard::getDiagonalAntiDiagonal(position, allpieces);
        return (nuovo & enemies) | (nuovo & ~allpieces);
    }

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setRepetitionMapCount(const int i);

    inline int getDiagShiftCount(const int position, const u64 allpieces) const {
        ASSERT_RANGE(position, 0, 63);
        return bitCount(Bitboard::getDiagonalAntiDiagonal(position, allpieces) & ~allpieces);
    }

    bool performKingShiftCapture(const int side, const u64 enemies);

    bool performKnightShiftCapture(const int piece, const u64 enemies, const int side);

    bool performDiagCapture(const int piece, const u64 enemies, const int side, const u64 allpieces);

    u64 getTotMoves() const;

    bool performRankFileCapture(const int piece, const u64 enemies, const int side, const u64 allpieces);

    template<int side>
    bool performPawnCapture(const u64 enemies) {
        if (!chessboard[side]) {
            if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
                updateZobristKey(13, chessboard[ENPASSANT_IDX]);
            }
            chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
            return false;
        }
        constexpr int sh = side ? -7 : 7;

        u64 x = shiftForward<side, 7>(chessboard[side]) & enemies;

        while (x) {
            const int o = BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {//PROMOTION
                if (pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, QUEEN_BLACK + side, side)) {
                    return true;        //queen
                }
                if (perftMode) {
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, KNIGHT_BLACK + side, side)) {
                        return true;        //knight
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, ROOK_BLACK + side, side)) {
                        return true;        //rock
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, BISHOP_BLACK + side, side)) {
                        return true;        //bishop
                    }
                }
            } else if (pushmove<STANDARD_MOVE_MASK>(o + sh, o, side, NO_PROMOTION, side)) {
                return true;
            }
            RESET_LSB(x);
        }
        constexpr int sh2 = side ? -9 : 9;
        x = shiftForward<side, 9>(chessboard[side]) & enemies;

        while (x) {
            const int o = BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {    //PROMOTION
                if (pushmove<PROMOTION_MOVE_MASK>(o + sh2, o, side, QUEEN_BLACK + side, side)) {
                    return true;        //queen
                }
                if (perftMode) {
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh2, o, side, KNIGHT_BLACK + side, side)) {
                        return true;        //knight
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh2, o, side, BISHOP_BLACK + side, side)) {
                        return true;        //bishop
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + sh2, o, side, ROOK_BLACK + side, side)) {
                        return true;        //rock
                    }
                }
            } else if (pushmove<STANDARD_MOVE_MASK>(o + sh2, o, side, NO_PROMOTION, side)) {
                return true;
            }
            RESET_LSB(x);
        }
        //ENPASSANT
        if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
            x = ENPASSANT_MASK[side ^ 1][chessboard[ENPASSANT_IDX]] & chessboard[side];
            while (x) {
                const int o = BITScanForward(x);
                pushmove<ENPASSANT_MOVE_MASK>(o, (side ? chessboard[ENPASSANT_IDX] + 8 : chessboard[ENPASSANT_IDX] - 8),
                                              side, NO_PROMOTION, side);
                RESET_LSB(x);
            }
            updateZobristKey(13, chessboard[ENPASSANT_IDX]);
            chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
        }
        return false;
    }


    template<int side>
    void performPawnShift(const u64 xallpieces) {

        u64 x = chessboard[side];
        if (x & PAWNS_JUMP[side]) {
            checkJumpPawn<side>(x, xallpieces);
        }
        constexpr int sh = side ? -8 : 8;
        x = side ? x << 8 : x >> 8;

        x &= xallpieces;
        while (x) {
            const int o = BITScanForward(x);
            ASSERT(getPieceAt(side, POW2[o + sh]) != SQUARE_FREE);
            ASSERT(getBitmap(side) & POW2[o + sh]);
            if (o > 55 || o < 8) {
                pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, QUEEN_BLACK + side, side);
                if (perftMode) {
                    pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, KNIGHT_BLACK + side, side);
                    pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, BISHOP_BLACK + side, side);
                    pushmove<PROMOTION_MOVE_MASK>(o + sh, o, side, ROOK_BLACK + side, side);
                }
            } else {
                pushmove<STANDARD_MOVE_MASK>(o + sh, o, side, NO_PROMOTION, side);
            }
            RESET_LSB(x);
        }
    }

    void clearKillerHeuristic();

    void performDiagShift(const int piece, const int side, const u64 allpieces);

    void performRankFileShift(const int piece, const int side, const u64 allpieces);

    bool makemove(const _Tmove *move, const bool rep = true, const bool = false);

    void incListId() {
        listId++;
#ifdef DEBUG_MODE
        if (listId < 0 || listId >= MAX_PLY) {
            display();
        }
        ASSERT_RANGE(listId, 0, MAX_PLY - 1);
#endif
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

    void incKillerHeuristic(const int from, const int to, const int value) {
        if (!getRunning()) {
            return;
        }
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        ASSERT(killerHeuristic[from][to] <= killerHeuristic[from][to] + value);
        killerHeuristic[from][to] += value;
    }

    _Tmove *getNextMove();

    template<int side>
    u64 getPinned(const u64 allpieces, const u64 friends, const int kingPosition) const {
        u64 result = 0;
        const u64 *s = LINK_SQUARE[kingPosition];
        constexpr int xside = side ^1;
        u64 attacked = DIAGONAL_ANTIDIAGONAL[kingPosition] &
                       (chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside]);
        attacked |=
                RANK_FILE[kingPosition] & (chessboard[QUEEN_BLACK + xside] | chessboard[ROOK_BLACK + xside]);
        while (attacked) {
            const int pos = BITScanForward(attacked);
            const u64 b = *(s + pos) & allpieces;
#ifdef DEBUG_MODE
            u64 x = *(s + pos) & (allpieces & NOTPOW2[kingPosition]);
            ASSERT(b == x);
#endif
            if (!(b & (b - 1))) {
                result |= b & friends;
            }
            RESET_LSB(attacked);
        }
        return result;
    }

#ifdef DEBUG_MODE
    unsigned nCutAB, nNullMoveCut, nCutFp, nCutRazor;
    double betaEfficiency;
#endif
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
    static constexpr int NO_PROMOTION = -1;
    int repetitionMapCount;

    u64 *repetitionMap;
    int currentPly;

    u64 numMoves = 0;
    u64 numMovesq = 0;

    _Tmove *getNextMove(decltype(gen_list));

    template<int side>
    bool isAttacked(const int position, const u64 allpieces) const {
        return getAttackers<side, true>(position, allpieces);
    }

    template<int side>
    u64 getAllAttackers(const int position, const u64 allpieces) const {
        return getAttackers<side, false>(position, allpieces);
    }

    int getMobilityRook(const int position, const u64 enemies, const u64 friends);

    int getMobilityCastle(const int side, const u64 allpieces) const;

    int getMobilityQueen(const int position, const u64 enemies, const u64 allpieces);

    void initKillerHeuristic();

    void pushRepetition(u64);

    int killerHeuristic[64][64];

#ifdef DEBUG_MODE

    template<int side, uchar type>
    bool inCheckSlow(const int from, const int to, const int pieceFrom, const int pieceTo, const int promotionPiece) {
        bool result = false;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_FREE);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2[to];
                };
                chessboard[pieceFrom] &= NOTPOW2[from];
                chessboard[pieceFrom] |= POW2[to];
                ASSERT(chessboard[KING_BLACK]);
                ASSERT(chessboard[KING_WHITE]);

                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard[pieceFrom] = from1;
                if (pieceTo != SQUARE_FREE) {
                    chessboard[pieceTo] = to1;
                };
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard[pieceTo];
                }
                u64 from1 = chessboard[pieceFrom];
                u64 p1 = chessboard[promotionPiece];
                chessboard[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_FREE) {
                    chessboard[pieceTo] &= NOTPOW2[to];
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2[to];
                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
                if (pieceTo != SQUARE_FREE) {
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
                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
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

    template<int side, uchar type>
    bool inCheck(const int from, const int to, const int pieceFrom, const int pieceTo, int promotionPiece) {
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
                    return false;
                } else {
                    ASSERT ((inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
                    return true;
                }
            }

        }
//#ifdef DEBUG_MODE
//        _Tchessboard a;
//        memcpy(&a, chessboard, sizeof(_Tchessboard));
//#endif
        bool result = 0;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_FREE);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard[pieceFrom];
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard[pieceTo];
                    chessboard[pieceTo] &= NOTPOW2[to];
                };
                chessboard[pieceFrom] &= NOTPOW2[from];
                chessboard[pieceFrom] |= POW2[to];
                ASSERT(chessboard[KING_BLACK]);
                ASSERT(chessboard[KING_WHITE]);

                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard[pieceFrom] = from1;
                if (pieceTo != SQUARE_FREE) {
                    chessboard[pieceTo] = to1;
                };
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard[pieceTo];
                }
                u64 from1 = chessboard[pieceFrom];
                u64 p1 = chessboard[promotionPiece];
                chessboard[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_FREE) {
                    chessboard[pieceTo] &= NOTPOW2[to];
                }
                chessboard[promotionPiece] = chessboard[promotionPiece] | POW2[to];
                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
                if (pieceTo != SQUARE_FREE) {
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
                result = isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]),
                                          getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard[side ^ 1] = to1;
                chessboard[side] = from1;;
                break;
            }
            default:
            _assert(0);
        }

#ifdef DEBUG_MODE
        ASSERT(!memcmp(&a, chessboard, sizeof(_Tchessboard)));
#endif
        return result;
    }

    void performCastle(const int side, const uchar type);

    void unPerformCastle(const int side, const uchar type);

    void tryAllCastle(const int side, const u64 allpieces);


    template<uchar type>
    bool pushmove(const int from, const int to, const int side, const int promotionPiece, const int pieceFrom) {
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
        int piece_captured = SQUARE_FREE;
        bool res = false;
        if (((type & 0x3) != ENPASSANT_MOVE_MASK) && !(type & 0xc)) {
            piece_captured = side ? getPieceAt<BLACK>(POW2[to]) : getPieceAt<WHITE>(POW2[to]);
            if (piece_captured == KING_BLACK + (side ^ 1)) {
                res = true;
            }
        } else if (!(type & 0xc)) {//no castle
            piece_captured = side ^ 1;
        }
        if (!(type & 0xc) && (forceCheck || perftMode)) {//no castle
            if (side == WHITE && inCheck<WHITE, type>(from, to, pieceFrom, piece_captured, promotionPiece)) {
                return false;
            }
            if (side == BLACK && inCheck<BLACK, type>(from, to, pieceFrom, piece_captured, promotionPiece)) {
                return false;
            }
        }
        _Tmove *mos;
        ASSERT_RANGE(listId, 0, MAX_PLY - 1);

        ASSERT(getListSize() < MAX_MOVE);
        mos = &gen_list[listId].moveList[getListSize()];
        ++gen_list[listId].size;
        mos->type = (uchar) chessboard[RIGHT_CASTLE_IDX] | type;
        mos->side = (char) side;
        mos->capturedPiece = piece_captured;
        if (type & 0x3) {
            mos->from = (uchar) from;
            mos->to = (uchar) to;
            mos->pieceFrom = pieceFrom;
            mos->promotionPiece = (char) promotionPiece;
            if (!perftMode) {
                if (res == true) {
                    mos->score = _INFINITE;
                } else {
                    ASSERT_RANGE(pieceFrom, 0, 11);
                    ASSERT_RANGE(to, 0, 63);
                    ASSERT_RANGE(from, 0, 63);
                    mos->score = killerHeuristic[from][to];
                    mos->score += (PIECES_VALUE[piece_captured] >= PIECES_VALUE[pieceFrom]) ?
                                  (PIECES_VALUE[piece_captured] - PIECES_VALUE[pieceFrom]) * 2
                                                                                            : PIECES_VALUE[piece_captured];
                    //mos->score += (MOV_ORD[pieceFrom][to] - MOV_ORD[pieceFrom][from]);
                }
            }
        } else if (type & 0xc) {    //castle
            ASSERT(chessboard[RIGHT_CASTLE_IDX]);
            mos->score = 100;
        }
        mos->used = false;
        ASSERT(getListSize() < MAX_MOVE);
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

    template<int side>
    bool inCheck() const {
        return isAttacked<side>(BITScanForward(chessboard[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
    }

    void setKillerHeuristic(const int from, const int to, const int value) {
        if (getRunning()) {
            ASSERT_RANGE(from, 0, 63);
            ASSERT_RANGE(to, 0, 63);
            killerHeuristic[from][to] = value;
        }
    }


private:
    int running;
    bool isInCheck;
    static bool forceCheck;
    static constexpr u64 TABJUMPPAWN = 0xFF00000000FF00ULL;

    void writeRandomFen(const vector<int>);

    template<int side>
    void checkJumpPawn(u64 x, const u64 xallpieces) {
        x &= TABJUMPPAWN;
        if (side) {
            x = (((x << 8) & xallpieces) << 8) & xallpieces;
        } else {
            x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
        }
        while (x) {
            const int o = BITScanForward(x);
            pushmove<STANDARD_MOVE_MASK>(o + (side ? -16 : 16), o, side, NO_PROMOTION, side);
            RESET_LSB(x);
        }
    }

    int performRankFileCaptureAndShiftCount(const int position, const u64 enemies, const u64 allpieces);

    void popStackMove() {
        ASSERT(repetitionMapCount > 0);
        if (--repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0) {
            repetitionMapCount--;
        }
    }

    void pushStackMove(const u64 key) {
        ASSERT(repetitionMapCount < MAX_REP_COUNT - 1);
        repetitionMap[repetitionMapCount++] = key;
    }

    template<int side, bool exitOnFirst>
    u64 getAttackers(const int position, const u64 allpieces) const {
        ASSERT_RANGE(position, 0, 63);
        ASSERT_RANGE(side, 0, 1);

        ///knight
        u64 attackers = KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///king
        attackers |= NEAR_MASK1[position] & chessboard[KING_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///pawn
        attackers |= PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///bishop queen
        u64 enemies = chessboard[BISHOP_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        u64 nuovo = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
        while (nuovo) {
            const int bound = BITScanForward(nuovo);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers)return 1;
            RESET_LSB(nuovo);
        }
        enemies = chessboard[ROOK_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        nuovo = Bitboard::getRankFile(position, allpieces) & enemies;
        while (nuovo) {
            const int bound = BITScanForward(nuovo);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers)return 1;
            RESET_LSB(nuovo);
        }
        return attackers;
    }

};

