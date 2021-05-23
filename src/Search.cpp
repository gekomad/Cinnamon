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

#include "Search.h"

bool volatile Search::runningThread;
high_resolution_clock::time_point Search::startTime;

DEBUG(unsigned Search::cumulativeMovesCount)

void Search::run() {
    if (getRunning()) {
        if (searchMovesVector.size())
            aspirationWindow<true>(mainDepth, valWindow);
        else
            aspirationWindow<false>(mainDepth, valWindow);
    }
}

template<bool searchMoves>
void Search::aspirationWindow(const int depth, const int valWin) {
    valWindow = valWin;
    init();

    if (depth == 1) {
        valWindow = searchRoot<searchMoves>(depth, -_INFINITE - 1, _INFINITE + 1);
    } else {
        int tmp = searchRoot<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);

        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = searchRoot<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW);
            } else {
                tmp = searchRoot<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2);
            }

            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = searchRoot<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW);
                } else {
                    tmp = searchRoot<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4);
                }

                if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                    tmp = searchRoot<searchMoves>(mainDepth, -_INFINITE - 1, _INFINITE + 1);
                }
            }
        }
        if (getRunning()) {
            valWindow = tmp;
        }
    }
}

Search::Search() : ponder(false), nullSearch(false) {

    DEBUG(eval.lazyEvalCuts = cumulativeMovesCount = totGen = 0)

}

void Search::clone(const Search *s) {
    memcpy(chessboard, s->chessboard, sizeof(_Tchessboard));
}

#ifndef JS_MODE

int Search::SZtbProbeWDL() const {
    const auto tot = bitCount(board::getBitmap<WHITE>(chessboard) | board::getBitmap<BLACK>(chessboard));
    return syzygy->SZtbProbeWDL(chessboard, sideToMove, tot);
}

void Search::printWdlSyzygy() {
    perftMode = true;
    u64 friends = sideToMove == WHITE ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    u64 enemies = sideToMove == BLACK ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);

    incListId();
    generateCaptures(sideToMove, enemies, friends);
    generateMoves(sideToMove, friends | enemies);
    _Tmove *move;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;
    display();

    unsigned res = syzygy->SZtbProbeWDL(chessboard, sideToMove);
    cout << "current: ";
    if (res != TB_RESULT_FAILED) {
        res = TB_GET_WDL(res);
        if (res == TB_DRAW)cout << " draw" << endl;
        else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
            cout << " loss" << endl;
        else
            cout << " win" << endl;

    } else
        cout << " none" << endl;

    for (int i = 0; i < getListSize(); i++) {
        move = &genList[listId].moveList[i];
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }
        print(move);

        unsigned res = syzygy->SZtbProbeWDL(chessboard, X(sideToMove));

        if (res != TB_RESULT_FAILED) {
            res = TB_GET_WDL(res);
            if (res == TB_DRAW)cout << " draw" << endl;
            else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
                cout << " win" << endl;
            else
                cout << " loss" << endl;

        } else
            cout << " none" << endl;
        takeback(move, oldKey, oldEnpassant, false);
    }
    cout << endl;
    decListId();
}

void Search::printDtzSyzygy() {
    perftMode = true;

    if ((sideToMove == BLACK) ? board::inCheck1<WHITE>(chessboard) : board::inCheck1<BLACK>(chessboard)) {
        cout << "invalid position" << endl;
        return;
    }

    unsigned results[TB_MAX_MOVES];

    const u64 white = board::getBitmap<WHITE>(chessboard);
    const u64 black = board::getBitmap<BLACK>(chessboard);

    unsigned res = syzygy->SZtbProbeWDL(chessboard, sideToMove);
    display();
    cout << "current: ";
    if (res != TB_RESULT_FAILED) {
        res = TB_GET_WDL(res);
        if (res == TB_DRAW)cout << " draw" << endl;
        else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
            cout << " loss" << endl;
        else
            cout << " win" << endl;

    } else
        cout << " none" << endl;

    const unsigned res1 = syzygy->SZtbProbeRoot(white, black, chessboard, sideToMove, results);
    if (res1 != TB_RESULT_FAILED) {
        for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {

            string from = BOARD[_decodeSquare[TB_GET_FROM(results[i])]];
            string to = BOARD[_decodeSquare[TB_GET_TO(results[i])]];

            cout << from << to;
            auto a = TB_GET_PROMOTES(results[i]);
            if (a)cout << SYZYGY::getPromotion(a);

            cout << " ";
            unsigned dtz = TB_GET_DTZ(results[i]);
            unsigned res = TB_GET_WDL(results[i]);
            if (res == TB_DRAW) cout << " draw" << endl;
            else if (res == TB_LOSS)
                cout << " loss dtz: " << dtz << endl;
            else if (res == TB_BLESSED_LOSS)
                cout << " loss but 50-move draw dtz: " << dtz << endl;
            else if (res == TB_WIN)
                cout << " win dtz: " << dtz << endl;
            else if (res == TB_CURSED_WIN)
                cout << " win but 50-move draw dtz: " << dtz << endl;

        }
        cout << endl;
    }
}


