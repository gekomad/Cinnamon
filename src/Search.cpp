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
#include "SearchManager.h"
#include "db/bitbase/kpk.h"

bool volatile Search::runningThread;
high_resolution_clock::time_point Search::startTime;
using namespace _bitbase;

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
        valWindow = search<searchMoves>(depth, -_INFINITE - 1, _INFINITE + 1);
    } else {
        int tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);

        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW);
            } else {
                tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2);
            }

            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW);
                } else {
                    tmp = search<searchMoves>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4);
                }

                if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                    tmp = search<searchMoves>(mainDepth, -_INFINITE - 1, _INFINITE + 1);
                }
            }
        }
        if (getRunning()) {
            valWindow = tmp;
        }
    }
}

Search::Search() : ponder(false), nullSearch(false) {
#ifdef DEBUG_MODE
    lazyEvalCuts = cumulativeMovesCount = totGen = 0;
#endif

}

void Search::clone(const Search *s) {
    memcpy(chessboard, s->chessboard, sizeof(_Tchessboard));
}

#ifndef JS_MODE

int Search::SZtbProbeWDL() const {
    const auto tot = bitCount(board::getBitmap<WHITE>(chessboard) | board::getBitmap<BLACK>(chessboard));
    const int side = board::getSide(chessboard);
    return syzygy->SZtbProbeWDL(chessboard, side, tot);
}

void Search::printWdlSyzygy() {
    perftMode = true;
    const int side = board::getSide(chessboard);
    u64 friends = side == WHITE ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    u64 enemies = side == BLACK ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);

    incListId();
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    _Tmove *move;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    display();

    unsigned res = syzygy->SZtbProbeWDL(chessboard, side);
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
        move = &gen_list[listId].moveList[i];
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        }
        print(move, chessboard);

        unsigned res = syzygy->SZtbProbeWDL(chessboard, side ^ 1);

        if (res != TB_RESULT_FAILED) {
            res = TB_GET_WDL(res);
            if (res == TB_DRAW)cout << " draw" << endl;
            else if (res == TB_LOSS || res == TB_BLESSED_LOSS)
                cout << " win" << endl;
            else
                cout << " loss" << endl;

        } else
            cout << " none" << endl;
        takeback(move, oldKey, false);
    }
    cout << endl;
    decListId();
}

void Search::printDtzSyzygy() {
    perftMode = true;
    const int side = board::getSide(chessboard);

    if ((side == BLACK) ? board::inCheck1<WHITE>(chessboard) : board::inCheck1<BLACK>(chessboard)) {
        cout << "invalid position" << endl;
        return;
    }

    unsigned results[TB_MAX_MOVES];

    const u64 white = board::getBitmap<WHITE>(chessboard);
    const u64 black = board::getBitmap<BLACK>(chessboard);

    unsigned res = syzygy->SZtbProbeWDL(chessboard, side);
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

    const unsigned res1 = syzygy->SZtbProbeRoot(white, black, chessboard, side, results);
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
    int side = board::getSide(chessboard);
    u64 friends = side == WHITE ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    u64 enemies = side == BLACK ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    display();
    cout << "current: ";
    unsigned pliestomate;
    const int res = GTB::getInstance().getDtmWdl(GTB_STM(side), 2, chessboard, &pliestomate, dtm);
    incListId();
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    _Tmove *move;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];

    for (int i = 0; i < getListSize(); i++) {
        move = &gen_list[listId].moveList[i];
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        }
        print(move, chessboard);

        GTB::getInstance().getDtmWdl(GTB_STM(side ^ 1), 1, chessboard, &pliestomate, dtm);

        takeback(move, oldKey, false);

    }
    cout << endl;
    decListId();
    return res;
}

#endif

