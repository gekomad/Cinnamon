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
        if (searchMovesVector.size()) {
            if (sideToMove == WHITE)
                aspirationWindow<WHITE, true>(mainDepth, valWindow);
            else
                aspirationWindow<BLACK, true>(mainDepth, valWindow);
        } else {
            if (sideToMove == WHITE)
                aspirationWindow<WHITE, false>(mainDepth, valWindow);
            else
                aspirationWindow<BLACK, false>(mainDepth, valWindow);
        }
    }
}

template<uchar side, bool searchMoves>
void Search::aspirationWindow(const int depth, const int valWin) {
    valWindow = valWin;
    init();
    const auto nPieces = bitCount(board::getBitmap<WHITE>(chessboard) | board::getBitmap<BLACK>(chessboard));
    if (depth < 3) {
        valWindow = search<side, searchMoves>(depth, -_INFINITE, _INFINITE, &pvLine, nPieces);
    } else {
        int tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW, &pvLine, nPieces);
        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW * 3, valWindow + VAL_WINDOW, &pvLine,
                                                nPieces);
            } else {
                tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 3, &pvLine,
                                                nPieces);
            }
            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW * 6, valWindow + VAL_WINDOW, &pvLine,
                                                    nPieces);
                } else {
                    tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 6, &pvLine,
                                                    nPieces);
          }
                if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                    tmp = search<side, searchMoves>(depth, -_INFINITE - 1, _INFINITE + 1, &pvLine, nPieces);
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