int Search::printDtmWdlGtb(const bool dtm) {
    perftMode = true;

    u64 friends = sideToMove == WHITE ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    u64 enemies = sideToMove == BLACK ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    display();
    cout << "current: ";
    unsigned pliestomate;
    const int res = GTB::getInstance().getDtmWdl(GTB_STM(sideToMove), 2, chessboard, &pliestomate, dtm, rightCastle);
    incListId();
    generateCaptures(sideToMove, enemies, friends);
    generateMoves(sideToMove, friends | enemies);
    _Tmove *move;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;
    for (int i = 0; i < getListSize(); i++) {
        move = &genList[listId].moveList[i];
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }
        print(move);

        GTB::getInstance().getDtmWdl(GTB_STM(X(sideToMove)), 1, chessboard, &pliestomate, dtm, rightCastle);

        takeback(move, oldKey, oldEnpassant, false);

    }
    cout << endl;
    decListId();
    return res;
}

#endif

void Search::setNullMove(const bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Search::setMainPly(const int ply, const int mply) {
    mainDepth = mply;
    this->ply = ply;
}

int Search::checkTime() const {
    if (getRunning() == 2) {
        return 2;
    }
    if (ponder) {
        return 1;
    }
    auto t_current = std::chrono::high_resolution_clock::now();
    return Time::diffTime(t_current, startTime) >= maxTimeMillsec ? 0 : 1;
}

Search::~Search() {
    join();
}

template<uchar side>
int Search::qsearch(int alpha, const int beta, const uchar promotionPiece, const int depth) {
    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];
    if (!getRunning()) return 0;

    ++numMovesq;
    int score = eval.getScore(chessboard, zobristKeyR, side, alpha, beta);
    if (score > alpha) {
        if (score >= beta) return score;
        alpha = score;
    }

    /// **************Delta Pruning ****************
    bool fprune = false;
    int fscore;
    if ((fscore = score + (promotionPiece == NO_PROMOTION ? VALUEQUEEN : 2 * VALUEQUEEN)) < alpha) {
        fprune = true;
    }
    /// ************ end Delta Pruning *************
    if (score > alpha) alpha = score;

    incListId();

    u64 friends = board::getBitmap<side>(chessboard);
    u64 enemies = board::getBitmap<X(side)>(chessboard);
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        return _INFINITE - (mainDepth + depth);
    }
    if (!getListSize()) {
        --listId;
        return score;
    }
    _Tmove *move;
    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;
    int first = 0;
    if (!(numMoves % 2048)) setRunning(checkTime());
    while ((move = getNextMoveQ(&genList[listId], first++))) {
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }

        if (badCapure<side>(*move, friends | enemies)) {
            INC(nCutBadCaputure);
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }
        /// **************Delta Pruning ****************
        if (fprune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) &&
            fscore + PIECES_VALUE[move->capturedPiece] <= alpha) {
            INC(nCutFp);
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }
        /// ************ end Delta Pruning *************
        int val = -qsearch<X(side)>(-beta, -alpha, move->promotionPiece, depth - 1);
        score = max(score, val);
        takeback(move, oldKey, oldEnpassant, false);
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                return beta;
            }
            alpha = score;
        }
    }
    decListId();
    return score;
}

void Search::setPonder(const bool r) {
    ponder = r;
}

void Search::setRunning(const int r) {
    GenMoves::setRunning(r);
    if (!r) {
        maxTimeMillsec = 0;
    }
}

int Search::getRunning() const {
    if (!runningThread)return 0;
    return GenMoves::getRunning();
}

void Search::setMaxTimeMillsec(const int n) {
    maxTimeMillsec = n;
}

int Search::getMaxTimeMillsec() const {
    return maxTimeMillsec;
}

bool Search::checkDraw(const u64 key) {
    int o = 0;
    int count = 0;
    for (int i = repetitionMapCount - 1; i >= 0; i--) {
        if (repetitionMap[i] == 0) {
            return false;
        }

        //fifty-move rule
        if (++count > 100) {
            return true;
        }

        //Threefold repetition
        if (repetitionMap[i] == key && ++o > 2) {
            return true;
        }
    }
    return false;
}

void Search::setMainParam(const int depth) {
    memset(&pvLine, 0, sizeof(_TpvLine));
    mainDepth = depth;
}