void Search::setNullMove(bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Search::setMainPly(const int m) {
    mainDepth = m;
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

template<int side>
int Search::quiescence(int alpha, const int beta, const char promotionPiece, const int depth) {

    if (!getRunning()) {
        return 0;
    }

    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];
    int score = getScore(zobristKeyR, side, alpha, beta, false);
    if (score >= beta) {
        return beta;
    }
    ///************* hash ****************
    char hashf = Hash::hashfALPHA;

    pair<int, _TcheckHash> hashAlwaysItem = checkHash(Hash::HASH_ALWAYS, true, alpha, beta, depth, zobristKeyR);
    if (hashAlwaysItem.first != INT_MAX) {
        return hashAlwaysItem.first;
    };

    pair<int, _TcheckHash> hashGreaterItem = checkHash(Hash::HASH_GREATER, true, alpha, beta, depth, zobristKeyR);
    if (hashGreaterItem.first != INT_MAX) {
        return hashGreaterItem.first;
    };

///********** end hash ***************
/**************Delta Pruning ****************/
    char fprune = 0;
    int fscore;
    if ((fscore = score + (promotionPiece == NO_PROMOTION ? VALUEQUEEN : 2 * VALUEQUEEN)) < alpha) {
        fprune = 1;
    }
/************ end Delta Pruning *************/
    if (score > alpha) {
        alpha = score;
    }

    incListId();

    u64 friends = board::getBitmap<side>(chessboard);
    u64 enemies = board::getBitmap<side ^ 1>(chessboard);
    if (generateCaptures<side>(enemies, friends)) {
        decListId();

        return _INFINITE - (mainDepth + depth);
    }
    if (!getListSize()) {
        --listId;
        return score;
    }
    _Tmove *move;
    _Tmove *best = &gen_list[listId].moveList[0];
    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    Hash::_ThashData *c = nullptr;
    if (hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3) {
        c = &hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS];
    } else if (hashGreaterItem.second.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3) {
        c = &hashGreaterItem.second.phasheType[Hash::HASH_GREATER];
    }
    int first = 0;
    while ((move = getNextMove(&gen_list[listId], depth, c, first++))) {
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        }
/**************Delta Pruning ****************/
        if (fprune && ((move->s.type & 0x3) != PROMOTION_MOVE_MASK) &&
            fscore + PIECES_VALUE[move->s.capturedPiece] <= alpha) {
            INC(nCutFp);
            takeback(move, oldKey, false);
            continue;
        }
/************ end Delta Pruning *************/
        int val = -quiescence<side ^ 1>(-beta, -alpha, move->s.promotionPiece, depth - 1);
        score = max(score, val);
        takeback(move, oldKey, false);
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                if (getRunning()) {
                    Hash::_ThashData data(score, depth, move->s.from, move->s.to, 0, Hash::hashfBETA);
                    hash.recordHash(zobristKeyR, data);
                }
                return beta;
            }
            best = move;
            alpha = score;
            hashf = Hash::hashfEXACT;
        }
    }
    if (getRunning()) {
        Hash::_ThashData data(score, depth, best->s.from, best->s.to, 0, hashf);
        hash.recordHash(zobristKeyR, data);
    }
    decListId();

    return score;
}

void Search::setPonder(bool r) {
    ponder = r;
}

void Search::setRunning(int r) {
    GenMoves::setRunning(r);
    if (!r) {
        maxTimeMillsec = 0;
    }
}

int Search::getRunning() const {
    if (!runningThread)return 0;
    return GenMoves::getRunning();

}

void Search::setMaxTimeMillsec(int n) {
    maxTimeMillsec = n;
}

int Search::getMaxTimeMillsec() const {
    return maxTimeMillsec;
}

bool Search::checkDraw(u64 key) {
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
int Search::search(const int depth, const int alpha, const int beta) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    auto ep = chessboard[ENPASSANT_IDX];
    incListId();
    const int side = board::getSide(chessboard);
    const u64 friends = (side == WHITE) ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(chessboard);
    const u64 enemies = (side == WHITE) ? board::getBitmap<BLACK>(chessboard) : board::getBitmap<WHITE>(chessboard);
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    int n_root_moves = getListSize();
    decListId();
    chessboard[ENPASSANT_IDX] = ep;
    return board::getSide(chessboard) ? search<WHITE, searchMoves>(depth,
                                                                   alpha,
                                                                   beta,
                                                                   &pvLine,
                                                                   bitCount(board::getBitmap<WHITE>(chessboard) |
                                                                            board::getBitmap<BLACK>(chessboard)),
                                                                   &mainMateIn,
                                                                   n_root_moves)
                                      : search<BLACK, searchMoves>(depth, alpha, beta, &pvLine,
                                                                   bitCount(board::getBitmap<WHITE>(chessboard) |
                                                                            board::getBitmap<BLACK>(chessboard)),
                                                                   &mainMateIn,
                                                                   n_root_moves);
}

