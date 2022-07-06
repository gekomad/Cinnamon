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


    if (depth == 1) {
        valWindow = search<side, searchMoves>(depth, -_INFINITE - 1, _INFINITE + 1, &pvLine, nPieces);
    } else {
        int tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW, &pvLine, nPieces);
        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW, &pvLine,
                                                nPieces);
            } else {
                tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2, &pvLine,
                                                nPieces);
            }
            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW, &pvLine,
                                                    nPieces);
                } else {
                    tmp = search<side, searchMoves>(depth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4, &pvLine,
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
        if (!makemove(move, false)) {
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
int Search::search(const int depth, int alpha, const int beta, _TpvLine *pline, const int N_PIECE) {
    ASSERT_RANGE(side, 0, 1)
    if (!getRunning()) return 0;


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

    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    uchar oldEnpassant = enPassant;
    if (depth >= MAX_PLY - 1) {
        return eval.getScore(chessboard, oldKey, side, alpha, beta);
    }
    INC(cumulativeMovesCount);
#ifndef JS_MODE
    int wdl = TB::probeWdl(depth, side, N_PIECE, mainDepth, rightCastle, chessboard);
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
    const u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
    u64 hashItem;
    const int hashValue = hash.readHash(alpha, beta, depth, zobristKeyR, hashItem, currentPly);
    if (hashValue != INT_MAX) {
        return hashValue;
    }

    /// ********** end hash ***************

    if (!(numMoves % 2048)) setRunning(checkTime());
    ++numMoves;
    _TpvLine newLine1;
    newLine1.cmove = 0;

    /// ********* null move ***********
    if (!nullSearch && !pvNode && !isIncheckSide) {
        int nDepth = (depth > 3) ? 1 : 3;
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
                nullScore = searchLambda(&newLine1, depth + extension - R - 1, -beta, -beta + 1, nullptr);
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
    int countMove = 0;
    char hashf = Hash::hashfALPHA;
    int first = 0;

    while ((move = getNextMove(&genList[listId], depth, hashItem, first++))) {
        if (!checkSearchMoves<checkMoves>(move) && depth == mainDepth) continue;
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

        if (val > alpha) {
            const int doMws = (score > -_INFINITE + MAX_PLY);
            const int lwb = max(alpha, score);
            const int upb = (doMws ? (lwb + 1) : beta);
            val = searchLambda(&newLine, depth + extension - 1, -upb, -lwb, move);
            if (doMws && (lwb < val) && (val < beta)) {
                val = searchLambda(&newLine, depth + extension - 1, -beta, -val + 1, move);
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

                    if (move->capturedPiece == SQUARE_EMPTY && move->promotionPiece == NO_PROMOTION) {
                        setHistoryHeuristic(move->from, move->to, depth);
                    }
                }
                return score;
            }
            alpha = score;
            hashf = Hash::hashfEXACT;
            best = move;
            updatePv(pline, &newLine, move);
        }
    }
    if (getRunning()) {
        if (best->capturedPiece == SQUARE_EMPTY && best->promotionPiece == NO_PROMOTION && (depth - extension) >= 0) {
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

int Search::getParameter(const string &p, const int phase) {
//    if (p == "ATTACK_KING")return eval.ATTACK_KING[phase];
//    if (p == "BISHOP_ON_QUEEN")return eval.BISHOP_ON_QUEEN[phase];
    if (p == "BACKWARD_PAWN")return eval.BACKWARD_PAWN[phase];
    if (p == "DOUBLED_ISOLATED_PAWNS")return eval.DOUBLED_ISOLATED_PAWNS[phase];
//    if (p == "DOUBLED_PAWNS")return eval.DOUBLED_PAWNS[phase];
    if (p == "PAWN_IN_7TH")return eval.PAWN_IN_7TH[phase];
//    if (p == "PAWN_CENTER")return eval.PAWN_CENTER[phase];
    if (p == "PAWN_IN_PROMOTION")return eval.PAWN_IN_PROMOTION[phase];
//    if (p == "PAWN_ISOLATED")return eval.PAWN_ISOLATED[phase];
    if (p == "PAWN_NEAR_KING")return eval.PAWN_NEAR_KING[phase];
    if (p == "PAWN_BLOCKED")return eval.PAWN_BLOCKED[phase];
//    if (p == "UNPROTECTED_PAWNS")return eval.UNPROTECTED_PAWNS[phase];
//    if (p == "ENEMY_NEAR_KING")return eval.ENEMY_NEAR_KING[phase];
    if (p == "FRIEND_NEAR_KING")return eval.FRIEND_NEAR_KING[phase];
//    if (p == "HALF_OPEN_FILE_Q")return eval.HALF_OPEN_FILE_Q[phase];
    if (p == "BONUS2BISHOP")return eval.BONUS2BISHOP[phase];
    if (p == "BISHOP_PAWN_ON_SAME_COLOR")return eval.BISHOP_PAWN_ON_SAME_COLOR[phase];
//    if (p == "CONNECTED_ROOKS")return eval.CONNECTED_ROOKS[phase];
//    if (p == "OPEN_FILE")return eval.OPEN_FILE[phase];
    if (p == "OPEN_FILE_Q")return eval.OPEN_FILE_Q[phase];
    if (p == "ROOK_7TH_RANK")return eval.ROOK_7TH_RANK[phase];
//    if (p == "ROOK_BLOCKED")return eval.ROOK_BLOCKED[phase];
//    if (p == "ROOK_TRAPPED")return eval.ROOK_TRAPPED[phase];
//    if (p == "UNDEVELOPED_KNIGHT")return eval.UNDEVELOPED_KNIGHT[phase];
//    if (p == "UNDEVELOPED_BISHOP")return eval.UNDEVELOPED_BISHOP[phase];
    if (p == "KNIGHT_PINNED")return eval.KNIGHT_PINNED[phase];
    if (p == "ROOK_PINNED")return eval.ROOK_PINNED[phase];
    if (p == "BISHOP_PINNED")return eval.BISHOP_PINNED[phase];
    if (p == "ROOK_IN_7_KING_IN_8")return eval.ROOK_IN_7_KING_IN_8[phase];
    if (p == "MOB_QUEEN_")return eval.MOB_QUEEN_[phase];
    if (p == "MOB_ROOK_")return eval.MOB_ROOK_[phase];
    if (p == "MOB_BISHOP_")return eval.MOB_BISHOP_[phase];
    if (p == "MOB_KNIGHT_")return eval.MOB_KNIGHT_[phase];
    if (p == "MOB_KING_")return eval.MOB_KING_[phase];
    if (p == "PAWN_PASSED_")return eval.PAWN_PASSED_;
    if (p == "DISTANCE_KING_ENDING_")return eval.DISTANCE_KING_ENDING_;
    if (p == "DISTANCE_KING_OPENING_")return eval.DISTANCE_KING_OPENING_;
//    if (p == "QUEEN_PINNED")return eval.QUEEN_PINNED[phase];
//    if (p == "QUEEN_IN_7")return eval.QUEEN_IN_7[phase];
//    if (p == "ROOK_IN_7")return eval.ROOK_IN_7[phase];
//    if (p == "PAWN_PINNED")return eval.PAWN_PINNED[phase];
    fatal("Not found ", p)
    exit(1);
}

void Search::setParameter(const string &p, const int value, const int phase) {
    //cout << "setParameter " << param << " " << value << endl;
//    if (p == "ATTACK_KING")eval.ATTACK_KING[phase] = value;
//    else if (p == "BISHOP_ON_QUEEN")eval.BISHOP_ON_QUEEN[phase] = value;
     if (p == "BACKWARD_PAWN")eval.BACKWARD_PAWN[phase] = value;
    else if (p == "DOUBLED_ISOLATED_PAWNS")eval.DOUBLED_ISOLATED_PAWNS[phase] = value;
//    else if (p == "DOUBLED_PAWNS")eval.DOUBLED_PAWNS[phase] = value;
    else if (p == "PAWN_IN_7TH")eval.PAWN_IN_7TH[phase] = value;
//    else if (p == "PAWN_CENTER")eval.PAWN_CENTER[phase] = value;
    else if (p == "PAWN_IN_PROMOTION")eval.PAWN_IN_PROMOTION[phase] = value;
//    else if (p == "PAWN_ISOLATED")eval.PAWN_ISOLATED[phase] = value;
    else if (p == "PAWN_NEAR_KING")eval.PAWN_NEAR_KING[phase] = value;
    else if (p == "PAWN_BLOCKED")eval.PAWN_BLOCKED[phase] = value;
//    else if (p == "UNPROTECTED_PAWNS")eval.UNPROTECTED_PAWNS[phase] = value;
//    else if (p == "ENEMY_NEAR_KING")eval.ENEMY_NEAR_KING[phase] = value;
    else if (p == "FRIEND_NEAR_KING")eval.FRIEND_NEAR_KING[phase] = value;
//    else if (p == "HALF_OPEN_FILE_Q")eval.HALF_OPEN_FILE_Q[phase] = value;
    else if (p == "BONUS2BISHOP")eval.BONUS2BISHOP[phase] = value;
    else if (p == "BISHOP_PAWN_ON_SAME_COLOR")eval.BISHOP_PAWN_ON_SAME_COLOR[phase] = value;
//    else if (p == "CONNECTED_ROOKS")eval.CONNECTED_ROOKS[phase] = value;
//    else if (p == "OPEN_FILE")eval.OPEN_FILE[phase] = value;
    else if (p == "OPEN_FILE_Q")eval.OPEN_FILE_Q[phase] = value;
    else if (p == "ROOK_7TH_RANK")eval.ROOK_7TH_RANK[phase] = value;
//    else if (p == "ROOK_BLOCKED")eval.ROOK_BLOCKED[phase] = value;
//    else if (p == "ROOK_TRAPPED")eval.ROOK_TRAPPED[phase] = value;
//    else if (p == "UNDEVELOPED_KNIGHT")eval.UNDEVELOPED_KNIGHT[phase] = value;
//    else if (p == "UNDEVELOPED_BISHOP")eval.UNDEVELOPED_BISHOP[phase] = value;
    else if (p == "KNIGHT_PINNED")eval.KNIGHT_PINNED[phase] = value;
    else if (p == "ROOK_PINNED")eval.ROOK_PINNED[phase] = value;
    else if (p == "BISHOP_PINNED")eval.BISHOP_PINNED[phase] = value;
    else if (p == "ROOK_IN_7_KING_IN_8")eval.ROOK_IN_7_KING_IN_8[phase] = value;
    else if (p == "MOB_QUEEN_")eval.MOB_QUEEN_[phase] = value;
    else if (p == "MOB_ROOK_")eval.MOB_ROOK_[phase] = value;
    else if (p == "MOB_BISHOP_")eval.MOB_BISHOP_[phase] = value;
    else if (p == "MOB_KNIGHT_")eval.MOB_KNIGHT_[phase] = value;
    else if (p == "MOB_KING_")eval.MOB_KING_[phase] = value;
    else if (p == "PAWN_PASSED_")eval.PAWN_PASSED_ = value;
    else if (p == "DISTANCE_KING_ENDING_")eval.DISTANCE_KING_ENDING_ = value;
    else if (p == "DISTANCE_KING_OPENING_")eval.DISTANCE_KING_OPENING_ = value;

//    else if (p == "QUEEN_PINNED")eval.QUEEN_PINNED[phase] = value;
//    else if (p == "QUEEN_IN_7")eval.QUEEN_IN_7[phase] = value;
//    else if (p == "ROOK_IN_7")eval.ROOK_IN_7[phase] = value;
//    else if (p == "PAWN_PINNED")eval.PAWN_PINNED[phase] = value;
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