template<bool searchMoves>
int Search::searchRoot(const int depth, const int alpha, const int beta) {
    ASSERT_RANGE(depth, 0, MAX_PLY)
    auto ep = enPassant;
    incListId();

    const u64 friends = (sideToMove == WHITE) ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(
            chessboard);
    const u64 enemies = (sideToMove == WHITE) ? board::getBitmap<BLACK>(chessboard) : board::getBitmap<WHITE>(
            chessboard);
    generateCaptures(sideToMove, enemies, friends);
    generateMoves(sideToMove, friends | enemies);
    int n_root_moves = getListSize();
    decListId();
    enPassant = ep;
    return sideToMove ? search<WHITE, searchMoves>(depth,
                                                   alpha,
                                                   beta,
                                                   &pvLine,
                                                   bitCount(board::getBitmap<WHITE>(chessboard) |
                                                            board::getBitmap<BLACK>(chessboard)),
                                                   n_root_moves)
                      : search<BLACK, searchMoves>(depth, alpha, beta, &pvLine,
                                                   bitCount(board::getBitmap<WHITE>(chessboard) |
                                                            board::getBitmap<BLACK>(chessboard)),
                                                   n_root_moves);
}

bool Search::probeRootTB(_Tmove *res) {
    const u64 white = board::getBitmap<WHITE>(chessboard);
    const u64 black = board::getBitmap<BLACK>(chessboard);
    const auto tot = bitCount(white | black);

    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;

#ifndef JS_MODE
    //gaviota
    if (GTB::getInstance().isInstalledPieces(tot)) {
        u64 friends = sideToMove == WHITE ? white : black;
        u64 enemies = sideToMove == BLACK ? white : black;
        _Tmove *bestMove = nullptr;
        incListId();
        generateCaptures(sideToMove, enemies, friends);
        generateMoves(sideToMove, friends | enemies);

        _Tmove *drawMove = nullptr;
        _Tmove *worstMove = nullptr;

        int minDtz = -1;
        unsigned maxDtzWorst = 1000;

        const u64 allPieces = white | black;

        unsigned dtz;

        for (int i = 0; i < getListSize(); i++) {
            _Tmove *move = &genList[listId].moveList[i];
            if (!makemove(move, false, true)) {
                takeback(move, oldKey, oldEnpassant, false);
                continue;
            }
            BENCH_START("gtbTime")
            const auto res = GTB::getInstance().getDtmWdl(GTB_STM(X(sideToMove)), 0, chessboard, &dtz, true,
                                                          rightCastle);
            BENCH_STOP("gtbTime")
            if (res == TB_WIN && !worstMove && !drawMove) {
                if ((int) dtz > minDtz) {
                    bestMove = move;
                    minDtz = dtz;
                    if (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))
                        minDtz++;
                }
            } else if (res == TB_DRAW) {
                if (!drawMove || (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))) {
                    drawMove = move;
                }
            } else if (res == TB_LOSS) {
                if ((int) dtz < (int) maxDtzWorst) {
                    worstMove = move;
                    maxDtzWorst = dtz;
                    if (move->promotionPiece != NO_PROMOTION || board::isOccupied(move->to, allPieces))
                        maxDtzWorst--;
                }
            }
            takeback(move, oldKey, oldEnpassant, false);
        }

        if (worstMove) {
            debug("worstMove ***********\n")
            memcpy(res, worstMove, sizeof(_Tmove));
            if (res->promotionPiece != NO_PROMOTION)
                res->promotionPiece = FEN_PIECE[res->promotionPiece];
            decListId();
            return true;
        }

        if (drawMove) {
            debug("drawMove ***********\n")
            memcpy(res, drawMove, sizeof(_Tmove));
            if (res->promotionPiece != NO_PROMOTION)res->promotionPiece = FEN_PIECE[res->promotionPiece];
            decListId();
            return true;
        }
        if (bestMove) {
            debug("best ***********\n")
            memcpy(res, bestMove, sizeof(_Tmove));
            if (res->promotionPiece != NO_PROMOTION)
                res->promotionPiece = FEN_PIECE[res->promotionPiece];
            decListId();
            return true;
        }

        decListId();
        return false;
    }

    if (syzygy->getInstalledPieces() >= tot) {

        unsigned bestMove = 0;
        unsigned bestMove50 = 0;
        unsigned worstMove = 0;
        unsigned worstMove50 = 0;
        unsigned drawMove = 0;
        unsigned minDtz = 1000;
        unsigned minDtz50 = 1000;
        int maxDtzWorst = -1;
        int maxDtzWorst50 = -1;
        unsigned results[TB_MAX_MOVES];

        const u64 allPieces = white | black;
        BENCH_START("syzygyTime")
        const auto sz = syzygy->SZtbProbeRoot(white, black, chessboard, sideToMove, results);
        BENCH_STOP("syzygyTime")
        if (sz == TB_RESULT_FAILED) return false;

        for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {

            const unsigned dtz = TB_GET_DTZ(results[i]) * 10;
            const unsigned res = TB_GET_WDL(results[i]);

            if (res == TB_WIN) {
                if (dtz < minDtz) {
                    bestMove = results[i];
                    minDtz = dtz;
                    if (TB_GET_PROMOTES(bestMove) || board::isOccupied(_decodeSquare[TB_GET_TO(bestMove)], allPieces))
                        minDtz--;
                }
            } else if (res == TB_CURSED_WIN && !bestMove) {
                if (dtz < minDtz50) {
                    bestMove50 = results[i];
                    minDtz50 = dtz;
                    if (TB_GET_PROMOTES(bestMove50) ||
                        board::isOccupied(_decodeSquare[TB_GET_TO(bestMove50)], allPieces))
                        minDtz50--;
                }
            } else if (res == TB_DRAW) {
                if (!drawMove ||
                    (TB_GET_PROMOTES(results[i]) ||
                     board::isOccupied(_decodeSquare[TB_GET_TO(results[i])], allPieces))) {
                    drawMove = results[i];
                }
            } else if (res == TB_BLESSED_LOSS && !bestMove && !drawMove && !bestMove50) {
                if ((int) dtz > maxDtzWorst50) {
                    worstMove50 = results[i];
                    maxDtzWorst50 = dtz;
                    if (TB_GET_PROMOTES(worstMove50) ||
                        board::isOccupied(_decodeSquare[TB_GET_TO(worstMove50)], allPieces))
                        maxDtzWorst50++;
                }
            } else if (res == TB_LOSS && !bestMove && !drawMove && !bestMove50 && !worstMove50) {
                if ((int) dtz > maxDtzWorst) {
                    worstMove = results[i];
                    maxDtzWorst = dtz;
                    if (TB_GET_PROMOTES(worstMove) || board::isOccupied(_decodeSquare[TB_GET_TO(worstMove)], allPieces))
                        maxDtzWorst++;
                }
            }
        }

        res->type = STANDARD_MOVE_MASK;

        if (bestMove) {
            debug("best ***********\n")
            res->from = _decodeSquare[TB_GET_FROM(bestMove)];
            res->to = _decodeSquare[TB_GET_TO(bestMove)];
            res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove));
            return true;
        }
        if (bestMove50) {
            debug("bestMove50 ***********\n")
            res->from = _decodeSquare[TB_GET_FROM(bestMove50)];
            res->to = _decodeSquare[TB_GET_TO(bestMove50)];
            res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove50));
            return true;
        }
        if (drawMove) {
            debug("drawMove ***********\n")
            res->from = _decodeSquare[TB_GET_FROM(drawMove)];
            res->to = _decodeSquare[TB_GET_TO(drawMove)];
            res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(drawMove));
            return true;
        }
        if (worstMove50) {
            debug("worstMove50 ***********\n")
            res->from = _decodeSquare[TB_GET_FROM(worstMove50)];
            res->to = _decodeSquare[TB_GET_TO(worstMove50)];
            res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove50));
            return true;
        }
        if (worstMove) {
            debug("worstMove ***********\n")
            res->from = _decodeSquare[TB_GET_FROM(worstMove)];
            res->to = _decodeSquare[TB_GET_TO(worstMove)];
            res->promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove));
            return true;
        }
        return false;
    }
