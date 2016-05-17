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
#include "namespaces/board.h"
//#include "Endgame.h"
#include <vector>

template<class TYPE_MODE>
class GenMoves : public ChessBoard /* add Endgame and remove ChessBoard TODO*/ {

public:

    static const int MAX_MOVE = 130;

    GenMoves() : listId(-1) {
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

    ~GenMoves() {
        for (int i = 0; i < MAX_PLY; i++) {
            free(gen_list[i].moveList);
        }
        free(gen_list);
        free(repetitionMap);
    }

    bool generateCaptures(const int side, const u64 enemies, const u64 friends) {
        ASSERT_RANGE(side, 0, 1);
        return side ? generateCaptures<WHITE>(enemies, friends) : generateCaptures<BLACK>(enemies, friends);
    }

    void setKillerHeuristic(const int from, const int to, const int value) {
        if (getRunning()) {
            ASSERT_RANGE(from, 0, 63);
            ASSERT_RANGE(to, 0, 63);
            killerHeuristic[from][to] = value;
        }
    }

    void generateMoves(const int side, const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1);
        side ? generateMoves<WHITE>(allpieces) : generateMoves<BLACK>(allpieces);
    }

    template<int side>
    void generateMoves(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard.bit[KING_BLACK]);
        ASSERT(chessboard.bit[KING_WHITE]);
        tryAllCastle<side>(allpieces);
        performDiagShift<side>(BISHOP_BLACK + side, allpieces);
        performRankFileShift<side>(ROOK_BLACK + side, allpieces);
        performRankFileShift<side>(QUEEN_BLACK + side, allpieces);
        performDiagShift<side>(QUEEN_BLACK + side, allpieces);
        performPawnShift<side>(~allpieces);
        performKnightShiftCapture<side>(KNIGHT_BLACK + side, ~allpieces);
        performKingShiftCapture<side>(~allpieces);
    }

    template<int side>
    bool generateCaptures(const u64 enemies, const u64 friends) {
        ASSERT_RANGE(side, 0, 1);
        ASSERT(chessboard.bit[KING_BLACK]);
        ASSERT(chessboard.bit[KING_WHITE]);
        u64 allpieces = enemies | friends;
//        if (std::is_same<TYPE_MODE, PERFT_MODE>::value) {fast_perft
//            int kingPosition = BITScanForward(chessboard.bit[KING_BLACK + side]);
//            pinned = getPin<side>(allpieces, friends, kingPosition);
//            isInCheck = isAttacked<side>(kingPosition, allpieces);
//        }

        if (performPawnCapture<side>(enemies)) {
            return true;
        }
        if (performKingShiftCapture<side>(enemies)) {
            return true;
        }
        if (performKnightShiftCapture<side>(KNIGHT_BLACK + side, enemies)) {
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

    void setForceCheck(const bool b) {
        forceCheck = b;
    }

    int getMoveFromSan(const string &fenStr, _Tmove *move) {
        chessboard.bit[ENPASSANT_IDX] = NO_ENPASSANT;
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

    void init() {
        numMoves = numMovesq = listId = 0;
#ifdef DEBUG_MODE
        nCutFp = nCutRazor = 0;
        betaEfficiency = 0.0;
        nCutAB = 0;
        nNullMoveCut = 0;
#endif
    }

    int loadFen(const string fen = "") {
        repetitionMapCount = 0;
        int side = ChessBoard::loadFen(fen);
        if (side == 2) {
            fatal("Bad FEN position format ", fen);
            std::exit(1);
        }
        return side;
    }

    inline u64 getDiagCapture(const int position, const u64 allpieces, const u64 enemies) const {
        ASSERT_RANGE(position, 0, 63);
        return Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
    }

    u64 getDiagShiftAndCapture(const int position, const u64 enemies, const u64 allpieces) const {
        ASSERT_RANGE(position, 0, 63);
        u64 bit = Bitboard::getDiagonalAntiDiagonal(position, allpieces);
        return (bit & enemies) | (bit & ~allpieces);
    }

    template<bool rep>
    void takeback(const _Tmove *move, const u64 oldkey) {
        if (rep) {
            popStackMove();
        }
        chessboard.bit[ZOBRISTKEY_IDX] = oldkey;
        chessboard.bit[ENPASSANT_IDX] = NO_ENPASSANT;
        int pieceFrom, posTo, posFrom, movecapture;
        chessboard.bit[RIGHT_CASTLE_IDX] = move->type & 0xf0;
        if ((move->type & 0x3) == STANDARD_MOVE_MASK || (move->type & 0x3) == ENPASSANT_MOVE_MASK) {
            posTo = move->to;
            posFrom = move->from;
            movecapture = move->capturedPiece;
            ASSERT_RANGE(posFrom, 0, 63);
            ASSERT_RANGE(posTo, 0, 63);
            pieceFrom = move->pieceFrom;
            chessboard.bit[pieceFrom] = (chessboard.bit[pieceFrom] & NOTPOW2[posTo]) | POW2[posFrom];
            if (movecapture != SQUARE_FREE) {
                if (((move->type & 0x3) != ENPASSANT_MOVE_MASK)) {
                    chessboard.bit[movecapture] |= POW2[posTo];
                } else {
                    ASSERT(movecapture == (move->side ^ 1));
                    if (move->side) {
                        chessboard.bit[movecapture] |= POW2[posTo - 8];
                    } else {
                        chessboard.bit[movecapture] |= POW2[posTo + 8];
                    }
                }
            }
        } else if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
            posTo = move->to;
            posFrom = move->from;
            movecapture = move->capturedPiece;
            ASSERT(posTo >= 0 && move->side >= 0 && move->promotionPiece >= 0);
            chessboard.bit[(uchar) move->side] |= POW2[posFrom];
            chessboard.bit[(uchar) move->promotionPiece] &= NOTPOW2[posTo];
            if (movecapture != SQUARE_FREE) {
                chessboard.bit[movecapture] |= POW2[posTo];
            }
        } else if (move->type & 0xc) { //castle
            unPerformCastle(move->side, move->type);
        }
    }

    void setRepetitionMapCount(const int i) {
        repetitionMapCount = i;
    }

    inline int getDiagShiftCount(const int position, const u64 allpieces) {
        ASSERT_RANGE(position, 0, 63);
        return bitCount(Bitboard::getDiagonalAntiDiagonal(position, allpieces) & ~allpieces);
    }

    template<int side>
    bool performKingShiftCapture(const u64 enemies) {
        ASSERT_RANGE(side, 0, 1);
        int pos = BITScanForward(chessboard.bit[KING_BLACK + side]);
        ASSERT(pos != -1);
        u64 x1 = enemies & NEAR_MASK1[pos];
        while (x1) {
            if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, KING_BLACK + side)) {
                return true;
            }
            RESET_LSB(x1);
        };
        return false;
    }

    template<int side>
    bool performKnightShiftCapture(const int piece, const u64 enemies) {
        ASSERT_RANGE(piece, 0, 11);
        ASSERT_RANGE(side, 0, 1);
        u64 x = chessboard.bit[piece];
        while (x) {
            int pos = BITScanForward(x);
            u64 x1 = enemies & KNIGHT_MASK[pos];
            while (x1) {
                if (pushmove<STANDARD_MOVE_MASK, side>(pos, BITScanForward(x1), NO_PROMOTION, piece)) {
                    return true;
                }
                RESET_LSB(x1);
            };
            RESET_LSB(x);
        }
        return false;
    }

    template<int side>
    bool performDiagCapture(const int piece, const u64 enemies, const u64 allpieces) {
        ASSERT_RANGE(piece, 0, 11);
        ASSERT_RANGE(side, 0, 1);
        u64 x2 = chessboard.bit[piece];
        while (x2) {
            int position = BITScanForward(x2);
            u64 diag = getDiagonalAntiDiagonal(position, allpieces) & enemies;;
            while (diag) {
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece)) {
                    return true;
                }
                RESET_LSB(diag);
            }
            RESET_LSB(x2);
        }
        return false;
    }

    u64 getTotMoves() {
        return numMoves + numMovesq;
    }

    template<int side>
    bool performRankFileCapture(const int piece, const u64 enemies, const u64 allpieces) {
        ASSERT_RANGE(piece, 0, 11);
        ASSERT_RANGE(side, 0, 1);
        u64 x2 = chessboard.bit[piece];
        while (x2) {
            int position = BITScanForward(x2);
            u64 rankFile = getRankFile(position, allpieces) & enemies;;
            while (rankFile) {
                if (pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece)) {
                    return true;
                }
                RESET_LSB(rankFile);
            }
            RESET_LSB(x2);
        }
        return false;
    }

    template<int side>
    bool performPawnCapture(const u64 enemies) {
        if (!chessboard.bit[side]) {
            if (chessboard.bit[ENPASSANT_IDX] != NO_ENPASSANT) {
                updateZobristKey(13, chessboard.bit[ENPASSANT_IDX]);
            }
            chessboard.bit[ENPASSANT_IDX] = NO_ENPASSANT;
            return false;
        }

        constexpr int GG1 = side ? -7 : 7;

        u64 x;
        if (side) {
            x = (chessboard.bit[side] << 7) & TABCAPTUREPAWN_LEFT & enemies;
        } else {
            x = (chessboard.bit[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies;
        };
        while (x) {
            int o = BITScanForward(x);
            if ((side && o > 55) || (!side && o < 8)) {//PROMOTION
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG1, o, QUEEN_BLACK + side, side)) {
                    return true;        //queen
                }
                if (std::is_same<TYPE_MODE, PERFT_MODE>::value) {
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG1, o, KNIGHT_BLACK + side, side)) {
                        return true;        //knight
                    }
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG1, o, ROOK_BLACK + side, side)) {
                        return true;        //rock
                    }
                    if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG1, o, BISHOP_BLACK + side, side)) {
                        return true;        //bishop
                    }
                }
            } else if (pushmove<STANDARD_MOVE_MASK, side>(o + GG1, o, NO_PROMOTION, side)) {
                return true;
            }
            RESET_LSB(x);
        };
        constexpr int GG2 = side ? -9 : 9;
        if (side) {
            x = (chessboard.bit[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies;
        } else {
            x = (chessboard.bit[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies;
        };
        u64 inPromotion = x & 0xff000000000000ffULL;
        while (inPromotion) {    //PROMOTION
            int o = BITScanForward(inPromotion);
            if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG2, o, QUEEN_BLACK + side, side)) {
                return true;        //queen
            }
            if (std::is_same<TYPE_MODE, PERFT_MODE>::value) {
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG2, o, KNIGHT_BLACK + side, side)) {
                    return true;        //knight
                }
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG2, o, BISHOP_BLACK + side, side)) {
                    return true;        //bishop
                }
                if (pushmove<PROMOTION_MOVE_MASK, side>(o + GG2, o, ROOK_BLACK + side, side)) {
                    return true;        //rock
                }
            }
            RESET_LSB(inPromotion);
        };

        x &= 0xffffffffffff00ULL;
        while (x) {
            int o = BITScanForward(x);
            if (pushmove<STANDARD_MOVE_MASK, side>(o + GG2, o, NO_PROMOTION, side)) {
                return true;
            }
            RESET_LSB(x);
        };
        //ENPASSANT
        if (chessboard.bit[ENPASSANT_IDX] != NO_ENPASSANT) {
            x = ENPASSANT_MASK[side ^ 1][chessboard.bit[ENPASSANT_IDX]] & chessboard.bit[side];
            while (x) {
                int o = BITScanForward(x);
                pushmove<ENPASSANT_MOVE_MASK, side>(o, (side ? chessboard.bit[ENPASSANT_IDX] + 8 : chessboard.bit[ENPASSANT_IDX] - 8), NO_PROMOTION, side);
                RESET_LSB(x);
            }
            updateZobristKey(13, chessboard.bit[ENPASSANT_IDX]);
            chessboard.bit[ENPASSANT_IDX] = NO_ENPASSANT;
        }
        return false;
    }


    template<int side>
    void performPawnShift(const u64 xallpieces) {
        constexpr int tt = side ? -8 : 8;
        u64 x = chessboard.bit[side];
        if (x & PAWNS_JUMP[side]) {
            checkJumpPawn<side>(x, xallpieces);
        }
        if (side) {
            x <<= 8;
        } else {
            x >>= 8;
        };
        x &= xallpieces;

        u64 inPromotion = x & 0xff000000000000ffULL;
        while (inPromotion) {
            int o = BITScanForward(inPromotion);
            ASSERT(getPieceAt(side, POW2[o + tt]) != SQUARE_FREE);
            ASSERT(getBitmap(side) & POW2[o + tt]);
            pushmove<PROMOTION_MOVE_MASK, side>(o + tt, o, QUEEN_BLACK + side, side);
            if (std::is_same<TYPE_MODE, PERFT_MODE>::value) {
                pushmove<PROMOTION_MOVE_MASK, side>(o + tt, o, KNIGHT_BLACK + side, side);
                pushmove<PROMOTION_MOVE_MASK, side>(o + tt, o, BISHOP_BLACK + side, side);
                pushmove<PROMOTION_MOVE_MASK, side>(o + tt, o, ROOK_BLACK + side, side);
            }
            RESET_LSB(inPromotion);
        }

        x &= 0xffffffffffff00ULL;
        while (x) {
            int o = BITScanForward(x);
            ASSERT(getPieceAt(side, POW2[o + tt]) != SQUARE_FREE);
            ASSERT(getBitmap(side) & POW2[o + tt]);
            pushmove<STANDARD_MOVE_MASK, side>(o + tt, o, NO_PROMOTION, side);
            RESET_LSB(x);
        };
    }

    void clearKillerHeuristic() {
        memset(killerHeuristic, 0, sizeof(killerHeuristic));
    }

    template<int side>
    void performDiagShift(const int piece, const u64 allpieces) {
        ASSERT_RANGE(piece, 0, 11);
        ASSERT_RANGE(side, 0, 1);
        u64 x2 = chessboard.bit[piece];
        while (x2) {
            int position = BITScanForward(x2);
            u64 diag = getDiagonalAntiDiagonal(position, allpieces) & ~allpieces;
            while (diag) {
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(diag), NO_PROMOTION, piece);
                RESET_LSB(diag);
            }
            RESET_LSB(x2);
        }
    }

    template<int side>
    void performRankFileShift(const int piece, const u64 allpieces) {
        ASSERT_RANGE(piece, 0, 11);
        ASSERT_RANGE(side, 0, 1);
        u64 x2 = chessboard.bit[piece];
        while (x2) {
            int position = BITScanForward(x2);
            u64 rankFile = getRankFile(position, allpieces) & ~allpieces;
            while (rankFile) {
                pushmove<STANDARD_MOVE_MASK, side>(position, BITScanForward(rankFile), NO_PROMOTION, piece);
                RESET_LSB(rankFile);
            }
            RESET_LSB(x2);
        }
    }

    bool makemove(const _Tmove *move, const bool rep = true, const bool checkInCheck = false) {
        ASSERT(move);
        ASSERT(bitCount(chessboard.bit[KING_WHITE]) == 1 && bitCount(chessboard.bit[KING_BLACK]) == 1);
        int pieceFrom = SQUARE_FREE, posTo, posFrom, movecapture = SQUARE_FREE;
        uchar rightCastleOld = chessboard.bit[RIGHT_CASTLE_IDX];
        if (!(move->type & 0xc)) { //no castle
            posTo = move->to;
            posFrom = move->from;
            movecapture = move->capturedPiece;
            ASSERT_RANGE(posFrom, 0, 63);
            ASSERT_RANGE(posTo, 0, 63);
            pieceFrom = move->pieceFrom;
            if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
                chessboard.bit[pieceFrom] &= NOTPOW2[posFrom];
                updateZobristKey(pieceFrom, posFrom);
                ASSERT(move->promotionPiece >= 0);
                chessboard.bit[(uchar) move->promotionPiece] |= POW2[posTo];
                updateZobristKey((uchar) move->promotionPiece, posTo);
            } else {
                chessboard.bit[pieceFrom] = (chessboard.bit[pieceFrom] | POW2[posTo]) & NOTPOW2[posFrom];
                updateZobristKey(pieceFrom, posFrom);
                updateZobristKey(pieceFrom, posTo);
            }
            if (movecapture != SQUARE_FREE) {
                if ((move->type & 0x3) != ENPASSANT_MOVE_MASK) {
                    chessboard.bit[movecapture] &= NOTPOW2[posTo];
                    updateZobristKey(movecapture, posTo);
                } else { //en passant
                    ASSERT(movecapture == (move->side ^ 1));
                    if (move->side) {
                        chessboard.bit[movecapture] &= NOTPOW2[posTo - 8];
                        updateZobristKey(movecapture, posTo - 8);
                    } else {
                        chessboard.bit[movecapture] &= NOTPOW2[posTo + 8];
                        updateZobristKey(movecapture, posTo + 8);
                    }
                }
            }
            //lost castle right
            switch (pieceFrom) {
                case KING_WHITE: {
                    chessboard.bit[RIGHT_CASTLE_IDX] &= 0xcf;
                }
                    break;
                case KING_BLACK: {
                    chessboard.bit[RIGHT_CASTLE_IDX] &= 0x3f;
                }
                    break;
                case ROOK_WHITE:
                    if (posFrom == 0) {
                        chessboard.bit[RIGHT_CASTLE_IDX] &= 0xef;
                    } else if (posFrom == 7) {
                        chessboard.bit[RIGHT_CASTLE_IDX] &= 0xdf;
                    }
                    break;
                case ROOK_BLACK:
                    if (posFrom == 56) {
                        chessboard.bit[RIGHT_CASTLE_IDX] &= 0xbf;
                    } else if (posFrom == 63) {
                        chessboard.bit[RIGHT_CASTLE_IDX] &= 0x7f;
                    }
                    break;
                    //en passant
                case PAWN_WHITE:
                    if ((RANK_1 & POW2[posFrom]) && (RANK_3 & POW2[posTo])) {
                        chessboard.bit[ENPASSANT_IDX] = posTo;
                        updateZobristKey(13, chessboard.bit[ENPASSANT_IDX]);
                    }
                    break;
                case PAWN_BLACK:
                    if ((RANK_6 & POW2[posFrom]) && (RANK_4 & POW2[posTo])) {
                        chessboard.bit[ENPASSANT_IDX] = posTo;
                        updateZobristKey(13, chessboard.bit[ENPASSANT_IDX]);
                    }
                    break;
                default:;
            }
        } else { //castle
            performCastle(move->side, move->type);
            if (move->side == WHITE) {
                chessboard.bit[RIGHT_CASTLE_IDX] &= 0xcf;
            } else {
                chessboard.bit[RIGHT_CASTLE_IDX] &= 0x3f;
            }
        }
        u64 x2 = rightCastleOld ^chessboard.bit[RIGHT_CASTLE_IDX];
        while (x2) {
            int position = BITScanForward(x2);
            updateZobristKey(14, position);
            RESET_LSB(x2);
        }
        if (rep) {
            if (movecapture != SQUARE_FREE || pieceFrom == WHITE || pieceFrom == BLACK || move->type & 0xc) {
                pushStackMove(0);
            }
            pushStackMove(chessboard.bit[ZOBRISTKEY_IDX]);
        }
        if ((forceCheck || (checkInCheck && !(std::is_same<TYPE_MODE, PERFT_MODE>::value))) && ((move->side == WHITE && inCheck<WHITE>()) || (move->side == BLACK && inCheck<BLACK>()))) {
            return false;
        }
        return true;
    }

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
        pushStackMove(chessboard.bit[ZOBRISTKEY_IDX]);
    }

    void resetList() {
        gen_list[listId].size = 0;
    }

    bool generatePuzzle(const string type) {
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

    void incKillerHeuristic(const int from, const int to, const int value) {
        if (!getRunning()) {
            return;
        }
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        ASSERT(killerHeuristic[from][to] <= killerHeuristic[from][to] + value);
        killerHeuristic[from][to] += value;
    }

//    template<int side>fast_perft
//    u64 getPin(const u64 allpieces, const u64 friends, const int kingPosition) const {
//        u64 result = 0;
////        allpieces &= NOTPOW2[kingPosition];
//        const u64 *s = LINK_SQUARE[kingPosition];
//        constexpr int xside = side ^1;
//        u64 attacked = DIAGONAL_ANTIDIAGONAL[kingPosition] & (chessboard.bit[QUEEN_BLACK + xside] | chessboard.bit[BISHOP_BLACK + xside]);
//        attacked |= RANK_FILE[kingPosition] & (chessboard.bit[QUEEN_BLACK + xside] | chessboard.bit[ROOK_BLACK + xside]);
//        while (attacked) {
//            int pos = BITScanForward(attacked);
//            u64 b = *(s + pos) & allpieces;
//#ifdef DEBUG_MODE
//            u64 x = *(s + pos) & (allpieces & NOTPOW2[kingPosition]);
//            ASSERT(b == x);
//#endif
//            if (!static_cast<u64>(b & (b - 1))) {
//                result |= b & friends;
//            }
//            RESET_LSB(attacked);
//        }
//        return result;
//    }

#ifdef DEBUG_MODE
    unsigned nCutAB, nNullMoveCut, nCutFp, nCutRazor;
    double betaEfficiency;
#endif

protected:
//    u64 pinned;fast_perft
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

//    void sortList(_TmoveP *list) {
//        _Tmove *gen_list1 = list->moveList;
//
////        for(int i=0;i<list->size;i++)cout <<gen_list1[i].score<<",";
////        sort(gen_list1, gen_list1 + size, [](const _Tmove &x, const _Tmove &y) { return (x.score > y.score); });
//        std::sort((u64 *) gen_list1, (u64 *) (gen_list1 + list->size), std::greater<u64>());
////        cout <<"\n";
////        for(int i=0;i<list->size;i++)cout <<gen_list1[i].score<<",";
////        cout <<endl<<endl;
//    }

    _Tmove *getNextMove(const _TmoveP *list) {
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

    template<int side>
    inline bool isAttacked(const int position, const u64 allpieces) const {
        return getAttackers<side, true>(position, allpieces);
    }

    template<int side>
    u64 getAllAttackers(const int position, const u64 allpieces) const {
        return getAttackers<side, false>(position, allpieces);
    }

    int getMobilityRook(const int position, const u64 enemies, const u64 friends) {
        ASSERT_RANGE(position, 0, 63);
        return performRankFileCaptureAndShiftCount(position, enemies, enemies | friends);
    }

    int getMobilityPawns(const int side, const int ep, const u64 ped_friends, const u64 enemies, const u64 xallpieces) const {
        ASSERT_RANGE(side, 0, 1);
        return ep == NO_ENPASSANT ? 0 : bitCount(ENPASSANT_MASK[side ^ 1][ep] & chessboard.bit[side]) + side == WHITE ?
                                        bitCount((ped_friends << 8) & xallpieces) + bitCount(((((ped_friends & TABJUMPPAWN) << 8) & xallpieces) << 8) & xallpieces) + bitCount((chessboard.bit[side] << 7) & TABCAPTUREPAWN_LEFT & enemies) + bitCount((chessboard.bit[side] << 9) & TABCAPTUREPAWN_RIGHT & enemies) :
                                        bitCount((ped_friends >> 8) & xallpieces) +
                                        bitCount(((((ped_friends & TABJUMPPAWN) >> 8) & xallpieces) >> 8) & xallpieces) +
                                        bitCount((chessboard.bit[side] >> 7) & TABCAPTUREPAWN_RIGHT & enemies) +
                                        bitCount((chessboard.bit[side] >> 9) & TABCAPTUREPAWN_LEFT & enemies);
    }


    int getMobilityCastle(const int side, const u64 allpieces) const {
        ASSERT_RANGE(side, 0, 1);
        int count = 0;
        if (side == WHITE) {
            if (POW2_3 & chessboard.bit[KING_WHITE] && !(allpieces & 0x6ULL) && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK && chessboard.bit[ROOK_WHITE] & POW2_0 && !isAttacked<WHITE>(1, allpieces) && !isAttacked<WHITE>(2, allpieces) && !isAttacked<WHITE>(3, allpieces)) {
                count++;
            }
            if (POW2_3 & chessboard.bit[KING_WHITE] && !(allpieces & 0x70ULL) && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard.bit[ROOK_WHITE] & POW2_7 && !isAttacked<WHITE>(3, allpieces) && !isAttacked<WHITE>(4, allpieces) && !isAttacked<WHITE>(5, allpieces)) {
                count++;
            }
        } else {
            if (POW2_59 & chessboard.bit[KING_BLACK] && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK && !(allpieces & 0x600000000000000ULL) && chessboard.bit[ROOK_BLACK] & POW2_56 && !isAttacked<BLACK>(57, allpieces) && !isAttacked<BLACK>(58, allpieces) && !isAttacked<BLACK>(59, allpieces)) {
                count++;
            }
            if (POW2_59 & chessboard.bit[KING_BLACK] && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK && !(allpieces & 0x7000000000000000ULL) && chessboard.bit[ROOK_BLACK] & POW2_63 && !isAttacked<BLACK>(59, allpieces) && !isAttacked<BLACK>(60, allpieces) && !isAttacked<BLACK>(61, allpieces)) {
                count++;
            }
        }
        return count;
    }

    int getMobilityQueen(const int position, const u64 enemies, const u64 allpieces) {
        ASSERT_RANGE(position, 0, 63);
        return performRankFileCaptureAndShiftCount(position, enemies, allpieces) +
               bitCount(getDiagShiftAndCapture(position, enemies, allpieces));
    }

    int killerHeuristic[64][64];

#ifdef DEBUG_MODE

    template<int side, uchar type>
    bool inCheckSlow(const int from, const int to, const int pieceFrom, const int pieceTo, int promotionPiece) const {

        bool result = false;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_FREE);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard.bit[pieceFrom];
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard.bit[pieceTo];
                    chessboard.bit[pieceTo] &= NOTPOW2[to];
                };
                chessboard.bit[pieceFrom] &= NOTPOW2[from];
                chessboard.bit[pieceFrom] |= POW2[to];
                ASSERT(chessboard.bit[KING_BLACK]);
                ASSERT(chessboard.bit[KING_WHITE]);

                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard.bit[pieceFrom] = from1;
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] = to1;
                };
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard.bit[pieceTo];
                }
                u64 from1 = chessboard.bit[pieceFrom];
                u64 p1 = chessboard.bit[promotionPiece];
                chessboard.bit[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] &= NOTPOW2[to];
                }
                chessboard.bit[promotionPiece] = chessboard.bit[promotionPiece] | POW2[to];
                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] = to1;
                }
                chessboard.bit[pieceFrom] = from1;
                chessboard.bit[promotionPiece] = p1;
                break;
            }
            case ENPASSANT_MOVE_MASK: {
                u64 to1 = chessboard.bit[side ^ 1];
                u64 from1 = chessboard.bit[side];
                chessboard.bit[side] &= NOTPOW2[from];
                chessboard.bit[side] |= POW2[to];
                if (side) {
                    chessboard.bit[side ^ 1] &= NOTPOW2[to - 8];
                } else {
                    chessboard.bit[side ^ 1] &= NOTPOW2[to + 8];
                }
                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard.bit[side ^ 1] = to1;
                chessboard.bit[side] = from1;;
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
        ASSERT_RANGE(from, 0, 63);
        ASSERT_RANGE(to, 0, 63);
        ASSERT_RANGE(side, 0, 1);
        ASSERT_RANGE(pieceFrom, 0, 12);
        ASSERT_RANGE(pieceTo, 0, 12);
        ASSERT((std::is_same<TYPE_MODE, PERFT_MODE>::value) || forceCheck);
        ASSERT(!(type & 0xc));
//        if (std::is_same<TYPE_MODE, PERFT_MODE>::value) {fast_perft
//            if ((KING_BLACK + side) != pieceFrom && !isInCheck) {
//                if (!(pinned & POW2[from]) || (LINES[from][to] & chessboard.bit[KING_BLACK + side])) {
//                    ASSERT(!(inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
//                    return false;
//                } else {
//                    ASSERT ((inCheckSlow<side, type>(from, to, pieceFrom, pieceTo, promotionPiece)));
//                    return true;
//                }
//            }
//
//        }
#ifdef DEBUG_MODE
        _Tchessboard a;
        memcpy(&a, chessboard.bit, sizeof(_Tchessboard));
#endif
        bool result = false;
        switch (type & 0x3) {
            case STANDARD_MOVE_MASK: {
                u64 from1, to1 = -1;
                ASSERT(pieceFrom != SQUARE_FREE);
                ASSERT(pieceTo != KING_BLACK);
                ASSERT(pieceTo != KING_WHITE);
                from1 = chessboard.bit[pieceFrom];
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard.bit[pieceTo];
                    chessboard.bit[pieceTo] &= NOTPOW2[to];
                };
                chessboard.bit[pieceFrom] &= NOTPOW2[from];
                chessboard.bit[pieceFrom] |= POW2[to];
                ASSERT(chessboard.bit[KING_BLACK]);
                ASSERT(chessboard.bit[KING_WHITE]);

                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard.bit[pieceFrom] = from1;
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] = to1;
                };
                break;
            }
            case PROMOTION_MOVE_MASK: {
                u64 to1 = 0;
                if (pieceTo != SQUARE_FREE) {
                    to1 = chessboard.bit[pieceTo];
                }
                u64 from1 = chessboard.bit[pieceFrom];
                u64 p1 = chessboard.bit[promotionPiece];
                chessboard.bit[pieceFrom] &= NOTPOW2[from];
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] &= NOTPOW2[to];
                }
                chessboard.bit[promotionPiece] = chessboard.bit[promotionPiece] | POW2[to];
                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                if (pieceTo != SQUARE_FREE) {
                    chessboard.bit[pieceTo] = to1;
                }
                chessboard.bit[pieceFrom] = from1;
                chessboard.bit[promotionPiece] = p1;
                break;
            }
            case ENPASSANT_MOVE_MASK: {
                u64 to1 = chessboard.bit[side ^ 1];
                u64 from1 = chessboard.bit[side];
                chessboard.bit[side] &= NOTPOW2[from];
                chessboard.bit[side] |= POW2[to];
                if (side) {
                    chessboard.bit[side ^ 1] &= NOTPOW2[to - 8];
                } else {
                    chessboard.bit[side ^ 1] &= NOTPOW2[to + 8];
                }
                result = isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
                chessboard.bit[side ^ 1] = to1;
                chessboard.bit[side] = from1;;
                break;
            }
            default:
            _assert(0);
        }

