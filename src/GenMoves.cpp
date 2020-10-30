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

bool GenMoves::forceCheck = false;

GenMoves::GenMoves() : perftMode(false), listId(-1) {
    currentPly = 0;
    gen_list = (_TmoveP *) calloc(MAX_PLY, sizeof(_TmoveP));
    _assert(gen_list)
    for (int i = 0; i < MAX_PLY; i++) {
        gen_list[i].moveList = (_Tmove *) calloc(MAX_MOVE, sizeof(_Tmove));
        _assert(gen_list[i].moveList)
    }
    repetitionMap = (u64 *) malloc(sizeof(u64) * MAX_REP_COUNT);
    _assert(repetitionMap)
    repetitionMapCount = 0;
    init();
}

void GenMoves::generateMoves(const int side, const u64 allpieces) {
    ASSERT_RANGE(side, 0, 1)
    side ? generateMoves<WHITE>(allpieces) : generateMoves<BLACK>(allpieces);
}

bool GenMoves::generateCaptures(const int side, const u64 enemies, const u64 friends) {
    ASSERT_RANGE(side, 0, 1)
    return side ? generateCaptures<WHITE>(enemies, friends) : generateCaptures<BLACK>(enemies, friends);
}

void GenMoves::setPerft(const bool b) {
    perftMode = b;
}

void GenMoves::clearHeuristic() {
    memset(historyHeuristic, 0, sizeof(historyHeuristic));
    memset(killer, 0, sizeof(killer));
}

_Tmove *GenMoves::getNextMove(_TmoveP *list, const int depth, const Hash::_ThashData *hash, const int first) {
    BENCH(times->start("getNextMove"))

    int bestId = -1;
    int bestScore = -1;

    for (int i = first; i < list->size; i++) {
        auto mos = list->moveList[i];
        int score = 0;
        if (mos.s.type & 0x3) {
            if (mos.s.capturedPiece == KING_BLACK + (mos.s.side ^ 1)) {
                score = _INFINITE;
            } else {
                ASSERT_RANGE(mos.s.pieceFrom, 0, 11);
                ASSERT_RANGE(mos.s.to, 0, 63);
                ASSERT_RANGE(mos.s.from, 0, 63);

                if (hash && (hash->dataS.from == mos.s.from && hash->dataS.to == mos.s.to)) {
                    score = _INFINITE / 2;
                }
                score += historyHeuristic[mos.s.from][mos.s.to];
                score += (PIECES_VALUE[mos.s.capturedPiece] > PIECES_VALUE[mos.s.pieceFrom]) ?
                         (PIECES_VALUE[mos.s.capturedPiece] - PIECES_VALUE[mos.s.pieceFrom]) * 2
                                                                                             : PIECES_VALUE[mos.s.capturedPiece];
                if (isKillerMate(mos.s.from, mos.s.to, depth)) score += 100;
                else if (isKiller(0, mos.s.from, mos.s.to, depth)) score += 90;
                else if (isKiller(1, mos.s.from, mos.s.to, depth)) score += 80;

            }
        } else if (mos.s.type & 0xc) {    //castle
            ASSERT(chessboard[RIGHT_CASTLE_IDX]);
            score = 100;
        }
        if (score > bestScore) {
            bestScore = score;
            bestId = i;
        }
    }
    if (bestId == -1) {
        BENCH(times->stop("getNextMove"))
        return nullptr;
    }
//    swap(list->moveList[first], list->moveList[bestId]);
    const auto tmp = list->moveList[first].u;
    list->moveList[first].u = list->moveList[bestId].u;
    list->moveList[bestId].u = tmp;

    BENCH(times->stop("getNextMove"))
    return &list->moveList[first];
}

GenMoves::~GenMoves() {
    for (int i = 0; i < MAX_PLY; i++) {
        free(gen_list[i].moveList);
    }
    free(gen_list);
    free(repetitionMap);
}