#endif
    return false;
}

template<bool checkMoves>
bool Search::checkSearchMoves(const _Tmove *move) const {
    if (!checkMoves)return true;
    int m = move->to | (move->from << 8);
    if (std::find(searchMovesVector.begin(), searchMovesVector.end(), m) != searchMovesVector.end()) {
        return true;
    }
    return false;
}

#ifndef JS_MODE

int Search::probeWdl(const int depth, const uchar side, const int N_PIECE) {

//    if (N_PIECE == 3 &&  depth > 2 && depth != mainDepth && (board[WHITE] || board[BLACK])) {
//        const int kw = BITScanForward(board[KING_WHITE]);
//        const int kb = BITScanForward(board[KING_BLACK]);
//        const int pawnPos = BITScanForward(board[PAWN_BLACK]) | BITScanForward(board[PAWN_WHITE]);
//
//        const int winSide = board[PAWN_BLACK] ? BLACK : WHITE;
//
//        if (isDraw(winSide, side, kw, kb, pawnPos)) {
//            if (winSide == side) return -_INFINITE + depth;
//            else return _INFINITE - depth;
//        } else {
//            if (winSide == side) return _INFINITE - depth;
//            else return -_INFINITE + depth;
//        }
//    }
    if (N_PIECE < 8 && depth > 2 && depth != mainDepth) {
        int tbResult = INT_MAX;
        //syzygy
        const unsigned res = syzygy->SZtbProbeWDL(chessboard, side, N_PIECE);
        if (res != TB_RESULT_FAILED) {
            tbResult = TB_GET_WDL(res);
        }
        unsigned pliestomate;
        //gaviota
        if (GTB::getInstance().isInstalledPieces(N_PIECE)) {
            BENCH_START("gtbTime")
            tbResult = GTB::getInstance().getDtmWdl(GTB_STM(side), 0, chessboard, &pliestomate, false, rightCastle);
            BENCH_STOP("gtbTime")
        }
        switch (tbResult) {
            case TB_LOSS :
                return -_INFINITE + depth;
            case TB_BLESSED_LOSS :
                return -_INFINITE + depth + 50;
            case TB_WIN :
                return _INFINITE - depth;
            case TB_CURSED_WIN :
                return _INFINITE - depth - 50;
        }
    }
    return INT_MAX;
}

