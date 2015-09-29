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

#include "namespaces/eval.h"


class GenMoves : public virtual ChessBoard {

public:
    static const int MAX_MOVE = 130;

    GenMoves();

    virtual ~GenMoves();

    void setPerft(const bool b);

    bool generateCaptures(const int side, u64, u64);

    bool generateCapturesMoves();

    void generateMoves(const int side, const u64);

    void generateMoves(const int side);

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
        u64 allpieces = enemies | friends;
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

    bool getForceCheck() {
        return forceCheck;
    }

    void setForceCheck(bool b) {
        forceCheck = b;
    }

    int getMoveFromSan(const string fenStr, _Tmove *move);

    void init();

    virtual int loadFen(string fen = "");

    u64 performDiagCaptureCount(const int, const u64 allpieces);

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setRepetitionMapCount(int i);

    int performDiagShiftCount(const int, const u64 allpieces);

    bool performKingShiftCapture(int side, const u64 enemies);

    bool performKnightShiftCapture(const int piece, const u64 enemies, const int side);

    bool performDiagCapture(const int piece, const u64 enemies, const int side, const u64 allpieces);

    u64 getTotMoves();

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
        int GG;
        u64 x;
        if (side) {
            x = (chessboard[side] << 7) & TABCAPTUREPAWN_LEFT & enemies;
            GG = -7;
        } else {
            x = (chessboard[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies;
            GG = 7;
        };
        while (x) {
            int o = Bits::BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {//PROMOTION
                if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, QUEEN_BLACK + side, side)) {
                    return true;        //queen
                }
                if (perftMode) {
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, KNIGHT_BLACK + side, side)) {
                        return true;        //knight
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, ROOK_BLACK + side, side)) {
                        return true;        //rock
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, BISHOP_BLACK + side, side)) {
                        return true;        //bishop
                    }
                }
            } else if (pushmove<STANDARD_MOVE_MASK>(o + GG, o, side, NO_PROMOTION, side)) {
                return true;
            }
            x &= NOTPOW2[o];
        };
        if (side) {
            GG = -9;
            x = (chessboard[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies;
        } else {
            GG = 9;
            x = (chessboard[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies;
        };
        while (x) {
            int o = Bits::BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {    //PROMOTION
                if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, QUEEN_BLACK + side, side)) {
                    return true;        //queen
                }
                if (perftMode) {
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, KNIGHT_BLACK + side, side)) {
                        return true;        //knight
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, BISHOP_BLACK + side, side)) {
                        return true;        //bishop
                    }
                    if (pushmove<PROMOTION_MOVE_MASK>(o + GG, o, side, ROOK_BLACK + side, side)) {
                        return true;        //rock
                    }
                }
            } else if (pushmove<STANDARD_MOVE_MASK>(o + GG, o, side, NO_PROMOTION, side)) {
                return true;
            }
            x &= NOTPOW2[o];
        };
        //ENPASSANT
        if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
            x = ENPASSANT_MASK[side ^ 1][chessboard[ENPASSANT_IDX]] & chessboard[side];
            while (x) {
                int o = Bits::BITScanForward(x);
                pushmove<ENPASSANT_MOVE_MASK>(o, (side ? chessboard[ENPASSANT_IDX] + 8 : chessboard[ENPASSANT_IDX] - 8), side, NO_PROMOTION, side);
                x &= NOTPOW2[o];
            }
            updateZobristKey(13, chessboard[ENPASSANT_IDX]);
            chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
        }
        return false;
    }


    template<int side>
    void performPawnShift(const u64 xallpieces) {
        int tt;
        u64 x = chessboard[side];
        if (x & PAWNS_JUMP[side]) {
            checkJumpPawn<side>(x, xallpieces);
        }
        if (side) {
            x <<= 8;
            tt = -8;
        } else {
            tt = 8;
            x >>= 8;
        };
        x &= xallpieces;
        while (x) {
            int o = Bits::BITScanForward(x);
            ASSERT(getPieceAt(side, POW2[o + tt]) != SQUARE_FREE);
            ASSERT(getBitBoard(side) & POW2[o + tt]);
            if (o > 55 || o < 8) {
                pushmove<PROMOTION_MOVE_MASK>(o + tt, o, side, QUEEN_BLACK + side, side);
                if (perftMode) {
                    pushmove<PROMOTION_MOVE_MASK>(o + tt, o, side, KNIGHT_BLACK + side, side);
                    pushmove<PROMOTION_MOVE_MASK>(o + tt, o, side, BISHOP_BLACK + side, side);
                    pushmove<PROMOTION_MOVE_MASK>(o + tt, o, side, ROOK_BLACK + side, side);
                }
            } else {
                pushmove<STANDARD_MOVE_MASK>(o + tt, o, side, NO_PROMOTION, side);
            }
            x &= NOTPOW2[o];
        };
    }

    void clearKillerHeuristic();

    int performPawnShiftCount(int side, const u64 xallpieces);

    void performDiagShift(const int piece, const int side, const u64 allpieces);

    void performRankFileShift(const int piece, const int side, const u64 allpieces);

    bool makemove(_Tmove *move, bool rep = true, bool = false);

    //bool isPinned(const int side, const uchar Position, const uchar piece);
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

    int getListSize() {
        return gen_list[listId].size;
    }

    void pushStackMove() {
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }

    void resetList() {
        gen_list[listId].size = 0;
    }

    void generatePuzzle(const string type);

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