bool Search::probeRootTB(_Tmove *res) {
    const u64 white = board::getBitmap<WHITE>(chessboard);
    const u64 black = board::getBitmap<BLACK>(chessboard);
    const auto tot = bitCount(white | black);
    const int side = board::getSide(chessboard);

    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];

    if (tot == 3 && (chessboard[WHITE] || chessboard[BLACK])) {
        _Tmove *bestMove = nullptr;
        u64 friends = side == WHITE ? white : black;
        u64 enemies = side == BLACK ? white : black;

        incListId();
        generateCaptures(side, enemies, friends);
        generateMoves(side, friends | enemies);

        for (int i = 0; i < getListSize(); i++) {
            if (bestMove)break;
            _Tmove *move = &gen_list[listId].moveList[i];
            if (!makemove(move, false, true)) {
                takeback(move, oldKey, false);
                continue;
            }

            const int kw = BITScanForward(chessboard[KING_WHITE]);
            const int kb = BITScanForward(chessboard[KING_BLACK]);
            const int pawnQueenPos = BITScanForward(chessboard[PAWN_BLACK]) | BITScanForward(chessboard[PAWN_WHITE]) |
                                     BITScanForward(chessboard[QUEEN_BLACK]) | BITScanForward(chessboard[QUEEN_WHITE]);
            const int winSide = chessboard[PAWN_BLACK] | chessboard[QUEEN_BLACK] ? BLACK : WHITE;

            bool p;

            if (winSide != side) { // looking for draw
                p = isDraw(winSide, side ^ 1, kw, kb, pawnQueenPos);
            } else { //looking for win
                p = !isDraw(winSide, side ^ 1, kw, kb, pawnQueenPos);
            }
            if (p &&
                (bestMove == nullptr || move->s.capturedPiece != SQUARE_EMPTY ||
                 move->s.promotionPiece != NO_PROMOTION)) {
                bestMove = move;
            }
            takeback(move, oldKey, false);
        }

        decListId();
        if (bestMove) {
            memcpy(res, bestMove, sizeof(_Tmove));
            if (res->s.pieceFrom == PAWN_WHITE && res->s.to > 55)res->s.promotionPiece = 'q';
            else if (res->s.pieceFrom == PAWN_BLACK && res->s.to < 8)res->s.promotionPiece = 'q';
            return true;
        }
    }
