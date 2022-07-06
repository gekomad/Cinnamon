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

GenMoves::GenMoves() : perftMode(false), listId(-1) {
    currentPly = 0;
    genList = (_TmoveP *) calloc(MAX_PLY, sizeof(_TmoveP));
    _assert(genList)
    for (int i = 0; i < MAX_PLY; i++) {
        genList[i].moveList = (_Tmove *) calloc(MAX_MOVE, sizeof(_Tmove));
        _assert(genList[i].moveList)
    }
    repetitionMap = (u64 *) malloc(sizeof(u64) * MAX_REP_COUNT);
    _assert(repetitionMap)
    repetitionMapCount = 0;
    init();
}

void GenMoves::generateMoves(const uchar side, const u64 allpieces) {
    ASSERT_RANGE(side, 0, 1)
    side ? generateMoves<WHITE>(allpieces) : generateMoves<BLACK>(allpieces);
}

bool GenMoves::generateCaptures(const uchar side, const u64 enemies, const u64 friends) {
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

_Tmove *GenMoves::getNextMoveQ(_TmoveP *list, const int first) {
    BENCH_AUTO_CLOSE("getNextMoveQ")
    int bestId = -1;
    int bestScore = -INT_MAX;

    for (int i = first; i < list->size; i++) {
        const auto move = list->moveList[i];
        ASSERT_RANGE(move.pieceFrom, 0, 11)
        ASSERT_RANGE(move.to, 0, 63)
        ASSERT_RANGE(move.from, 0, 63)

        const int score = CAPTURES[move.pieceFrom][move.capturedPiece];
        if (score > bestScore) {
            bestScore = score;
            bestId = i;
        }
    }
    if (bestId == -1) {
        return nullptr;
    }
    return swap(list, first, bestId);
}

_Tmove *GenMoves::getNextMove(_TmoveP *list, const int depth, const u64 &hash, const int first) {
    BENCH_AUTO_CLOSE("getNextMove")
    int bestId = -1;
    int bestScore = -1;

    for (int i = first; i < list->size; i++) {
        const auto move = list->moveList[i];
        int score = 0;
        if (move.type & 0x3) {

            ASSERT_RANGE(move.pieceFrom, 0, 11)
            ASSERT_RANGE(move.to, 0, 63)
            ASSERT_RANGE(move.from, 0, 63)

            if (GET_FROM(hash) == move.from && GET_TO(hash) == move.to) {
                return swap(list, first, i);
            }
            score += historyHeuristic[move.from][move.to];
            score += CAPTURES[move.pieceFrom][move.capturedPiece];

            if (isKiller(0, move.from, move.to, depth)) score += 50;
            else if (isKiller(1, move.from, move.to, depth)) score += 30;

        } else if (move.type & 0xc) {    //castle
            assert(rightCastle);
            score = 100;
        }
        if (score > bestScore) {
            bestScore = score;
            bestId = i;
        }
    }
    if (bestId == -1) {
        return nullptr;
    }
    return swap(list, first, bestId);
}

GenMoves::~GenMoves() {
    for (int i = 0; i < MAX_PLY; i++) {
        free(genList[i].moveList);
    }
    free(genList);
    free(repetitionMap);
}

void GenMoves::performCastle(const uchar side, const uchar type) {
    ASSERT_RANGE(side, 0, 1)

    if (side == WHITE) {
        assert(chessboard[KING_WHITE] & POW2(startPosWhiteKing));
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            if (startPosWhiteKing != G1) {
                updateZobristKey(KING_WHITE, startPosWhiteKing);
                updateZobristKey(KING_WHITE, G1);
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_1) & NOTPOW2(startPosWhiteKing);
            }
            if (startPosWhiteRookKingSide != F1) {
                updateZobristKey(ROOK_WHITE, F1);
                updateZobristKey(ROOK_WHITE, startPosWhiteRookKingSide);

                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_2) & NOTPOW2(startPosWhiteRookKingSide);
            }

            assert(chessboard[KING_WHITE] & POW2(G1));
            assert(chessboard[ROOK_WHITE] & POW2(F1));

        } else {
            if (startPosWhiteKing != C1) {
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2_5) & NOTPOW2(startPosWhiteKing);
                updateZobristKey(KING_WHITE, C1);
                updateZobristKey(KING_WHITE, startPosWhiteKing);
            }
            if (startPosWhiteRookQueenSide != D1) {
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2_4) & NOTPOW2(startPosWhiteRookQueenSide);
                updateZobristKey(ROOK_WHITE, D1);
                updateZobristKey(ROOK_WHITE, startPosWhiteRookQueenSide);
            }
            assert(chessboard[KING_WHITE] & POW2(C1));
            assert(chessboard[ROOK_WHITE] & POW2(D1));

        }
    } else {
        assert(chessboard[KING_BLACK] & POW2(startPosBlackKing));

        if (type & KING_SIDE_CASTLE_MOVE_MASK) {

            if (startPosBlackKing != G8) {
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_57) & NOTPOW2(startPosBlackKing);
                updateZobristKey(KING_BLACK, G8);
                updateZobristKey(KING_BLACK, startPosBlackKing);
            }
            if (startPosBlackRookKingSide != F8) {
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_58) & NOTPOW2(startPosBlackRookKingSide);
                updateZobristKey(ROOK_BLACK, F8);
                updateZobristKey(ROOK_BLACK, startPosBlackRookKingSide);
            }

            assert(chessboard[KING_BLACK] & POW2(G8));
            assert(chessboard[ROOK_BLACK] & POW2(F8));

        } else {

            if (startPosBlackKing != C8) {
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2_61) & NOTPOW2(startPosBlackKing);
                updateZobristKey(KING_BLACK, C8);
                updateZobristKey(KING_BLACK, startPosBlackKing);
            }
            if (startPosBlackRookQueenSide != D8) {
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2_60) & NOTPOW2(startPosBlackRookQueenSide);
                updateZobristKey(ROOK_BLACK, D8);
                updateZobristKey(ROOK_BLACK, startPosBlackRookQueenSide);
            }
            assert(chessboard[KING_BLACK] & POW2(C8));
            assert(chessboard[ROOK_BLACK] & POW2(D8));
        }
    }
    assert(chessboard[KING_WHITE]);
    assert(chessboard[KING_BLACK]);
}