#ifdef DEBUG_MODE
    unsigned nCutAB, nNullMoveCut, nCutFp, nCutRazor;
    double betaEfficiency;
#endif
protected:
    bool perftMode;
    int listId;
    _TmoveP *gen_list;
    static const u64 RANK_1 = 0xff00ULL;
    static const u64 RANK_3 = 0xff000000ULL;
    static const u64 RANK_4 = 0xff00000000ULL;
    static const u64 RANK_6 = 0xff000000000000ULL;
    static const uchar STANDARD_MOVE_MASK = 0x3;
    static const uchar ENPASSANT_MOVE_MASK = 0x1;
    static const uchar PROMOTION_MOVE_MASK = 0x2;
    static const int MAX_REP_COUNT = 1024;
    static const int NO_PROMOTION = -1;
    int repetitionMapCount;

    u64 *repetitionMap;
    int currentPly;

    u64 numMoves = 0;
    u64 numMovesq = 0;

    _Tmove *getNextMove(decltype(gen_list));

    template<int side>
    bool isAttacked(const int position, const u64 allpieces) const {
        u64 a = getAttackers<side, true>(position, allpieces);//TODO se modifica verificare ./cinnamon -perft -f"8/PPP4k/8/8/8/8/4Kppp/8 w - - 0 1" -h4000 -c2 -d8
        if (a == 0)return false;
        return true;
    }

    template<int side>
    u64 getAllAttackers(int position, u64 allpieces) const {
        return getAttackers<side, false>(position, allpieces);
    }

    int getMobilityRook(const int position, const u64 enemies, const u64 friends);

    int getMobilityPawns(const int side, const int ep, const u64 ped_friends, const u64 enemies, const u64 xallpieces);

    int getMobilityCastle(const int side, const u64 allpieces);

    int getMobilityQueen(const int position, const u64 enemies, const u64 friends);

    void initKillerHeuristic();

    void pushRepetition(u64);

    int killerHeuristic[64][64];

    template<int side>
    bool inCheck(const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, int promotionPiece);

    void performCastle(const int side, const uchar type);

    void unPerformCastle(const int side, const uchar type);

    void tryAllCastle(const int side, const u64 allpieces);


    template<uchar type>
    bool pushmove(const int from, const int to, const int side, int promotionPiece, int pieceFrom) {
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
            if (side == WHITE && inCheck<WHITE>(from, to, type, pieceFrom, piece_captured, promotionPiece)) {
                return false;
            }
            if (side == BLACK && inCheck<BLACK>(from, to, type, pieceFrom, piece_captured, promotionPiece)) {
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
                    mos->score += (_eval::PIECES_VALUE[piece_captured] >= _eval::PIECES_VALUE[pieceFrom]) ? (_eval::PIECES_VALUE[piece_captured] - _eval::PIECES_VALUE[pieceFrom]) * 2 : _eval::PIECES_VALUE[piece_captured];

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

    _Tmove *getMove(int i) const {
        return &gen_list[listId].moveList[i];
    }

    void setRunning(int t) {
        running = t;
    }

    int getRunning() const {
        return running;
    }

    template<int side>
    bool inCheck() const{
        return isAttacked<side>(Bits::BITScanForward(chessboard[KING_BLACK + side]), getBitBoard<BLACK>() | getBitBoard<WHITE>());
    }

    void setKillerHeuristic(const int from, const int to, const int value) {
        if (!getRunning()) {
            return;
        }
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        killerHeuristic[from][to] = value;
    }


private:
    int running;
    static bool forceCheck;
    static const u64 TABJUMPPAWN = 0xFF00000000FF00ULL;
    static const u64 TABCAPTUREPAWN_RIGHT = 0xFEFEFEFEFEFEFEFEULL;
    static const u64 TABCAPTUREPAWN_LEFT = 0x7F7F7F7F7F7F7F7FULL;

    void writeFen(const vector<int>);

    template<int side>
    void checkJumpPawn(u64 x, const u64 xallpieces) {
        x &= TABJUMPPAWN;
        if (side) {
            x = (((x << 8) & xallpieces) << 8) & xallpieces;
        } else {
            x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
        };
        while (x) {
            int o = Bits::BITScanForward(x);
            pushmove<STANDARD_MOVE_MASK>(o + (side ? -16 : 16), o, side, NO_PROMOTION, side);
            x &= NOTPOW2[o];
        };
    }

    int performRankFileCaptureCount(const int, const u64 enemies, const u64 allpieces);

    int performRankFileShiftCount(const int piece, const u64 allpieces);

    void popStackMove() {
        ASSERT(repetitionMapCount > 0);
        if (--repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0) {
            repetitionMapCount--;
        }
    }

    void pushStackMove(u64 key) {
        ASSERT(repetitionMapCount < MAX_REP_COUNT - 1);
        repetitionMap[repetitionMapCount++] = key;
    }

    template<int side, bool getBoolean>
    u64 getAttackers(const int position, const u64 allpieces) const {
        ASSERT_RANGE(position, 0, 63);
        ASSERT_RANGE(side, 0, 1);
        int bound;
        ///knight
        u64 attackers = KNIGHT_MASK[position] & chessboard[KNIGHT_BLACK + (side ^ 1)];
        if (getBoolean && attackers)return 1;
        ///king
        attackers |= NEAR_MASK1[position] & chessboard[KING_BLACK + (side ^ 1)];
        if (getBoolean && attackers)return 1;
        ///pawn
        attackers |= PAWN_FORK_MASK[side][position] & chessboard[PAWN_BLACK + (side ^ 1)];
        if (getBoolean && attackers)return 1;
        ///bishop queen
        u64 enemies = chessboard[BISHOP_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        if (LEFT_RIGHT_DIAG[position] & enemies) {
            ///LEFT
            u64 q = allpieces & MASK_BIT_UNSET_LEFT_UP[position];
            if (q) {
                bound = Bits::BITScanReverse(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
            q = allpieces & MASK_BIT_UNSET_LEFT_DOWN[position];
            if (q) {
                bound = Bits::BITScanForward(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
            ///RIGHT
            q = allpieces & MASK_BIT_UNSET_RIGHT_UP[position];
            if (q) {
                bound = Bits::BITScanReverse(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
            q = allpieces & MASK_BIT_UNSET_RIGHT_DOWN[position];
            if (q) {
                bound = Bits::BITScanForward(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
        }
        enemies = chessboard[ROOK_BLACK + (side ^ 1)] | chessboard[QUEEN_BLACK + (side ^ 1)];
        u64 q;
        ///rook queen
        u64 x = allpieces & FILE_[position];
        if (x & enemies) {
            q = x & MASK_BIT_UNSET_UP[position];
            if (q) {
                bound = Bits::BITScanReverse(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
            q = x & MASK_BIT_UNSET_DOWN[position];
            if (q) {
                bound = Bits::BITScanForward(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
        }
        x = allpieces & RANK[position];
        if (x & enemies) {
            q = x & MASK_BIT_UNSET_RIGHT[position];
            if (q) {
                bound = Bits::BITScanForward(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
            q = x & MASK_BIT_UNSET_LEFT[position];
            if (q) {
                bound = Bits::BITScanReverse(q);
                if (enemies & POW2[bound]) {
                    attackers |= POW2[bound];
                    if (getBoolean && attackers)return 1;
                }
            }
        }
        if (getBoolean && attackers)return 1;
        return attackers;
    }

};