void GenMoves::performCastle(const int side, const uchar type) {
    ASSERT_RANGE(side, 0, 1)

    if (side == WHITE) {
        ASSERT(chessboard[KING_WHITE] & POW2[startPosWhiteKing])
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            if (startPosWhiteKing != G1) {
                updateZobristKey(KING_WHITE, startPosWhiteKing);
                updateZobristKey(KING_WHITE, G1);
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_1) & NOTPOW2[startPosWhiteKing];
            }
            if (startPosWhiteRookKingSide != F1) {
                updateZobristKey(ROOK_WHITE, F1);
                updateZobristKey(ROOK_WHITE, startPosWhiteRookKingSide);

                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_2) & NOTPOW2[startPosWhiteRookKingSide];
            }

            ASSERT(chessboard[KING_WHITE] & POW2[G1])
            ASSERT(chessboard[ROOK_WHITE] & POW2[F1])

        } else {
            if (startPosWhiteKing != C1) {
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_5) & NOTPOW2[startPosWhiteKing];
                updateZobristKey(KING_WHITE, C1);
                updateZobristKey(KING_WHITE, startPosWhiteKing);
            }
            if (startPosWhiteRookQueenSide != D1) {
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_4) & NOTPOW2[startPosWhiteRookQueenSide];
                updateZobristKey(ROOK_WHITE, D1);
                updateZobristKey(ROOK_WHITE, startPosWhiteRookQueenSide);
            }
            ASSERT(chessboard[KING_WHITE] & POW2[C1])
            ASSERT(chessboard[ROOK_WHITE] & POW2[D1])

        }
    } else {
        ASSERT(chessboard[KING_BLACK] & POW2[startPosBlackKing])

        if (type & KING_SIDE_CASTLE_MOVE_MASK) {

            if (startPosBlackKing != G8) {
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_57) & NOTPOW2[startPosBlackKing];
                updateZobristKey(KING_BLACK, G8);
                updateZobristKey(KING_BLACK, startPosBlackKing);
            }
            if (startPosBlackRookKingSide != F8) {
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_58) & NOTPOW2[startPosBlackRookKingSide];
                updateZobristKey(ROOK_BLACK, F8);
                updateZobristKey(ROOK_BLACK, startPosBlackRookKingSide);
            }

            ASSERT(chessboard[KING_BLACK] & POW2[G8])
            ASSERT(chessboard[ROOK_BLACK] & POW2[F8])

        } else {

            if (startPosBlackKing != C8) {
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_61) & NOTPOW2[startPosBlackKing];
                updateZobristKey(KING_BLACK, C8);
                updateZobristKey(KING_BLACK, startPosBlackKing);
            }
            if (startPosBlackRookQueenSide != D8) {
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_60) & NOTPOW2[startPosBlackRookQueenSide];
                updateZobristKey(ROOK_BLACK, D8);
                updateZobristKey(ROOK_BLACK, startPosBlackRookQueenSide);
            }
            ASSERT(chessboard[KING_BLACK] & POW2[C8])
            ASSERT(chessboard[ROOK_BLACK] & POW2[D8])
        }
    }
    ASSERT(chessboard[KING_WHITE])
    ASSERT(chessboard[KING_BLACK])
}

bool GenMoves::allowCastleBlackQueen(const u64 allpieces) const {
    return POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK &&
           !(allpieces & 0x7000000000000000ULL) && chessboard[ROOK_BLACK] & POW2_63 &&
           !board::isAttacked<BLACK>(59, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(60, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(61, allpieces, chessboard);
}

bool GenMoves::allowCastleWhiteQueen(const u64 allpieces) const {
    return POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x70ULL) &&
           chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_7 &&
           !board::isAttacked<WHITE>(3, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(4, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(5, allpieces, chessboard);
}

bool GenMoves::allowCastleBlackKing(const u64 allpieces) const {
    return POW2_59 & chessboard[KING_BLACK] && chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK &&
           !(allpieces & 0x600000000000000ULL) && chessboard[ROOK_BLACK] & POW2_56 &&
           !board::isAttacked<BLACK>(57, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(58, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(59, allpieces, chessboard);
}

bool GenMoves::allowCastleWhiteKing(const u64 allpieces) const {
    return POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x6ULL) &&
           chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_0 &&
           !board::isAttacked<WHITE>(1, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(2, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(3, allpieces, chessboard);
}

void GenMoves::unPerformCastle(const int side, const uchar type) {
    ASSERT_RANGE(side, 0, 1)
    ASSERT(chessboard[KING_WHITE])
    ASSERT(chessboard[KING_BLACK])
    if (side == WHITE) {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            ASSERT(board::getPieceAt(side, POW2_1, chessboard) == KING_WHITE)
            ASSERT(board::getPieceAt(side, POW2_2, chessboard) == ROOK_WHITE)
            if (startPosWhiteKing != 1)
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2[startPosWhiteKing]) & NOTPOW2_1;
            if (startPosWhiteRookKingSide != F1)
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2[startPosWhiteRookKingSide]) & NOTPOW2_2;
        } else {
            ASSERT(board::getPieceAt(side, POW2_5, chessboard) == KING_WHITE)
            ASSERT(board::getPieceAt(side, POW2_4, chessboard) == ROOK_WHITE)
            if (startPosWhiteKing != C1)
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2[startPosWhiteKing]) & NOTPOW2_5;
            if (startPosWhiteRookQueenSide != D1)
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2[startPosWhiteRookQueenSide]) & NOTPOW2_4;
        }

        ASSERT(chessboard[KING_WHITE] & POW2[startPosWhiteKing])
    } else {

        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            ASSERT(board::getPieceAt(side, POW2_57, chessboard) == KING_BLACK)
            ASSERT(board::getPieceAt(side, POW2_58, chessboard) == ROOK_BLACK)
            if (startPosBlackKing != G8)
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2[startPosBlackKing]) & NOTPOW2_57;
            if (startPosBlackRookKingSide != F8)
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2[startPosBlackRookKingSide]) & NOTPOW2_58;
        } else {
            ASSERT(board::getPieceAt(side, POW2_61, chessboard) == KING_BLACK)
            ASSERT(board::getPieceAt(side, POW2_60, chessboard) == ROOK_BLACK)
            if (startPosBlackKing != C8)
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2[startPosBlackKing]) & NOTPOW2_61;
            if (startPosBlackRookQueenSide != D8)
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2[startPosBlackRookQueenSide]) & NOTPOW2_60;
        }
        ASSERT(chessboard[KING_BLACK] & POW2[startPosBlackKing])
    }

}