#endif

template<uchar side, bool checkMoves>
int Search::search(const int depth, int alpha, const int beta, _TpvLine *pline, const int N_PIECE,
                   const int nRootMoves) {

    ASSERT_RANGE(side, 0, 1)
    if (!getRunning()) return 0;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;
    if (depth >= MAX_PLY - 1) {
        return eval.getScore(chessboard, oldKey, side, alpha, beta);
    }
    INC(cumulativeMovesCount);
#ifndef JS_MODE
    int wdl = probeWdl(depth, side, N_PIECE);
    if (wdl != INT_MAX) return wdl;
#endif

    int score = -_INFINITE;
    const bool pvNode = alpha != beta - 1;

    assert(chessboard[KING_BLACK]);
    assert(chessboard[KING_WHITE]);

    const bool isIncheckSide = board::inCheck1<side>(chessboard);
    if (!isIncheckSide && depth != mainDepth) {
        if (board::checkInsufficientMaterial(N_PIECE, chessboard) || checkDraw(chessboard[ZOBRISTKEY_IDX])) {
            if (board::inCheck1<X(side)>(chessboard)) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -eval.lazyEval<side>(chessboard) * 2;
        }
    }
    int extension = isIncheckSide;
    if (depth + extension == 0) {
        return qsearch<side>(alpha, beta, NO_PROMOTION, 0);
    }

    /// ************* hash ****************
    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];
    u64 hashItem;
    const int hashValue = hash.readHash(alpha, beta, depth, zobristKeyR, hashItem, currentPly);
    if (hashValue != INT_MAX) {
        return hashValue;
    }

    /// ********** end hash ***************

    if (!(numMoves % 2048)) setRunning(checkTime());
    ++numMoves;
    _TpvLine line;
    line.cmove = 0;

    /// ********* null move ***********
    if (!nullSearch && !pvNode && !isIncheckSide) {
        int nDepth = (nRootMoves > 17 || depth > 3) ? 1 : 3;
        if (nDepth == 3) {
            const u64 pieces = board::getPiecesNoKing<side>(chessboard);
            if (pieces != chessboard[PAWN_BLACK + side] || bitCount(pieces) > 9)
                nDepth = 1;
        }
        if (depth > nDepth) {
            nullSearch = true;
            const int R = NULL_DEPTH + depth / NULL_DIVISOR;
            int nullScore;
            if (depth - R - 1 > 0) {
                nullScore = -search<X(side), checkMoves>(depth + extension - R - 1, -beta, -beta + 1, &line, N_PIECE,
                                                         nRootMoves);
                if (!forceCheck && abs(nullScore) > _INFINITE - MAX_PLY) {
                    currentPly++;
                    forceCheck = true;
                    nullScore = -search<X(side), checkMoves>(depth + extension - R - 1, -beta, -beta + 1, &line,
                                                             N_PIECE,
                                                             nRootMoves);
                    forceCheck = false;
                    currentPly--;
                }
            } else {
                nullScore = -qsearch<X(side)>(-beta, -beta + 1, -1, 0);
            }
            nullSearch = false;
            if (nullScore >= beta) {
                INC(nNullMoveCut);
                return nullScore;
            }
        }
    }

    /// ******* null move end ********

    /// ********************** Futility Pruning *********************
    /// ************* Futility Pruning razor at pre-pre-frontier ****
    bool futilPrune = false;
    int futilScore = 0;
    if (depth <= 3 && !isIncheckSide) {
        const int matBalance = eval.lazyEval<side>(chessboard);
        /// ******** reverse futility pruning ***********
        if (depth < 3 && !pvNode && abs(beta - 1) > -_INFINITE + MAX_PLY) {
            const int evalMargin = matBalance - eval.REVERSE_FUTIL_MARGIN * depth;
            if (evalMargin >= beta) return evalMargin;
        }
        /// *********************************************
        if ((futilScore = matBalance + eval.FUTIL_MARGIN) <= alpha) {
            if (depth == 3 && (matBalance + eval.RAZOR_MARGIN) <= alpha &&
                bitCount(board::getBitmapNoPawnsNoKing<X(side)>(chessboard)) > 3) {
                INC(nCutRazor);
                extension--;
            } else
                /// **************Futility Pruning at pre-frontier*****
            if (depth == 2 && (futilScore = matBalance + eval.EXT_FUTIL_MARGIN) <= alpha) {
                futilPrune = true;
                score = futilScore;
            } else
                /// **************Futility Pruning at frontier*****
            if (depth == 1) {
                futilPrune = true;
                score = futilScore;
            }
        }
    }
    /// ************ end Futility Pruning*************
    incListId();
    ASSERT_RANGE(KING_BLACK + side, 0, 11)
    ASSERT_RANGE(KING_BLACK + (X(side)), 0, 11)
    const u64 friends = board::getBitmap<side>(chessboard);
    const u64 enemies = board::getBitmap<X(side)>(chessboard);
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        return _INFINITE - (mainDepth - depth + 1);
    }
    generateMoves<side>(friends | enemies);
    const int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (isIncheckSide) {
            return -_INFINITE + (mainDepth - depth + 1);
        } else {
            return -eval.lazyEval<side>(chessboard) * 2;
        }
    }
    assert(genList[listId].size > 0);

    _Tmove *best = &genList[listId].moveList[0];

    INC(totGen);
    _Tmove *move;
    bool checkInCheck = false;
    int countMove = 0;
    char hashf = Hash::hashfALPHA;
    int first = 0;
    while ((move = getNextMove(&genList[listId], depth, hashItem, first++))) {
        if (!checkSearchMoves<checkMoves>(move) && depth == mainDepth) continue;
        countMove++;

        if (!makemove(move, true, checkInCheck)) {
            takeback(move, oldKey, oldEnpassant, true);
            continue;
        }
        checkInCheck = true;
        int val = INT_MAX;
        if (move->promotionPiece == NO_PROMOTION) {
            if (futilPrune && futilScore + PIECES_VALUE[move->capturedPiece] <= alpha &&
                !board::inCheck1<side>(chessboard)) {
                INC(nCutFp);
                takeback(move, oldKey, oldEnpassant, true);
                continue;
            }
            //Late Move Reduction
            if (countMove > 3 && !isIncheckSide && depth >= 3 && move->capturedPiece == SQUARE_EMPTY) {
                currentPly++;
                const int R = countMove > 6 ? 3 : 2;
                val = -search<X(side), checkMoves>(depth + extension - R, -(alpha + 1), -alpha, &line, N_PIECE,
                                                   nRootMoves);
                currentPly--;
                if (!forceCheck && abs(val) > _INFINITE - MAX_PLY) {
                    currentPly++;
                    forceCheck = true;
                    val = -search<X(side), checkMoves>(depth + extension - R, -(alpha + 1), -alpha, &line, N_PIECE,
                                                       nRootMoves);
                    forceCheck = false;
                    currentPly--;
                }
            }
        }
        if (val > alpha) {
            const int doMws = (score > -_INFINITE + MAX_PLY);
            const int lwb = max(alpha, score);
            const int upb = (doMws ? (lwb + 1) : beta);
            currentPly++;
            val = -search<X(side), checkMoves>(depth + extension - 1, -upb, -lwb, &line,
                                               move->capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                               nRootMoves);
            currentPly--;
            if (!forceCheck && abs(val) > _INFINITE - MAX_PLY) {
                currentPly++;
                forceCheck = true;
                val = -search<X(side), checkMoves>(depth + extension - 1, -upb, -lwb, &line,
                                                   move->capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                                   nRootMoves);
                forceCheck = false;
                currentPly--;
            }
            if (doMws && (lwb < val) && (val < beta)) {
                currentPly++;
                val = -search<X(side), checkMoves>(depth + extension - 1, -beta, -val + 1,
                                                   &line,
                                                   move->capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                                   nRootMoves);
                currentPly--;
                if (!forceCheck && abs(val) > _INFINITE - MAX_PLY) {
                    currentPly++;
                    forceCheck = true;
                    val = -search<X(side), checkMoves>(depth + extension - 1, -beta, -val + 1,
                                                       &line,
                                                       move->capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                                       nRootMoves);
                    forceCheck = false;
                    currentPly--;
                }
            }
        }
        score = max(score, val);
        takeback(move, oldKey, oldEnpassant, true);
        assert(chessboard[KING_BLACK]);
        assert(chessboard[KING_WHITE]);

        if (score > alpha) {
            if (move->capturedPiece == SQUARE_EMPTY && move->promotionPiece == NO_PROMOTION) {
                setKiller(move->from, move->to, depth);
            }
            if (score >= beta) {
                decListId();
                INC(nCutAB);
                INC(betaEfficiencyCount);
                DEBUG(betaEfficiency +=
                              (100.0 - ((double) countMove * 100.0 / (double) listcount)) +
                              (((double) countMove * 100.0 / (double) listcount) / (double) countMove))
                if (getRunning()) {
                    Hash::_Thash data(zobristKeyR, score, depth, move->from, move->to, Hash::hashfBETA);
                    hash.recordHash(data, ply);
                }

                if (move->capturedPiece == SQUARE_EMPTY && move->promotionPiece == NO_PROMOTION) {
                    setHistoryHeuristic(move->from, move->to, depth);
                }
                return score;
            }
            alpha = score;
            hashf = Hash::hashfEXACT;
            best = move;
            updatePv(pline, &line, move);
        }
    }
    if (getRunning()) {
        if (best->capturedPiece == SQUARE_EMPTY && best->promotionPiece == NO_PROMOTION) {
            setHistoryHeuristic(best->from, best->to, depth - extension);
            setKiller(best->from, best->to, depth - extension);
        }
        Hash::_Thash data(zobristKeyR, score, depth, best->from, best->to, hashf);
        hash.recordHash(data, ply);
    }
    decListId();
    return score;

}