#ifndef JS_MODE
    //gaviota
    if (GTB::getInstance().isInstalledPieces(tot)) {
        u64 friends = side == WHITE ? white : black;
        u64 enemies = side == BLACK ? white : black;
        _Tmove *bestMove = nullptr;
        incListId();
        generateCaptures(side, enemies, friends);
        generateMoves(side, friends | enemies);

        _Tmove *drawMove = nullptr;
        _Tmove *worstMove = nullptr;

        int minDtz = -1;
        unsigned maxDtzWorst = 1000;

        const u64 allPieces = white | black;

        unsigned dtz;

        for (int i = 0; i < getListSize(); i++) {
            _Tmove *move = &gen_list[listId].moveList[i];
            if (!makemove(move, false, true)) {
                takeback(move, oldKey, false);
                continue;
            }
            BENCH(times->start("gtbTime"))
            const auto res = GTB::getInstance().getDtmWdl(GTB_STM(side ^ 1), 0, chessboard, &dtz, true);
            BENCH(times->stop("gtbTime"))
            if (res == TB_WIN && !worstMove && !drawMove) {
                if ((int) dtz > minDtz) {
                    bestMove = move;
                    minDtz = dtz;
                    if (move->s.promotionPiece != NO_PROMOTION || board::isOccupied(move->s.to, allPieces))
                        minDtz++;
                }
            } else if (res == TB_DRAW) {
                if (!drawMove || (move->s.promotionPiece != NO_PROMOTION || board::isOccupied(move->s.to, allPieces))) {
                    drawMove = move;
                }
            } else if (res == TB_LOSS) {
                if ((int) dtz < (int) maxDtzWorst) {
                    worstMove = move;
                    maxDtzWorst = dtz;
                    if (move->s.promotionPiece != NO_PROMOTION || board::isOccupied(move->s.to, allPieces))
                        maxDtzWorst--;
                }
            }
            takeback(move, oldKey, false);
        }

        if (worstMove) {
            debug("worstMove ***********\n");
            memcpy(res, worstMove, sizeof(_Tmove));
            if (res->s.promotionPiece != NO_PROMOTION)
                res->s.promotionPiece = FEN_PIECE[res->s.promotionPiece];
            decListId();
            return true;
        }

        if (drawMove) {
            debug("drawMove ***********\n");
            memcpy(res, drawMove, sizeof(_Tmove));
            if (res->s.promotionPiece != NO_PROMOTION)res->s.promotionPiece = FEN_PIECE[res->s.promotionPiece];
            decListId();
            return true;
        }
        if (bestMove) {
            debug("best ***********\n");
            memcpy(res, bestMove, sizeof(_Tmove));
            if (res->s.promotionPiece != NO_PROMOTION)
                res->s.promotionPiece = FEN_PIECE[res->s.promotionPiece];
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
        BENCH(times->start("syzygyTime"))
        const auto sz = syzygy->SZtbProbeRoot(white, black, chessboard, side, results);
        BENCH(times->stop("syzygyTime"))
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

        res->s.type = STANDARD_MOVE_MASK;

        if (bestMove) {
            debug("best ***********\n");
            res->s.from = _decodeSquare[TB_GET_FROM(bestMove)];
            res->s.to = _decodeSquare[TB_GET_TO(bestMove)];
            res->s.promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove));
            return true;
        }
        if (bestMove50) {
            debug("bestMove50 ***********\n");
            res->s.from = _decodeSquare[TB_GET_FROM(bestMove50)];
            res->s.to = _decodeSquare[TB_GET_TO(bestMove50)];
            res->s.promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(bestMove50));
            return true;
        }
        if (drawMove) {
            debug("drawMove ***********\n");
            res->s.from = _decodeSquare[TB_GET_FROM(drawMove)];
            res->s.to = _decodeSquare[TB_GET_TO(drawMove)];
            res->s.promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(drawMove));
            return true;
        }
        if (worstMove50) {
            debug("worstMove50 ***********\n");
            res->s.from = _decodeSquare[TB_GET_FROM(worstMove50)];
            res->s.to = _decodeSquare[TB_GET_TO(worstMove50)];
            res->s.promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove50));
            return true;
        }
        if (worstMove) {
            debug("worstMove ***********\n");
            res->s.from = _decodeSquare[TB_GET_FROM(worstMove)];
            res->s.to = _decodeSquare[TB_GET_TO(worstMove)];
            res->s.promotionPiece = SYZYGY::getPromotion(TB_GET_PROMOTES(worstMove));
            return true;
        }
        return false;
    }
#endif
    return false;
}

template<bool checkMoves>
bool Search::checkSearchMoves(_Tmove *move) const {
    if (!checkMoves)return true;
    int m = move->s.to | (move->s.from << 8);
    if (std::find(searchMovesVector.begin(), searchMovesVector.end(), m) != searchMovesVector.end()) {
        return true;
    }
    return false;
}

#ifndef JS_MODE