void Search::setNullMove(const bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Search::setMainPly(const int ply, const int iter_depth) {
    mainDepth = iter_depth;
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
    if (!getRunning()) return 0;
    ++numMovesq;
    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
    int score = eval.getScore(chessboard, zobristKeyR, side, alpha, beta);
    if (score > alpha) {
        if (score >= beta) return score;
        alpha = score;
    }

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
        if (!makemove(move, false)) {
            takeback(move, oldKey, oldEnpassant, false);
            continue;
        }

        // if (badCapure<side>(move, friends | enemies)) {
        //     INC(nCutBadCaputure);
        //     takeback(move, oldKey, oldEnpassant, false);
        //     continue;
        // }
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

bool Search::checkDraw(const u64 key) const {
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

void Search::setMainParam(const int iter_depth) {
    memset(&pvLine, 0, sizeof(_TpvLine));
    mainDepth = iter_depth;
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


template<uchar side, bool checkMoves>
int Search::search(const int depth, int alpha, int beta, _TpvLine *pline, const int N_PIECE) {
    ASSERT_RANGE(side, 0, 1)
    if (!getRunning()) return 0;
    const int oldAlpha = alpha;

    const auto searchLambda = [&](_TpvLine *newLine, const int depth, const int alpha, const int beta,
                                  const _Tmove *move) {
        const auto nPieces = move ? (move->capturedPiece == SQUARE_EMPTY ? N_PIECE : N_PIECE - 1) : N_PIECE;
        currentPly++;
        int val = -search<X(side), checkMoves>(depth, alpha, beta, newLine, nPieces);
        if (!forceCheck && abs(val) > _INFINITE - MAX_PLY) {
            forceCheck = true;
            val = -search<X(side), checkMoves>(depth, alpha, beta, newLine, nPieces);
            forceCheck = false;
        }
        currentPly--;
        return val;
    };


    const u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    const uchar oldEnpassant = enPassant;
    if (depth >= MAX_PLY - 1) {
        return eval.getScore(chessboard, oldKey, side, alpha, beta);
    }
    INC(cumulativeMovesCount);
#ifndef JS_MODE
    // int wdl = TB::probeWdl(depth, side, N_PIECE, mainDepth, rightCastle, chessboard);
    // if (wdl != INT_MAX) return wdl;
#endif
    const bool pvNode = alpha != beta - 1;
    ASSERT(chessboard[KING_BLACK]);
    ASSERT(chessboard[KING_WHITE]);
    const bool isIncheckSide = board::inCheck1<side>(chessboard);
    if (!isIncheckSide && depth != mainDepth) {
        if (board::checkInsufficientMaterial(N_PIECE, chessboard) || checkDraw(chessboard[ZOBRISTKEY_IDX])) {
            if (board::inCheck1<X(side)>(chessboard)) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -eval.lazyEval<side>(chessboard) * 2;
        }
    }
    int extension = 0;//isIncheckSide;
    if (depth + extension == 0) {
        return qsearch<side>(alpha, beta, NO_PROMOTION, 0);
    }

    /// ************* hash ****************
    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
    u64 hashItem;
    const int ttScore = hash.readHash(alpha, beta, depth, zobristKeyR, hashItem, currentPly);
    if (ttScore != INT_MAX)
        return ttScore;

    /// ********** end hash ***************

    if (!(numMoves % 2048)) setRunning(checkTime());
    ++numMoves;
    int futilScore = 0;
    bool futilPrune = false;
    int score = -_INFINITE;
    if (!isIncheckSide && !pvNode) {
        const int matBalance = eval.lazyEval<side>(chessboard);
        /// ******** reverse futility pruning ***********
        if (depth < 8 && abs(beta - 1) > -_INFINITE + MAX_PLY) {
            const int evalMargin = matBalance - eval.REVERSE_FUTIL_MARGIN * depth;
            if (evalMargin >= beta)  {
                INC(rfcCut);
                return beta;
            }
        }
        /// ******** razor pruning ***********
        if (depth <= 3) {
            if (matBalance + _eval::RAZOR_MARGIN[depth] <= alpha) {
                if (depth == 1) {
                    INC(nCutRazor);
                    return qsearch<side>(alpha, beta, NO_PROMOTION, 0);
                }
                const int rAlpha = alpha - _eval::RAZOR_MARGIN[depth];
                const int v = qsearch<side>(rAlpha, rAlpha+1, NO_PROMOTION, 0);
                if (v <= rAlpha) {
                    INC(nCutRazor);
                    return v;
                }
            }
        }

        if ((futilScore = matBalance + eval.FUTIL_MARGIN) <= alpha) {
         /// **************Futility Pruning at pre-frontier *****
            if (depth == 2 && (futilScore = matBalance + eval.EXT_FUTIL_MARGIN) <= alpha) {
                futilPrune = true;
                score = futilScore;
            } else
            /// **************Futility Pruning at frontier *****
            if (depth == 1) {
                futilPrune = true;
                score = futilScore;
            }
        }
    }

    _Tmove *best = nullptr;
    ASSERT_RANGE(KING_BLACK + side, 0, 11)
    ASSERT_RANGE(KING_BLACK + (X(side)), 0, 11)
    const u64 friends = board::getBitmap<side>(chessboard);
    const u64 enemies = board::getBitmap<X(side)>(chessboard);
    incListId();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        return _INFINITE - (mainDepth - depth + 1);
    }
    generateMoves<side>(friends | enemies);
    const int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (isIncheckSide) return -_INFINITE + (mainDepth - depth + 1);
        return -eval.lazyEval<side>(chessboard) * 2;
                   // TODO se ho meno materiale dell'avversario è positivo altrimenti negativo

    }
    ASSERT(genList[listId].size > 0);
    // _Tmove *best = &genList[listId].moveList[0];
    INC(totGen);
    _Tmove *move;
    int countMove = 0;
    // char hashf = Hash::hashfALPHA;
    int first = 0;


    while ((move = getNextMove(&genList[listId], depth, hashItem, first++))) {
        if (!checkSearchMoves<checkMoves>(move) && depth == mainDepth)
            continue;
        countMove++;

        if (!makemove(move, true)) {
            takeback(move, oldKey, oldEnpassant, true);
            continue;
        }
        int val = INT_MAX;
        _TpvLine newLine;
        newLine.cmove = 0;

        if (move->promotionPiece == NO_PROMOTION) {
            if (futilPrune && futilScore + PIECES_VALUE[move->capturedPiece] <= alpha &&
                !board::inCheck1<side>(chessboard)) {
                INC(nCutFp);
                takeback(move, oldKey, oldEnpassant, true);
                continue;
                }
            //Late Move Reduction
            if (countMove > 3 && !isIncheckSide && depth >= 3 && move->capturedPiece == SQUARE_EMPTY) {
                val = searchLambda(&newLine, depth + extension - (countMove > 6 ? 3 : 2), -(alpha + 1), -alpha,
                                   nullptr);
            }
        }
        // PVS
        if (val > alpha) {
            const int doMws = (score > -_INFINITE + MAX_PLY);
            const int lwb = max(alpha, score);
            const int upb = doMws ? lwb + 1 : beta;
            DEBUG(if (-upb == -lwb-1) pvsTot++);
            val = searchLambda(&newLine, depth + extension - 1, -upb, -lwb, move);
            if (doMws && (lwb < val) && (val < beta)) {
                val = searchLambda(&newLine, depth + extension - 1, -beta, -val + 1, move);
            } DEBUG(else if (-upb == -lwb-1) pvsOK++);
        }
        score = max(score, val);
        takeback(move, oldKey, oldEnpassant, true);
        ASSERT(chessboard[KING_BLACK]);
        ASSERT(chessboard[KING_WHITE]);
         if (score > alpha) {
          if (score >= beta) {
            INC(nCutAB);
            INC(betaEfficiencyCount);
            DEBUG(betaEfficiency +=
                          (100.0 - ((double) countMove * 100.0 / (double) listcount)) +
                          (((double) countMove * 100.0 / (double) listcount) / (double) countMove))
            if (getRunning()) {
                if (move->capturedPiece == SQUARE_EMPTY && move->promotionPiece == NO_PROMOTION) {
                    setHistoryHeuristic(move->pieceFrom, move->to, depth);
                    }
                }

               best = move;
               updatePv(pline, &newLine, move);
               break;
            }

            alpha = score;
            best = move;
            updatePv(pline, &newLine, move);
        }
    }
    decListId();
    if (best) {
        const char hashf =
                (score <= oldAlpha) ? Hash::hashfALPHA :
                (score >= beta) ? Hash::hashfBETA : Hash::hashfEXACT;
        Hash::_Thash data(zobristKeyR, score, depth, best->from, best->to, hashf);
        hash.recordHash(data, ply);
    }

    return score;

}

void Search::updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move) {

    ASSERT(line->cmove < MAX_PLY - 1);
    memcpy(&(pline->argmove[0]), move, sizeof(_Tmove));
    memcpy(pline->argmove + 1, line->argmove, line->cmove * sizeof(_Tmove));
    ASSERT(line->cmove >= 0);
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

template<uchar side>
bool Search::badCapure(const _Tmove &move, const u64 allpieces) const {

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