void Search::updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move) {
    assert(line->cmove < MAX_PLY - 1);
    memcpy(&(pline->argmove[0]), move, sizeof(_Tmove));
    memcpy(pline->argmove + 1, line->argmove, line->cmove * sizeof(_Tmove));
    assert(line->cmove >= 0);
    pline->cmove = line->cmove + 1;
}

u64 Search::getZobristKey() const {
    return chessboard[ZOBRISTKEY_IDX];
}

void Search::unsetSearchMoves() {
    searchMovesVector.clear();
}

void Search::setSearchMoves(const vector<int> &s) {
    searchMovesVector = s;
}

#ifdef TUNING

int Search::getParameter(const string &p) {
    if (p == "ATTACK_KING")return eval.ATTACK_KING;
    if (p == "BISHOP_ON_QUEEN")return eval.BISHOP_ON_QUEEN;
    if (p == "BACKWARD_PAWN")return eval.BACKWARD_PAWN;
    if (p == "DOUBLED_ISOLATED_PAWNS")return eval.DOUBLED_ISOLATED_PAWNS;
//    if (p == "DOUBLED_PAWNS")return eval.DOUBLED_PAWNS;
    if (p == "PAWN_IN_7TH")return eval.PAWN_IN_7TH;
//    if (p == "PAWN_CENTER")return eval.PAWN_CENTER;
    if (p == "PAWN_IN_PROMOTION")return eval.PAWN_IN_PROMOTION;
//    if (p == "PAWN_ISOLATED")return eval.PAWN_ISOLATED;
    if (p == "PAWN_NEAR_KING")return eval.PAWN_NEAR_KING;
    if (p == "PAWN_BLOCKED")return eval.PAWN_BLOCKED;
    if (p == "UNPROTECTED_PAWNS")return eval.UNPROTECTED_PAWNS;
//    if (p == "ENEMY_NEAR_KING")return eval.ENEMY_NEAR_KING;
    if (p == "FRIEND_NEAR_KING")return eval.FRIEND_NEAR_KING;
//    if (p == "HALF_OPEN_FILE_Q")return eval.HALF_OPEN_FILE_Q;
    if (p == "BONUS2BISHOP")return eval.BONUS2BISHOP;
    if (p == "BISHOP_PAWN_ON_SAME_COLOR")return eval.BISHOP_PAWN_ON_SAME_COLOR;
//    if (p == "CONNECTED_ROOKS")return eval.CONNECTED_ROOKS;
//    if (p == "OPEN_FILE")return eval.OPEN_FILE;
    if (p == "OPEN_FILE_Q")return eval.OPEN_FILE_Q;
    if (p == "ROOK_7TH_RANK")return eval.ROOK_7TH_RANK;
//    if (p == "ROOK_BLOCKED")return eval.ROOK_BLOCKED;
//    if (p == "ROOK_TRAPPED")return eval.ROOK_TRAPPED;
//    if (p == "UNDEVELOPED_KNIGHT")return eval.UNDEVELOPED_KNIGHT;
//    if (p == "UNDEVELOPED_BISHOP")return eval.UNDEVELOPED_BISHOP;
    if (p == "KNIGHT_PINNED")return eval.KNIGHT_PINNED;
    if (p == "ROOK_PINNED")return eval.ROOK_PINNED;
    if (p == "BISHOP_PINNED")return eval.BISHOP_PINNED;
    if (p == "QUEEN_PINNED")return eval.QUEEN_PINNED;
    if (p == "QUEEN_IN_7")return eval.QUEEN_IN_7;
    if (p == "ROOK_IN_7")return eval.ROOK_IN_7;
//    if (p == "PAWN_PINNED")return eval.PAWN_PINNED;
    fatal("Not found ", p)
    exit(1);
}