bool GenMoves::allowCastleBlackQueen(const u64 allpieces) const {
    return POW2_59 & chessboard[KING_BLACK] && rightCastle & RIGHT_QUEEN_CASTLE_BLACK_MASK &&
           !(allpieces & 0x7000000000000000ULL) && chessboard[ROOK_BLACK] & POW2_63 &&
           !board::isAttacked<BLACK>(59, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(60, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(61, allpieces, chessboard);
}

bool GenMoves::allowCastleWhiteQueen(const u64 allpieces) const {
    return POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x70ULL) &&
           rightCastle & RIGHT_QUEEN_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_7 &&
           !board::isAttacked<WHITE>(3, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(4, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(5, allpieces, chessboard);
}

bool GenMoves::allowCastleBlackKing(const u64 allpieces) const {
    return POW2_59 & chessboard[KING_BLACK] && rightCastle & RIGHT_KING_CASTLE_BLACK_MASK &&
           !(allpieces & 0x600000000000000ULL) && chessboard[ROOK_BLACK] & POW2_56 &&
           !board::isAttacked<BLACK>(57, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(58, allpieces, chessboard) &&
           !board::isAttacked<BLACK>(59, allpieces, chessboard);
}

bool GenMoves::allowCastleWhiteKing(const u64 allpieces) const {
    return POW2_3 & chessboard[KING_WHITE] && !(allpieces & 0x6ULL) &&
           rightCastle & RIGHT_KING_CASTLE_WHITE_MASK && chessboard[ROOK_WHITE] & POW2_0 &&
           !board::isAttacked<WHITE>(1, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(2, allpieces, chessboard) &&
           !board::isAttacked<WHITE>(3, allpieces, chessboard);
}

void GenMoves::unPerformCastle(const uchar side, const uchar type) {
    ASSERT_RANGE(side, 0, 1)
    assert(chessboard[KING_WHITE]);
    assert(chessboard[KING_BLACK]);
    if (side == WHITE) {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            assert(board::getPieceAt(side, POW2_1, chessboard) == KING_WHITE);
            assert(board::getPieceAt(side, POW2_2, chessboard) == ROOK_WHITE);
            if (startPosWhiteKing != 1)
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2(startPosWhiteKing)) & NOTPOW2_1;
            if (startPosWhiteRookKingSide != F1)
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2(startPosWhiteRookKingSide)) & NOTPOW2_2;
        } else {
            assert(board::getPieceAt(side, POW2_5, chessboard) == KING_WHITE);
            assert(board::getPieceAt(side, POW2_4, chessboard) == ROOK_WHITE);
            if (startPosWhiteKing != C1)
                chessboard[KING_WHITE] = (chessboard[KING_WHITE] | POW2(startPosWhiteKing)) & NOTPOW2_5;
            if (startPosWhiteRookQueenSide != D1)
                chessboard[ROOK_WHITE] = (chessboard[ROOK_WHITE] | POW2(startPosWhiteRookQueenSide)) & NOTPOW2_4;
        }
        assert(chessboard[KING_WHITE] & POW2(startPosWhiteKing));
    } else {
        if (type & KING_SIDE_CASTLE_MOVE_MASK) {
            assert(board::getPieceAt(side, POW2_57, chessboard) == KING_BLACK);
            assert(board::getPieceAt(side, POW2_58, chessboard) == ROOK_BLACK);
            if (startPosBlackKing != G8)
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2(startPosBlackKing)) & NOTPOW2_57;
            if (startPosBlackRookKingSide != F8)
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2(startPosBlackRookKingSide)) & NOTPOW2_58;
        } else {
            assert(board::getPieceAt(side, POW2_61, chessboard) == KING_BLACK);
            assert(board::getPieceAt(side, POW2_60, chessboard) == ROOK_BLACK);
            if (startPosBlackKing != C8)
                chessboard[KING_BLACK] = (chessboard[KING_BLACK] | POW2(startPosBlackKing)) & NOTPOW2_61;
            if (startPosBlackRookQueenSide != D8)
                chessboard[ROOK_BLACK] = (chessboard[ROOK_BLACK] | POW2(startPosBlackRookQueenSide)) & NOTPOW2_60;
        }
        assert(chessboard[KING_BLACK] & POW2(startPosBlackKing));
    }

}