#ifdef DEBUG_MODE
        ASSERT(!memcmp(&a, chessboard.bit, sizeof(_Tchessboard)));
#endif

        return result;
    }

    void performCastle(const int side, const uchar type) {
        ASSERT_RANGE(side, 0, 1);
        if (side == WHITE) {
            if (type & KING_SIDE_CASTLE_MOVE_MASK) {
                ASSERT(getPieceAt(side, POW2_3) == KING_WHITE);
                ASSERT(getPieceAt(side, POW2_1) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_2) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_0) == ROOK_WHITE);
                updateZobristKey(KING_WHITE, E1);
                updateZobristKey(KING_WHITE, G1);
                chessboard.bit[KING_WHITE] = (chessboard.bit[KING_WHITE] | POW2_1) & NOTPOW2_3;
                updateZobristKey(ROOK_WHITE, F1);
                updateZobristKey(ROOK_WHITE, H1);
                chessboard.bit[ROOK_WHITE] = (chessboard.bit[ROOK_WHITE] | POW2_2) & NOTPOW2_0;
            } else {
                ASSERT(type & QUEEN_SIDE_CASTLE_MOVE_MASK);
                ASSERT(getPieceAt(side, POW2_3) == KING_WHITE);
                ASSERT(getPieceAt(side, POW2_4) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_5) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_6) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_7) == ROOK_WHITE);
                chessboard.bit[KING_WHITE] = (chessboard.bit[KING_WHITE] | POW2_5) & NOTPOW2_3;
                updateZobristKey(KING_WHITE, C1);
                updateZobristKey(KING_WHITE, E1);
                chessboard.bit[ROOK_WHITE] = (chessboard.bit[ROOK_WHITE] | POW2_4) & NOTPOW2_7;
                updateZobristKey(ROOK_WHITE, D1);
                updateZobristKey(ROOK_WHITE, A1);
            }
        } else {
            if (type & KING_SIDE_CASTLE_MOVE_MASK) {
                ASSERT(getPieceAt(side, POW2_59) == KING_BLACK);
                ASSERT(getPieceAt(side, POW2_58) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_57) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_56) == ROOK_BLACK);
                chessboard.bit[KING_BLACK] = (chessboard.bit[KING_BLACK] | POW2_57) & NOTPOW2_59;
                updateZobristKey(KING_BLACK, G8);
                updateZobristKey(KING_BLACK, E8);
                chessboard.bit[ROOK_BLACK] = (chessboard.bit[ROOK_BLACK] | POW2_58) & NOTPOW2_56;
                updateZobristKey(ROOK_BLACK, F8);
                updateZobristKey(ROOK_BLACK, H8);
            } else {
                ASSERT(type & QUEEN_SIDE_CASTLE_MOVE_MASK);
                ASSERT(getPieceAt(side, POW2_59) == KING_BLACK);
                ASSERT(getPieceAt(side, POW2_60) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_61) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_62) == SQUARE_FREE);
                ASSERT(getPieceAt(side, POW2_63) == ROOK_BLACK);
                chessboard.bit[KING_BLACK] = (chessboard.bit[KING_BLACK] | POW2_61) & NOTPOW2_59;
                updateZobristKey(KING_BLACK, C8);
                updateZobristKey(KING_BLACK, E8);
                chessboard.bit[ROOK_BLACK] = (chessboard.bit[ROOK_BLACK] | POW2_60) & NOTPOW2_63;
                updateZobristKey(ROOK_BLACK, D8);
                updateZobristKey(ROOK_BLACK, A8);
            }
        }
    }

    void unPerformCastle(const int side, const uchar type) {
        ASSERT_RANGE(side, 0, 1);
        if (side == WHITE) {
            if (type & KING_SIDE_CASTLE_MOVE_MASK) {
                ASSERT(getPieceAt(side, POW2_1) == KING_WHITE);
                ASSERT(getPieceAt(side, POW2_0) == 12);
                ASSERT(getPieceAt(side, POW2_3) == 12);
                ASSERT(getPieceAt(side, POW2_2) == ROOK_WHITE);
                chessboard.bit[KING_WHITE] = (chessboard.bit[KING_WHITE] | POW2_3) & NOTPOW2_1;
                chessboard.bit[ROOK_WHITE] = (chessboard.bit[ROOK_WHITE] | POW2_0) & NOTPOW2_2;
            } else {
                chessboard.bit[KING_WHITE] = (chessboard.bit[KING_WHITE] | POW2_3) & NOTPOW2_5;
                chessboard.bit[ROOK_WHITE] = (chessboard.bit[ROOK_WHITE] | POW2_7) & NOTPOW2_4;
            }
        } else {
            if (type & KING_SIDE_CASTLE_MOVE_MASK) {
                chessboard.bit[KING_BLACK] = (chessboard.bit[KING_BLACK] | POW2_59) & NOTPOW2_57;
                chessboard.bit[ROOK_BLACK] = (chessboard.bit[ROOK_BLACK] | POW2_56) & NOTPOW2_58;
            } else {
                chessboard.bit[KING_BLACK] = (chessboard.bit[KING_BLACK] | POW2_59) & NOTPOW2_61;
                chessboard.bit[ROOK_BLACK] = (chessboard.bit[ROOK_BLACK] | POW2_63) & NOTPOW2_60;
            }
        }
    }

    template<int side>
    void tryAllCastle(const u64 allpieces) {
        ASSERT_RANGE(side, 0, 1);
        if (side == WHITE) {
            if (POW2_3 & chessboard.bit[KING_WHITE] && !(allpieces & 0x6ULL) && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK && chessboard.bit[ROOK_WHITE] & POW2_0 && !isAttacked<WHITE>(1, allpieces) && !isAttacked<WHITE>(2, allpieces) && !isAttacked<WHITE>(3, allpieces)) {
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, WHITE>(-1, -1, NO_PROMOTION, -1);
            }
            if (POW2_3 & chessboard.bit[KING_WHITE] && !(allpieces & 0x70ULL) && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard.bit[ROOK_WHITE] & POW2_7 && !isAttacked<WHITE>(3, allpieces) && !isAttacked<WHITE>(4, allpieces) && !isAttacked<WHITE>(5, allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, WHITE>(-1, -1, NO_PROMOTION, -1);
            }
        } else {
            if (POW2_59 & chessboard.bit[KING_BLACK] && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK && !(allpieces & 0x600000000000000ULL) && chessboard.bit[ROOK_BLACK] & POW2_56 && !isAttacked<BLACK>(57, allpieces) && !isAttacked<BLACK>(58, allpieces) && !isAttacked<BLACK>(59, allpieces)) {
                pushmove<KING_SIDE_CASTLE_MOVE_MASK, BLACK>(-1, -1, NO_PROMOTION, -1);
            }
            if (POW2_59 & chessboard.bit[KING_BLACK] && chessboard.bit[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK && !(allpieces & 0x7000000000000000ULL) && chessboard.bit[ROOK_BLACK] & POW2_63 && !isAttacked<BLACK>(59, allpieces) && !isAttacked<BLACK>(60, allpieces) && !isAttacked<BLACK>(61, allpieces)) {
                pushmove<QUEEN_SIDE_CASTLE_MOVE_MASK, BLACK>(-1, -1, NO_PROMOTION, -1);
            }
        }
    }

    template<uchar type, int side>
    bool pushmove(const int from, const int to, int promotionPiece, int pieceFrom) {
        ASSERT(chessboard.bit[KING_BLACK]);
        ASSERT(chessboard.bit[KING_WHITE]);
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
        if (!(type & 0xc) && (forceCheck || (std::is_same<TYPE_MODE, PERFT_MODE>::value))) {//no castle
            if (inCheck<side, type>(from, to, pieceFrom, piece_captured, promotionPiece)) {
                return false;
            }
        }
        _Tmove *mos;
        ASSERT_RANGE(listId, 0, MAX_PLY - 1);

        ASSERT(getListSize() < MAX_MOVE);
        mos = &gen_list[listId].moveList[getListSize()];
        ++gen_list[listId].size;
        mos->type = (uchar) chessboard.bit[RIGHT_CASTLE_IDX] | type;
        mos->side = (char) side;
        mos->capturedPiece = piece_captured;
        if (type & 0x3) {
            mos->from = (uchar) from;
            mos->to = (uchar) to;
            mos->pieceFrom = pieceFrom;
            mos->promotionPiece = (char) promotionPiece;
            if (!(std::is_same<TYPE_MODE, PERFT_MODE>::value)) {
                if (res == true) {
                    mos->score = _INFINITE;
                } else {
                    ASSERT_RANGE(pieceFrom, 0, 11);
                    ASSERT_RANGE(to, 0, 63);
                    ASSERT_RANGE(from, 0, 63);
                    mos->score = killerHeuristic[from][to];
                    mos->score += (PIECES_VALUE[piece_captured] >= PIECES_VALUE[pieceFrom]) ? (PIECES_VALUE[piece_captured] - PIECES_VALUE[pieceFrom]) * 2 : PIECES_VALUE[piece_captured];
                    //mos->score += (MOV_ORD[pieceFrom][to] - MOV_ORD[pieceFrom][from]);
                }
            }
        } else if (type & 0xc) {    //castle
            ASSERT(chessboard.bit[RIGHT_CASTLE_IDX]);
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
    inline bool inCheck(const u64 allpieces) const {
        return isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), allpieces);
    }

    template<int side>
    inline bool inCheck() const {
        return isAttacked<side>(BITScanForward(chessboard.bit[KING_BLACK + side]), getBitmap<BLACK>() | getBitmap<WHITE>());
    }

private:
    int running;
//    bool isInCheck;fast_perft

    static bool forceCheck;
    static const u64 TABJUMPPAWN = 0xFF00000000FF00ULL;
    static const u64 TABCAPTUREPAWN_RIGHT = 0xFEFEFEFEFEFEFEFEULL;
    static const u64 TABCAPTUREPAWN_LEFT = 0x7F7F7F7F7F7F7F7FULL;

    void writeRandomFen(const vector<int> pieces) {
        while (1) {
            memset(chessboard.bit, 0, sizeof(_Tchessboard));
            chessboard.bit[ENPASSANT_IDX] = NO_ENPASSANT;
            chessboard.bit[SIDETOMOVE_IDX] = rand() % 2;
            chessboard.bit[KING_BLACK] = POW2[rand() % 64];
            chessboard.bit[KING_WHITE] = POW2[rand() % 64];
            u64 check = chessboard.bit[KING_BLACK] | chessboard.bit[KING_WHITE];
            for (unsigned long i = 0; i < pieces.size(); i++) {
                chessboard.bit[pieces[i]] |= POW2[rand() % 64];
                check |= chessboard.bit[pieces[i]];
            }
            u64 allpieces = getBitmap<BLACK>() | getBitmap<WHITE>();
            if (bitCount(check) == (2 + (int) pieces.size()) && !inCheck<WHITE>(allpieces) && !inCheck<BLACK>(allpieces)) {
                cout << boardToFen() << "\n";
                loadFen(boardToFen());
                return;
            }
        }
    }

    template<int side>
    void checkJumpPawn(u64 x, const u64 xallpieces) {
        x &= TABJUMPPAWN;
        if (side) {
            x = (((x << 8) & xallpieces) << 8) & xallpieces;
        } else {
            x = (((x >> 8) & xallpieces) >> 8) & xallpieces;
        };
        while (x) {
            int o = BITScanForward(x);
            pushmove<STANDARD_MOVE_MASK, side>(o + (side ? -16 : 16), o, NO_PROMOTION, side);
            RESET_LSB(x);
        };
    }

    int performRankFileCaptureAndShiftCount(const int position, const u64 enemies, const u64 allpieces) {
        ASSERT_RANGE(position, 0, 63);
        u64 rankFile = getRankFile(position, allpieces);
        rankFile = (rankFile & enemies) | (rankFile & ~allpieces);
        return bitCount(rankFile);
    }

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
        int bound;
        ///knight
        u64 attackers = KNIGHT_MASK[position] & chessboard.bit[KNIGHT_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///king
        attackers |= NEAR_MASK1[position] & chessboard.bit[KING_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///pawn
        attackers |= PAWN_FORK_MASK[side][position] & chessboard.bit[PAWN_BLACK + (side ^ 1)];
        if (exitOnFirst && attackers)return 1;
        ///bishop queen
        u64 enemies = chessboard.bit[BISHOP_BLACK + (side ^ 1)] | chessboard.bit[QUEEN_BLACK + (side ^ 1)];
        u64 bit = Bitboard::getDiagonalAntiDiagonal(position, allpieces) & enemies;
        while (bit) {
            int bound = BITScanForward(bit);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers)return 1;
            RESET_LSB(bit);
        }
        enemies = chessboard.bit[ROOK_BLACK + (side ^ 1)] | chessboard.bit[QUEEN_BLACK + (side ^ 1)];
        bit = Bitboard::getRankFile(position, allpieces) & enemies;
        while (bit) {
            int bound = BITScanForward(bit);
            attackers |= POW2[bound];
            if (exitOnFirst && attackers)return 1;
            RESET_LSB(bit);
        }
        return attackers;
    }


};

template<class SEARCH_MODE>
bool GenMoves<SEARCH_MODE>::forceCheck = false;