void GenMoves::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    if (rep) {
        popStackMove();
    }
    chessboard[ZOBRISTKEY_IDX] = oldkey;
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    int pieceFrom, posTo, posFrom, movecapture;
    chessboard[RIGHT_CASTLE_IDX] = move->s.type & 0xf0;
    if ((move->s.type & 0x3) == STANDARD_MOVE_MASK || (move->s.type & 0x3) == ENPASSANT_MOVE_MASK) {
        posTo = move->s.to;
        posFrom = move->s.from;
        movecapture = move->s.capturedPiece;
        ASSERT_RANGE(posFrom, 0, 63)
        ASSERT_RANGE(posTo, 0, 63)
        pieceFrom = move->s.pieceFrom;
        chessboard[pieceFrom] = (chessboard[pieceFrom] & NOTPOW2[posTo]) | POW2[posFrom];
        if (movecapture != SQUARE_EMPTY) {
            if (((move->s.type & 0x3) != ENPASSANT_MOVE_MASK)) {
                chessboard[movecapture] |= POW2[posTo];
            } else {
                ASSERT(movecapture == (move->s.side ^ 1))
                if (move->s.side) {
                    chessboard[movecapture] |= POW2[posTo - 8];
                } else {
                    chessboard[movecapture] |= POW2[posTo + 8];
                }
            }
        }
    } else if ((move->s.type & 0x3) == PROMOTION_MOVE_MASK) {
        posTo = move->s.to;
        posFrom = move->s.from;
        movecapture = move->s.capturedPiece;
        ASSERT(posTo >= 0 && move->s.side >= 0 && move->s.promotionPiece >= 0)
        chessboard[(uchar) move->s.side] |= POW2[posFrom];
        chessboard[(uchar) move->s.promotionPiece] &= NOTPOW2[posTo];
        if (movecapture != SQUARE_EMPTY) {
            chessboard[movecapture] |= POW2[posTo];
        }
    } else if (move->s.type & 0xc) { //castle
        unPerformCastle(move->s.side, move->s.type);
    }
}