void GenMoves::takeback(const _Tmove *move, const u64 oldkey, const uchar oldEnpassant, const bool rep) {
    BENCH_AUTO_CLOSE("takeback")
    if (rep) popStackMove();
    chessboard[ZOBRISTKEY_IDX] = oldkey;
    enPassant = oldEnpassant;
    rightCastle = move->type & 0xf0;

    if (move->type & 0x1) {
        ASSERT_RANGE(move->from, 0, 63)
        ASSERT_RANGE(move->to, 0, 63)
        chessboard[move->pieceFrom] = (chessboard[move->pieceFrom] & NOTPOW2(move->to)) | POW2(move->from);
        if (move->capturedPiece != SQUARE_EMPTY) {
            if (((move->type & 0x3) != ENPASSANT_MOVE_MASK)) chessboard[move->capturedPiece] |= POW2(move->to);
            else {
                assert(move->capturedPiece == X(move->side));
                if (move->side) chessboard[move->capturedPiece] |= POW2(move->to - 8);
                else chessboard[move->capturedPiece] |= POW2(move->to + 8);
            }
        }
    } else if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
        assert(move->to >= 0 && move->side >= 0 && move->promotionPiece >= 0);
        chessboard[move->side] |= POW2(move->from);
        chessboard[move->promotionPiece] &= NOTPOW2(move->to);
        if (move->capturedPiece != SQUARE_EMPTY) chessboard[move->capturedPiece] |= POW2(move->to);
    } else {
        //castle
        assert(move->type & 0xc);
        unPerformCastle(move->side, move->type);
    }
}