void Search::setParameter(const string &p, const int value) {
    //cout << "setParameter " << param << " " << value << endl;
    if (p == "ATTACK_KING")eval.ATTACK_KING = value;
    else if (p == "BISHOP_ON_QUEEN")eval.BISHOP_ON_QUEEN = value;
    else if (p == "BACKWARD_PAWN")eval.BACKWARD_PAWN = value;
    else if (p == "DOUBLED_ISOLATED_PAWNS")eval.DOUBLED_ISOLATED_PAWNS = value;
//    else if (p == "DOUBLED_PAWNS")eval.DOUBLED_PAWNS = value;
    else if (p == "PAWN_IN_7TH")eval.PAWN_IN_7TH = value;
//    else if (p == "PAWN_CENTER")eval.PAWN_CENTER = value;
    else if (p == "PAWN_IN_PROMOTION")eval.PAWN_IN_PROMOTION = value;
//    else if (p == "PAWN_ISOLATED")eval.PAWN_ISOLATED = value;
    else if (p == "PAWN_NEAR_KING")eval.PAWN_NEAR_KING = value;
    else if (p == "PAWN_BLOCKED")eval.PAWN_BLOCKED = value;
    else if (p == "UNPROTECTED_PAWNS")eval.UNPROTECTED_PAWNS = value;
//    else if (p == "ENEMY_NEAR_KING")eval.ENEMY_NEAR_KING = value;
    else if (p == "FRIEND_NEAR_KING")eval.FRIEND_NEAR_KING = value;
//    else if (p == "HALF_OPEN_FILE_Q")eval.HALF_OPEN_FILE_Q = value;
    else if (p == "BONUS2BISHOP")eval.BONUS2BISHOP = value;
    else if (p == "BISHOP_PAWN_ON_SAME_COLOR")eval.BISHOP_PAWN_ON_SAME_COLOR = value;
//    else if (p == "CONNECTED_ROOKS")eval.CONNECTED_ROOKS = value;
//    else if (p == "OPEN_FILE")eval.OPEN_FILE = value;
    else if (p == "OPEN_FILE_Q")eval.OPEN_FILE_Q = value;
    else if (p == "ROOK_7TH_RANK")eval.ROOK_7TH_RANK = value;
//    else if (p == "ROOK_BLOCKED")eval.ROOK_BLOCKED = value;
//    else if (p == "ROOK_TRAPPED")eval.ROOK_TRAPPED = value;
//    else if (p == "UNDEVELOPED_KNIGHT")eval.UNDEVELOPED_KNIGHT = value;
//    else if (p == "UNDEVELOPED_BISHOP")eval.UNDEVELOPED_BISHOP = value;
    else if (p == "KNIGHT_PINNED")eval.KNIGHT_PINNED = value;
    else if (p == "ROOK_PINNED")eval.ROOK_PINNED = value;
    else if (p == "BISHOP_PINNED")eval.BISHOP_PINNED = value;
    else if (p == "QUEEN_PINNED")eval.QUEEN_PINNED = value;
    else if (p == "QUEEN_IN_7")eval.QUEEN_IN_7 = value;
    else if (p == "ROOK_IN_7")eval.ROOK_IN_7 = value;
//    else if (p == "PAWN_PINNED")eval.PAWN_PINNED = value;
    else {
        fatal("Not found ", p)
        exit(1);
    }
}

#endif

template<uchar side>
bool Search::badCapure(const _Tmove &move, const u64 allpieces) {

    if (move.pieceFrom == (PAWN_BLACK + side)) return false;

    if (PIECES_VALUE[move.capturedPiece] - 5 >= PIECES_VALUE[move.pieceFrom]) return false;

    if (PIECES_VALUE[move.capturedPiece] + 200 < PIECES_VALUE[move.pieceFrom] &&
        (PAWN_FORK_MASK[side][move.to] & chessboard[PAWN_BLACK + (X(side))]))
        return true;

    if (PIECES_VALUE[move.capturedPiece] + 500 < PIECES_VALUE[move.pieceFrom] &&
        board::isAttacked(side, move.to, allpieces, chessboard))
        return true;

    return false;
}