bool GenMoves::makemove(const _Tmove *move, const bool rep, const bool checkInCheck) {
    ASSERT(move)
    ASSERT(bitCount(chessboard[KING_WHITE]) == 1 && bitCount(chessboard[KING_BLACK]) == 1)
    int pieceFrom = SQUARE_EMPTY, posTo, posFrom, movecapture = SQUARE_EMPTY;
    const uchar rightCastleOld = chessboard[RIGHT_CASTLE_IDX];

    if (!(move->s.type & 0xc)) { //no castle
        posTo = move->s.to;
        posFrom = move->s.from;
        movecapture = move->s.capturedPiece;
        ASSERT_RANGE(posFrom, 0, 63)
        ASSERT_RANGE(posTo, 0, 63)
        pieceFrom = move->s.pieceFrom;
        if ((move->s.type & 0x3) == PROMOTION_MOVE_MASK) {
            chessboard[pieceFrom] &= NOTPOW2[posFrom];
            updateZobristKey(pieceFrom, posFrom);
            ASSERT(move->s.promotionPiece >= 0)
            chessboard[(uchar) move->s.promotionPiece] |= POW2[posTo];
            updateZobristKey((uchar) move->s.promotionPiece, posTo);
        } else {
            chessboard[pieceFrom] = (chessboard[pieceFrom] | POW2[posTo]) & NOTPOW2[posFrom];
            updateZobristKey(pieceFrom, posFrom);
            updateZobristKey(pieceFrom, posTo);
        }
        if (movecapture != SQUARE_EMPTY) {
            if ((move->s.type & 0x3) != ENPASSANT_MOVE_MASK) {
                chessboard[movecapture] &= NOTPOW2[posTo];
                updateZobristKey(movecapture, posTo);
            } else { //en passant
                ASSERT(movecapture == (move->s.side ^ 1))
                if (move->s.side) {
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
            case KING_WHITE:
                chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
                break;
            case KING_BLACK:
                chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
                break;
            case ROOK_WHITE:
                if (posFrom == startPosWhiteRookKingSide)
                    chessboard[RIGHT_CASTLE_IDX] &= 0xef;
                else if (posFrom == startPosWhiteRookQueenSide)
                    chessboard[RIGHT_CASTLE_IDX] &= 0xdf;
                break;
            case ROOK_BLACK:
                if (posFrom == startPosBlackRookKingSide)
                    chessboard[RIGHT_CASTLE_IDX] &= 0xbf;
                else if (posFrom == startPosBlackRookQueenSide)
                    chessboard[RIGHT_CASTLE_IDX] &= 0x7f;
                break;
                //en passant
            case PAWN_WHITE:
                if ((RANK_2 & POW2[posFrom]) && (RANK_3 & POW2[posTo])) {
                    chessboard[ENPASSANT_IDX] = posTo;
                    updateZobristKey(13, chessboard[ENPASSANT_IDX]);
                }
                break;
            case PAWN_BLACK:
                if ((RANK_7 & POW2[posFrom]) && (RANK_5 & POW2[posTo])) {
                    chessboard[ENPASSANT_IDX] = posTo;
                    updateZobristKey(13, chessboard[ENPASSANT_IDX]);
                }
                break;
            default:;
        }
    } else { //castle

        performCastle(move->s.side, move->s.type);
        if (move->s.side == WHITE) {
            chessboard[RIGHT_CASTLE_IDX] &= 0xcf;
        } else {
            chessboard[RIGHT_CASTLE_IDX] &= 0x3f;
        }
    }

    for (u64 x2 = rightCastleOld ^chessboard[RIGHT_CASTLE_IDX]; x2; RESET_LSB(x2)) {
        const int position = BITScanForward(x2);
        updateZobristKey(14, position);
    }
    if (rep) {
        if (movecapture != SQUARE_EMPTY || pieceFrom == WHITE || pieceFrom == BLACK || move->s.type & 0xc) {
            pushStackMove(0);
        }
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }
    if ((forceCheck || (checkInCheck && !perftMode)) &&
        ((move->s.side == WHITE && board::inCheck1<WHITE>(chessboard)) ||
         (move->s.side == BLACK && board::inCheck1<BLACK>(chessboard)))) {
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

u64 GenMoves::getTotMoves() const {
    return numMoves + numMovesq;
}

void GenMoves::setRepetitionMapCount(const int i) {
    repetitionMapCount = i;
}

int GenMoves::loadFen(string fen) {
    int side = ChessBoard::loadFen(fen);
    if (side == 2) {
        fatal("Bad FEN position format ", fen)
        std::_Exit(1);
    }

    return side;
}

int GenMoves::getMoveFromSan(const string fenStr, _Tmove *move) {
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    memset(move, 0, sizeof(_Tmove));
    movesCount++;
    if (
            (((MATCH_QUEENSIDE_WHITE.find(fenStr) != string::npos &&
               allowQueenSideWhite(board::getBitmap(chessboard))) ||
              (MATCH_KINGSIDE_WHITE.find(fenStr) != string::npos
               && allowKingSideWhite(board::getBitmap(chessboard))))
            )
            ||
            (((MATCH_QUEENSIDE_BLACK.find(fenStr) != string::npos &&
               allowQueenSideBlack(board::getBitmap(chessboard))) ||
              (MATCH_KINGSIDE_BLACK.find(fenStr) != string::npos
               && allowKingSideBlack(board::getBitmap(chessboard))))
            )
            ) {
        if (MATCH_QUEENSIDE.find(fenStr) != string::npos) {
            move->s.type = QUEEN_SIDE_CASTLE_MOVE_MASK;
            move->s.from = QUEEN_SIDE_CASTLE_MOVE_MASK;
        } else {
            move->s.from = KING_SIDE_CASTLE_MOVE_MASK;
            move->s.type = KING_SIDE_CASTLE_MOVE_MASK;
        }
        if (fenStr.find("1") != string::npos) {
            move->s.side = WHITE;
        } else if (fenStr.find("8") != string::npos) {
            move->s.side = BLACK;
        } else {
            _assert(0)
        }
        move->s.from = -1;
        move->s.capturedPiece = SQUARE_EMPTY;
        return move->s.side;
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
        _assert(0)
    }
    for (int i = 0; i < 64; i++) {
        if (!fenStr.compare(2, 2, BOARD[i])) {
            to = i;
            break;
        }
    }
    if (to == -1) {
        cout << fenStr << endl;
        _assert(0)
    }
    int pieceFrom;
    if ((pieceFrom = board::getPieceAt<WHITE>(POW2[from], chessboard)) != 12) {
        move->s.side = WHITE;
    } else if ((pieceFrom = board::getPieceAt<BLACK>(POW2[from], chessboard)) != 12) {
        move->s.side = BLACK;
    } else {
        display();
        cout << "fenStr: >" << fenStr << "< from: " << from << endl;
        _assert(0)
    }
    move->s.from = from;
    move->s.to = to;
    if (fenStr.length() == 4) {
        move->s.type = STANDARD_MOVE_MASK;
        if (pieceFrom == PAWN_WHITE || pieceFrom == PAWN_BLACK) {
            if (FILE_AT[from] != FILE_AT[to] &&
                (move->s.side ^ 1 ? board::getPieceAt<WHITE>(POW2[to], chessboard) : board::getPieceAt<BLACK>(POW2[to],
                                                                                                              chessboard)) ==
                SQUARE_EMPTY) {
                move->s.type = ENPASSANT_MOVE_MASK;
            }
        }
    } else if (fenStr.length() == 5) {
        move->s.type = PROMOTION_MOVE_MASK;
        if (move->s.side == WHITE) {
            move->s.promotionPiece = INV_FEN[toupper(fenStr.at(4))];
        } else {
            move->s.promotionPiece = INV_FEN[(uchar) fenStr.at(4)];
        }
        ASSERT(move->s.promotionPiece != -1)
    }
    if (move->s.side == WHITE) {
        move->s.capturedPiece = board::getPieceAt<BLACK>(POW2[move->s.to], chessboard);
        move->s.pieceFrom = board::getPieceAt<WHITE>(POW2[move->s.from], chessboard);
    } else {
        move->s.capturedPiece = board::getPieceAt<WHITE>(POW2[move->s.to], chessboard);
        move->s.pieceFrom = board::getPieceAt<BLACK>(POW2[move->s.from], chessboard);
    }
    if (move->s.type == ENPASSANT_MOVE_MASK) {
        move->s.capturedPiece = !move->s.side;
    }
    return move->s.side;
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

        if (bitCount(check) == (2 + (int) pieces.size()) && !board::inCheck1<WHITE>(chessboard) &&
            !board::inCheck1<BLACK>(chessboard) &&
            !(0xff000000000000ffULL & (chessboard[0] | chessboard[1]))) {
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

    for (unsigned k = 0; k < TOT; k++) {
        int side = WHITE;
        pieces.clear();
        _assert(toupper(type.at(0)) == 'K')
        for (unsigned i = 1; i < type.size(); i++) {
            const char up = toupper(type.at(i));
            if (!(up == 'K' || up == 'R' || up == 'P' || up == 'Q' || up == 'B' || up == 'N')) {
                cout << "format error" << endl;
                return false;
            }
            if (up == 'K') {
                side = BLACK;
            } else {
                auto x = PIECES[type.at(i)] + side;
                pieces.push_back(x);
            }
        }

        writeRandomFen(pieces);
    }
    return true;
}