int Search::probeWdl(const int depth, const int side, const int N_PIECE) {

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
            BENCH(times->start("gtbTime"))
            tbResult = GTB::getInstance().getDtmWdl(GTB_STM(side), 0, chessboard, &pliestomate, false);
            BENCH(times->stop("gtbTime"))
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

template<int side, bool checkMoves>
int Search::search(int depth, int alpha, const int beta, _TpvLine *pline, const int N_PIECE, int *mateIn,
                   const int n_root_moves) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    ASSERT_RANGE(side, 0, 1);
    if (!getRunning()) {
        return 0;
    }
    INC(cumulativeMovesCount);
#ifndef JS_MODE
    int wdl = probeWdl(depth, side, N_PIECE);
    if (wdl != INT_MAX) return wdl;
#endif

    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    *mateIn = INT_MAX;
    int score = -_INFINITE;
    const int pvNode = alpha != beta - 1;

#ifdef DEBUG_MODE
    double betaEfficiencyCount = 0.0;
#endif
    ASSERT(chessboard[KING_BLACK]);
    ASSERT(chessboard[KING_WHITE]);

    int extension = 0;
    const int is_incheck_side = board::inCheck1<side>(chessboard);
    if (!is_incheck_side && depth != mainDepth) {
        if (board::checkInsufficientMaterial(N_PIECE, chessboard) || checkDraw(chessboard[ZOBRISTKEY_IDX])) {
            if (board::inCheck1<side ^ 1>(chessboard)) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -lazyEval<side>() * 2;
        }
    }
    if (is_incheck_side) {
        extension++;
    }
    depth += extension;
    if (depth == 0) {
        return quiescence<side>(alpha, beta, -1, 0);
    }

    //************* hash ****************
    u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];

    pair<int, _TcheckHash> hashGreaterItem = checkHash(Hash::HASH_GREATER, false, alpha, beta, depth, zobristKeyR);
    if (hashGreaterItem.first != INT_MAX) {
        return hashGreaterItem.first;
    };

    pair<int, _TcheckHash> hashAlwaysItem = checkHash(Hash::HASH_ALWAYS, false, alpha, beta, depth, zobristKeyR);
    if (hashAlwaysItem.first != INT_MAX) {
        return hashAlwaysItem.first;
    };
    ///********** end hash ***************

    if (!(numMoves % 8192)) {
        setRunning(checkTime());
    }
    ++numMoves;
    _TpvLine line;
    line.cmove = 0;

    // ********* null move ***********
    if (!nullSearch && !pvNode && !is_incheck_side) {
        int n_depth = (n_root_moves > 17 || depth > 3) ? 1 : 3;
        if (n_depth == 3) {
            const u64 pieces = board::getPiecesNoKing<side>(chessboard);
            if (pieces != chessboard[PAWN_BLACK + side] || bitCount(pieces) > 9)
                n_depth = 1;
        }
        if (depth > n_depth) {
            nullSearch = true;
            const int R = NULL_DEPTH + depth / NULL_DIVISOR;
            const int nullScore =
                    (depth - R - 1 > 0) ?
                    -search<side ^ 1, checkMoves>(depth - R - 1, -beta, -beta + 1, &line, N_PIECE, mateIn,
                                                  n_root_moves)
                                        :
                    -quiescence<side ^ 1>(-beta, -beta + 1, -1, 0);
            nullSearch = false;
            if (nullScore >= beta) {
                INC(nNullMoveCut);
                return nullScore;
            }
        }
    }

    ///******* null move end ********

    /**************Futility Pruning****************/
    /**************Futility Pruning razor at pre-pre-frontier*****/
    bool futilPrune = false;
    int futilScore = 0;
    if (depth <= 3 && !is_incheck_side) {
        int matBalance = lazyEval<side>();
        if ((futilScore = matBalance + FUTIL_MARGIN) <= alpha) {
            if (depth == 3 && (matBalance + RAZOR_MARGIN) <= alpha &&
                board::getNpiecesNoPawnNoKing<side ^ 1>(chessboard) > 3) {
                INC(nCutRazor);
                depth--;
            } else
                ///**************Futility Pruning at pre-frontier*****
            if (depth == 2 && (futilScore = matBalance + EXT_FUTILY_MARGIN) <= alpha) {
                futilPrune = true;
                score = futilScore;
            } else
                ///**************Futility Pruning at frontier*****
            if (depth == 1) {
                futilPrune = true;
                score = futilScore;
            }
        }
    }
    /************ end Futility Pruning*************/
    incListId();
    ASSERT_RANGE(KING_BLACK + side, 0, 11);
    ASSERT_RANGE(KING_BLACK + (side ^ 1), 0, 11);
    const u64 friends = board::getBitmap<side>(chessboard);
    const u64 enemies = board::getBitmap<side ^ 1>(chessboard);
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        score = _INFINITE - (mainDepth - depth + 1);
        return score;
    }
    generateMoves<side>(friends | enemies);
    const int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (is_incheck_side) {
            return -_INFINITE + (mainDepth - depth + 1);
        } else {
            return -lazyEval<side>() * 2;
        }
    }
    ASSERT(gen_list[listId].size > 0);
    Hash::_ThashData *c = nullptr;
    _Tmove *best = &gen_list[listId].moveList[0];
    if (hashGreaterItem.second.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3) {
        c = &hashGreaterItem.second.phasheType[Hash::HASH_GREATER];
    } else if (hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3) {
        c = &hashAlwaysItem.second.phasheType[Hash::HASH_ALWAYS];
    }
    INC(totGen);
    _Tmove *move;
    bool checkInCheck = false;
    int countMove = 0;
    char hashf = Hash::hashfALPHA;
    int first = 0;
    while ((move = getNextMove(&gen_list[listId], depth, c, first++))) {
        if (!checkSearchMoves<checkMoves>(move) && depth == mainDepth) continue;
        countMove++;
        INC(betaEfficiencyCount);
        if (!makemove(move, true, checkInCheck)) {
            takeback(move, oldKey, true);
            continue;
        }
        checkInCheck = true;
        if (futilPrune && ((move->s.type & 0x3) != PROMOTION_MOVE_MASK) &&
            futilScore + PIECES_VALUE[move->s.capturedPiece] <= alpha && !board::inCheck1<side>(chessboard)) {
            INC(nCutFp);
            takeback(move, oldKey, true);
            continue;
        }
        //Late Move Reduction
        int val = INT_MAX;
        if (countMove > 4 && !is_incheck_side && depth >= 3 && move->s.capturedPiece == SQUARE_EMPTY &&
            move->s.promotionPiece == NO_PROMOTION) {
            currentPly++;
            val = -search<side ^ 1, checkMoves>(depth - 2, -(alpha + 1), -alpha, &line, N_PIECE, mateIn,
                                                n_root_moves);
            ASSERT(val != INT_MAX);
            currentPly--;
        }
        if (val > alpha) {
            const int doMws = (score > -_INFINITE + MAX_PLY);
            const int lwb = max(alpha, score);
            const int upb = (doMws ? (lwb + 1) : beta);
            currentPly++;
            val = -search<side ^ 1, checkMoves>(depth - 1, -upb, -lwb, &line,
                                                move->s.capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                                mateIn, n_root_moves);
            currentPly--;
            if (doMws && (lwb < val) && (val < beta)) {
                currentPly++;
                val = -search<side ^ 1, checkMoves>(depth - 1, -beta, -val + 1,
                                                    &line,
                                                    move->s.capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1,
                                                    mateIn, n_root_moves);
                if (move->s.capturedPiece == SQUARE_EMPTY && move->s.promotionPiece == NO_PROMOTION &&
                    val < -(_INFINITE - 100)) {
                    setKiller(move->s.from, move->s.to, depth, true);
                }
                currentPly--;
            } else {
                if (move->s.capturedPiece == SQUARE_EMPTY && move->s.promotionPiece == NO_PROMOTION &&
                    val < -(_INFINITE - 100)) {
                    setKiller(move->s.from, move->s.to, depth, true);
                }
            }
        }
        score = max(score, val);
        takeback(move, oldKey, true);
        ASSERT(chessboard[KING_BLACK])
        ASSERT(chessboard[KING_WHITE])

        if (score > alpha) {
            if (score >= beta) {
                decListId();
                INC(nCutAB);
                ADD(betaEfficiency, betaEfficiencyCount / (double) listcount * 100.0);
                if (getRunning()) {
                    Hash::_ThashData data(score, depth - extension, move->s.from, move->s.to, 0, Hash::hashfBETA);
                    hash.recordHash(zobristKeyR, data);
                }
                if (depth < 31)
                    setHistoryHeuristic(move->s.from, move->s.to, 1 << depth);
                else
                    setHistoryHeuristic(move->s.from, move->s.to, 0x40000000);
                if (move->s.capturedPiece == SQUARE_EMPTY && move->s.promotionPiece == NO_PROMOTION)
                    setKiller(move->s.from, move->s.to, depth, false);
                return score;
            }
            alpha = score;
            hashf = Hash::hashfEXACT;
            best = move;
            updatePv(pline, &line, move);
        }
    }
    if (getRunning()) {
        Hash::_ThashData data(score, depth - extension, best->s.from, best->s.to, 0, hashf);
        hash.recordHash(zobristKeyR, data);
    }
    decListId();
    return score;

}