bool GenMoves::makemove(const _Tmove *move, const bool rep) {
    BENCH_AUTO_CLOSE("makemove")
    assert(move);
    assert(bitCount(chessboard[KING_WHITE]) == 1 && bitCount(chessboard[KING_BLACK]) == 1);
    assert(verifyMove(move));
    const uchar rightCastleOld = rightCastle;
    if (!(move->type & 0xc)) { //no castle

        ASSERT_RANGE(move->from, 0, 63)
        ASSERT_RANGE(move->to, 0, 63)
        if ((move->type & 0x3) == PROMOTION_MOVE_MASK) {
            chessboard[move->pieceFrom] &= NOTPOW2(move->from);
            updateZobristKey(move->pieceFrom, move->from);
            assert(move->promotionPiece >= 0);
            chessboard[move->promotionPiece] |= POW2(move->to);
            updateZobristKey(move->promotionPiece, move->to);
        } else {
            chessboard[move->pieceFrom] = (chessboard[move->pieceFrom] | POW2(move->to)) & NOTPOW2(move->from);
            updateZobristKey(move->pieceFrom, move->from);
            updateZobristKey(move->pieceFrom, move->to);
        }
        if (move->capturedPiece != SQUARE_EMPTY) {
            if ((move->type & 0x3) != ENPASSANT_MOVE_MASK) {
                assert(chessboard[move->capturedPiece] & POW2(move->to));
                chessboard[move->capturedPiece] &= NOTPOW2(move->to);
                updateZobristKey(move->capturedPiece, move->to);
            } else { //en passant
                assert(move->capturedPiece == X(move->side));
                const int y = move->side ? -8 : 8;
                chessboard[move->capturedPiece] &= NOTPOW2(move->to + y);
                updateZobristKey(move->capturedPiece, move->to + y);
            }
        }
        //lost castle right
        if (POW2(move->to) & 0xff000000000000ffULL) {
            if (move->to == startPosWhiteRookKingSide) rightCastle &= 0xef;
            else if (move->to == startPosWhiteRookQueenSide) rightCastle &= 0xdf;
            else if (move->to == startPosBlackRookKingSide) rightCastle &= 0xbf;
            else if (move->to == startPosBlackRookQueenSide) rightCastle &= 0x7f;
        }
        if (move->pieceFrom & 0xb) {
            switch (move->pieceFrom) {
                case KING_WHITE:
                    rightCastle &= 0xcf;
                    break;
                case KING_BLACK:
                    rightCastle &= 0x3f;
                    break;
                case ROOK_WHITE:
                    if (move->from == startPosWhiteRookKingSide)
                        rightCastle &= 0xef;
                    else if (move->from == startPosWhiteRookQueenSide)
                        rightCastle &= 0xdf;
                    break;
                case ROOK_BLACK:
                    if (move->from == startPosBlackRookKingSide)
                        rightCastle &= 0xbf;
                    else if (move->from == startPosBlackRookQueenSide)
                        rightCastle &= 0x7f;
                    break;
                default:;
            }
            //en passant
        }
        switch (move->pieceFrom) {
            case PAWN_WHITE:
                if ((RANK_2 & POW2(move->from)) && (RANK_3 & POW2(move->to))) {
                    enPassant = move->to;
                    updateZobristKey(ENPASSANT_RAND, enPassant);
                }
                break;
            case PAWN_BLACK:
                if ((RANK_7 & POW2(move->from)) && (RANK_5 & POW2(move->to))) {
                    enPassant = move->to;
                    updateZobristKey(ENPASSANT_RAND, enPassant);
                }
                break;
            default:;
        }

    } else { //castle
        performCastle(move->side, move->type);
        if (move->side == WHITE) rightCastle &= 0xcf; else rightCastle &= 0x3f;
    }

    for (u64 x2 = rightCastleOld ^ rightCastle; x2; RESET_LSB(x2)) {
        const int position = BITScanForward(x2);
        updateZobristKey(14, position);
    }
    if (rep) {
        if (move->capturedPiece != SQUARE_EMPTY || move->pieceFrom == WHITE || move->pieceFrom == BLACK ||
            move->type & 0xc) {
            pushStackMove(0);
        }
        pushStackMove(chessboard[ZOBRISTKEY_IDX]);
    }
    if ((forceCheck || !perftMode) &&
        ((move->side == WHITE && board::inCheck1<WHITE>(chessboard)) ||
         (move->side == BLACK && board::inCheck1<BLACK>(chessboard)))) {
        return false;
    }
    return true;
}