void Search::updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move) {
    ASSERT(line->cmove < MAX_PLY - 1);
    memcpy(&(pline->argmove[0]), move, sizeof(_Tmove));
    memcpy(pline->argmove + 1, line->argmove, line->cmove * sizeof(_Tmove));
    ASSERT(line->cmove >= 0);
    pline->cmove = line->cmove + 1;
}

u64 Search::getZobristKey() {
    return chessboard[ZOBRISTKEY_IDX];
}

void Search::unsetSearchMoves() {
    searchMovesVector.clear();
}

void Search::setSearchMoves(vector<int> &s) {
    searchMovesVector = s;
}

bool Search::setParameter(String param, int value) {
#if defined(CLOP) || defined(DEBUG_MODE)
    param.toUpper();
    bool res = true;
    if (param == "FUTIL_MARGIN") {
        FUTIL_MARGIN = value;
    } else if (param == "EXT_FUTILY_MARGIN") {
        EXT_FUTILY_MARGIN = value;
    } else if (param == "RAZOR_MARGIN") {
        RAZOR_MARGIN = value;
    } else if (param == "ATTACK_KING") {
        ATTACK_KING = value;
    } else if (param == "BACKWARD_PAWN") {
        BACKWARD_PAWN = value;
    } else if (param == "BISHOP_ON_QUEEN") {
        BISHOP_ON_QUEEN = value;
    } else if (param == "BONUS2BISHOP") {
        BONUS2BISHOP = value;
    } else if (param == "CONNECTED_ROOKS") {
        CONNECTED_ROOKS = value;
    } else if (param == "DOUBLED_ISOLATED_PAWNS") {
        DOUBLED_ISOLATED_PAWNS = value;
    } else if (param == "DOUBLED_PAWNS") {
        DOUBLED_PAWNS = value;
    } else if (param == "END_OPENING") {
        END_OPENING = value;
    } else if (param == "ENEMY_NEAR_KING") {
        ENEMY_NEAR_KING = value;
    } else if (param == "FRIEND_NEAR_KING") {
        FRIEND_NEAR_KING = value;
    } else if (param == "HALF_OPEN_FILE_Q") {
        HALF_OPEN_FILE_Q = value;
    } else if (param == "OPEN_FILE") {
        OPEN_FILE = value;
    } else if (param == "OPEN_FILE_Q") {
        OPEN_FILE_Q = value;
    } else if (param == "PAWN_IN_7TH") {
        PAWN_IN_7TH = value;
    } else if (param == "PAWN_CENTER") {
        PAWN_CENTER = value;
    } else if (param == "PAWN_IN_8TH") {
        PAWN_IN_8TH = value;
    } else if (param == "PAWN_ISOLATED") {
        PAWN_ISOLATED = value;
    } else if (param == "PAWN_NEAR_KING") {
        PAWN_NEAR_KING = value;
    } else if (param == "PAWN_BLOCKED") {
        PAWN_BLOCKED = value;
    } else if (param == "ROOK_7TH_RANK") {
        ROOK_7TH_RANK = value;
    } else if (param == "ROOK_BLOCKED") {
        ROOK_BLOCKED = value;
    } else if (param == "ROOK_TRAPPED") {
        ROOK_TRAPPED = value;
    } else if (param == "UNDEVELOPED_KNIGHT") {
        UNDEVELOPED_KNIGHT = value;
    } else if (param == "UNDEVELOPED_BISHOP") {
        UNDEVELOPED_BISHOP = value;
    } else if (param == "VAL_WINDOW") {
        VAL_WINDOW = value;
    } else if (param == "UNPROTECTED_PAWNS") {
        UNPROTECTED_PAWNS = value;
    } else {
        res = false;
    }
    return res;
#else
    cout << param << " " << value << endl;
    return false;
#endif
}