void GenMoves::init() {
    numMoves = numMovesq = listId = 0;
    DEBUG(nCutFp = nCutRazor = nCutAB = nNullMoveCut = nCutBadCaputure = 0)
}

u64 GenMoves::getTotMoves() const {
    return numMoves + numMovesq;
}

void GenMoves::setRepetitionMapCount(const int i) {
    repetitionMapCount = i;
}

int GenMoves::getMoveFromSan(const string &fenStr, _Tmove *move) {
    enPassant = NO_ENPASSANT;
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
            _assert(0)
        }
        move->from = -1;
        move->capturedPiece = SQUARE_EMPTY;
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
    if ((pieceFrom = board::getPieceAt<WHITE>(POW2(from), chessboard)) != 12) {
        move->side = WHITE;
    } else if ((pieceFrom = board::getPieceAt<BLACK>(POW2(from), chessboard)) != 12) {
        move->side = BLACK;
    } else {
        display();
        cout << "fenStr: >" << fenStr << "< from: " << from << endl;
        _assert(0)
    }
    move->from = from;
    move->to = to;
    if (fenStr.length() == 4) {
        move->type = STANDARD_MOVE_MASK;
        if (pieceFrom == PAWN_WHITE || pieceFrom == PAWN_BLACK) {
            if (FILE_AT[from] != FILE_AT[to] &&
                (X(move->side) ? board::getPieceAt<WHITE>(POW2(to), chessboard) : board::getPieceAt<BLACK>(POW2(to),
                                                                                                           chessboard)) ==
                SQUARE_EMPTY) {
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
        assert(move->promotionPiece != NO_PROMOTION);
    }
    if (move->side == WHITE) {
        move->capturedPiece = board::getPieceAt<BLACK>(POW2(move->to), chessboard);
        move->pieceFrom = board::getPieceAt<WHITE>(POW2(move->from), chessboard);
    } else {
        move->capturedPiece = board::getPieceAt<WHITE>(POW2(move->to), chessboard);
        move->pieceFrom = board::getPieceAt<BLACK>(POW2(move->from), chessboard);
    }
    if (move->type == ENPASSANT_MOVE_MASK) {
        move->capturedPiece = !move->side;
    }
    return move->side;
}

void GenMoves::writeRandomFen(const vector<int> pieces) {
    while (1) {
        clearChessboard();
        sideToMove = rand() % 2;
        rightCastle = 0;
        chessboard[KING_BLACK] = POW2(rand() % 64);
        chessboard[KING_WHITE] = POW2(rand() % 64);
        u64 check = chessboard[KING_BLACK] | chessboard[KING_WHITE];
        for (unsigned long i = 0; i < pieces.size(); i++) {
            chessboard[pieces[i]] |= POW2(rand() % 64);
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
        uchar side = WHITE;
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

#ifndef NDEBUG

bool GenMoves::verifyMove(const _Tmove *move) {
    const int side = move->side;
    if (!(move->type & 0xc)) { // no castle
        const bool from = move->pieceFrom == (PAWN_BLACK + side) || move->pieceFrom == (KNIGHT_BLACK + side) ||
                          move->pieceFrom == (QUEEN_BLACK + side) || move->pieceFrom == (BISHOP_BLACK + side) ||
                          move->pieceFrom == (ROOK_BLACK + side) || move->pieceFrom == (KING_BLACK + side);
        if (!from) {
            cout << "ERROR *************************" << endl;
            print(move);
            cout << endl << "ERROR *************************" << endl;
            return false;
        }
        if (move->capturedPiece != SQUARE_EMPTY) {
            const int xside = X(side);
            const bool cap =
                    move->capturedPiece == (PAWN_BLACK + xside) || move->capturedPiece == (KNIGHT_BLACK + xside) ||
                    move->capturedPiece == (QUEEN_BLACK + xside) || move->capturedPiece == (BISHOP_BLACK + xside) ||
                    move->capturedPiece == (ROOK_BLACK + xside) || move->capturedPiece == (KING_BLACK + xside);
            if (!cap) {
                cout << "ERROR *************************" << endl;
                print(move);
                cout << endl << "ERROR *************************" << endl;
                return false;
            }
        }
    }
    return true;
}

#endif
