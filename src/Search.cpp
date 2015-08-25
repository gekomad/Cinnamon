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

#include <unistd.h>
#include "Search.h"
#include "SearchManager.h"

Hash *Search::hash;
Tablebase *Search::gtb;

bool Search::runningThread;


void Search::run() {
    if (getRunning()) {
        threadValue = search(mainSmp, mainDepth, mainAlpha, mainBeta);
    }
}

void Search::endRun() {
    ASSERT(pvsMode == false);
    if (pvsMode) {
        takeback(&mainMove, oldKeyPVS, true);
    }
    notifySearch(getId());
}

void Search::run(bool pvsplit, bool smp, int depth, int alpha, int beta) {
    setMainParam(pvsplit, smp, depth, alpha, beta);
    run();
}

Search::Search() : ponder(false), nullSearch(false) {
#ifdef DEBUG_MODE
    lazyEvalCuts = cumulativeMovesCount = totGen = 0;
#endif
    gtb = nullptr;
    hash = &Hash::getInstance();
}

void Search::clone(const Search *s) {
    memcpy(chessboard, s->chessboard, sizeof(_Tchessboard));
}

int Search::printDtm() {
    int side = getSide();
    u64 friends = side == WHITE ? getBitBoard<WHITE>() : getBitBoard<BLACK>();
    u64 enemies = side == BLACK ? getBitBoard<WHITE>() : getBitBoard<BLACK>();
    display();
    int res = side ? getGtb().getDtm<WHITE, true>(chessboard, chessboard[RIGHT_CASTLE_IDX], 100)
                   : getGtb().getDtm<BLACK, true>(chessboard, chessboard[RIGHT_CASTLE_IDX], 100);
    cout << " res: " << res;
    incListId();
    generateCaptures(side, enemies, friends);
    generateMoves(side, friends | enemies);
    _Tmove *move;
    u64 oldKey = 0;
    cout << "\n succ. \n";
    int best = -_INFINITE;
    for (int i = 0; i < getListSize(); i++) {
        move = &gen_list[listId].moveList[i];
        makemove(move, false, false);
        cout << "\n" << decodeBoardinv(move->type, move->from, getSide()) <<
        decodeBoardinv(move->type, move->to, getSide()) << " ";
        res = side ? -getGtb().getDtm<BLACK, true>(chessboard, chessboard[RIGHT_CASTLE_IDX], 100)
                   : getGtb().getDtm<WHITE, true>(chessboard, chessboard[RIGHT_CASTLE_IDX], 100);
        if (res != -INT_MAX) {
            cout << " res: " << res;
        }
        cout << "\n";
        takeback(move, oldKey, false);
        if (res > best) {
            best = res;
        }
    }
    if (best > 0) {
        best = _INFINITE - best;
    } else if (best < 0) {
        best = -(_INFINITE - best);
    }
    cout << endl;
    decListId();
    return best;
}

void Search::setNullMove(bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    ftime(&startTime);
}

void Search::setMainPly(int m) {
    mainDepth = m;
}

int Search::checkTime() {
    if (getRunning() == 2) {
        return 2;
    }
    if (ponder) {
        return 1;
    }
    struct timeb t_current;
    ftime(&t_current);
    return _time::diffTime(t_current, startTime) >= maxTimeMillsec ? 0 : 1;
}

Search::~Search() {
    join();
    if (gtb) {
        delete gtb;
        gtb = nullptr;
    }
}

template<int side, bool smp>
int Search::quiescence(int alpha, int beta, const char promotionPiece, int N_PIECE, int depth) {
    if (!getRunning()) {
        return 0;
    }
    ASSERT(chessboard[KING_BLACK + side]);
    if (!(numMovesq++ & 1023)) {
        setRunning(checkTime());
    }
    int score = getScore(side, alpha, beta);
    if (score >= beta) {
        return beta;
    }
    ///************* hash ****************
    char hashf = Hash::hashfALPHA;
    u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^RANDSIDE[side];
    Hash::_Thash *rootHash[2] = {nullptr, nullptr};
    /////////////////////greater
    Hash::_Thash phasheGreater;
    bool hashGreater = false;
    if (hash->readHash<smp>(rootHash, Hash::HASH_GREATER, zobristKeyR, phasheGreater)) {
        if (phasheGreater.from != phasheGreater.to && phasheGreater.flags & 0x3) {    // hashfEXACT or hashfBETA
            hashGreater = true;
        }
        if (phasheGreater.depth >= depth) {
            INC(hash->probeHash);
            if (!currentPly) {
                if (phasheGreater.flags == Hash::hashfBETA) {
                    incKillerHeuristic(phasheGreater.from, phasheGreater.to, 1);
                }
            } else {
                if (phasheGreater.flags == Hash::hashfALPHA) {
                    if (phasheGreater.score <= alpha) {
                        INC(hash->n_cut_hashA);
                        return alpha;
                    }
                } else {
                    ASSERT(phasheGreater.flags == Hash::hashfEXACT || phasheGreater.flags == Hash::hashfBETA);
                    if (phasheGreater.score >= beta) {
                        INC(hash->n_cut_hashB);
                        return beta;
                    }
                }
            }
        }
        INC(hash->cutFailed);

    }

//////////////////////////////////////////always

    Hash::_Thash phasheAlways;
    bool hashAlways = false;

    if (hash->readHash<smp>(rootHash, Hash::HASH_ALWAYS, zobristKeyR, phasheAlways)) {
        if (phasheAlways.from != phasheAlways.to && phasheAlways.flags & 0x3) {    // hashfEXACT or hashfBETA
            hashAlways = true;
        }
        if (phasheAlways.depth >= depth) {
            INC(hash->probeHash);
            if (!currentPly) {
                if (phasheAlways.flags == Hash::hashfBETA) {
                    incKillerHeuristic(phasheAlways.from, phasheAlways.to, 1);
                }
            } else {
                if (phasheAlways.flags == Hash::hashfALPHA) {
                    if (phasheAlways.score <= alpha) {
                        INC(hash->n_cut_hashA);
                        return alpha;
                    }
                } else {
                    ASSERT(phasheAlways.flags == Hash::hashfEXACT || phasheAlways.flags == Hash::hashfBETA);
                    if (phasheAlways.score >= beta) {
                        INC(hash->n_cut_hashB);
                        return beta;
                    }
                }
            }
        }
        INC(hash->cutFailed);
    }

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

    u64 friends = getBitBoard<side>();
    u64 enemies = getBitBoard<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();

        return _INFINITE - (mainDepth + depth);
    }
    if (!getListSize()) {
        --listId;
        return score;
    }
    _Tmove *move;
    _Tmove *best = nullptr;
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
    if (hashGreater) {
        sortHashMoves(listId, phasheGreater);
    } else if (hashAlways) {
        sortHashMoves(listId, phasheAlways);
    }
    while ((move = getNextMove(&gen_list[listId]))) {
        if (!makemove(move, false, true)) {
            takeback(move, oldKey, false);
            continue;
        }
/**************Delta Pruning ****************/
        if (fprune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) &&
            fscore + PIECES_VALUE[move->capturedPiece] <= alpha) {
            INC(nCutFp);
            takeback(move, oldKey, false);
            continue;
        }
/************ end Delta Pruning *************/
        int val = -quiescence<side ^ 1, smp>(-beta, -alpha, move->promotionPiece, N_PIECE - 1, depth - 1);
        score = max(score, val);
        takeback(move, oldKey, false);
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                ASSERT(rootHash[Hash::HASH_GREATER]);
                ASSERT(rootHash[Hash::HASH_ALWAYS]);
                hash->recordHash<smp>(zobristKeyR, getRunning(), rootHash, depth, Hash::hashfBETA, zobristKeyR, score,
                                      move);
                return beta;
            }
            best = move;
            alpha = score;
            hashf = Hash::hashfEXACT;
        }
    }
    ASSERT(rootHash[Hash::HASH_GREATER]);
    ASSERT(rootHash[Hash::HASH_ALWAYS]);
    hash->recordHash<smp>(zobristKeyR, getRunning(), rootHash, depth, hashf, zobristKeyR, score, best);

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

int Search::getRunning() {
    if (!runningThread)return 0;
    return GenMoves::getRunning();

}

void Search::setMaxTimeMillsec(int n) {
    maxTimeMillsec = n;
}

int Search::getMaxTimeMillsec() {
    return maxTimeMillsec;
}

void Search::sortHashMoves(int listId1, Hash::_Thash &phashe) {
    for (int r = 0; r < gen_list[listId1].size; r++) {
        _Tmove *mos = &gen_list[listId1].moveList[r];

        if (phashe.from == mos->from && phashe.to == mos->to) {
            mos->score = _INFINITE / 2;
            return;
        }
    }
}

bool Search::checkInsufficientMaterial(int N_PIECE) {
    //regexp: KN?B*KB*
    if (N_PIECE > 6) {
        return false;
    }
    // KK
    if (N_PIECE == 2) {
        return true;
    }
    if (!chessboard[PAWN_BLACK] && !chessboard[PAWN_WHITE] && !chessboard[ROOK_BLACK] && !chessboard[ROOK_WHITE] &&
        !chessboard[QUEEN_WHITE] && !chessboard[QUEEN_BLACK]) {
        u64 allBishop = chessboard[BISHOP_BLACK] | chessboard[BISHOP_WHITE];
        u64 allKnight = chessboard[KNIGHT_BLACK] | chessboard[KNIGHT_WHITE];
        if (allBishop || allKnight) {
            //insufficient material to mate
            if (!allKnight) {
                //regexp: KB+KB*
                if ((Bits::bitCount(allBishop) == 1) ||
                    ((allBishop & BLACK_SQUARES) == allBishop || (allBishop & WHITE_SQUARES) == allBishop)) {
                    return true;
                }
            } else {
                //KNKN*
                if (!allBishop && Bits::bitCount(allKnight) < 3) {
                    return true;
                }
            }
        }
    }
    return false;
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

bool Search::getGtbAvailable() {
    return gtb;
}

Tablebase &Search::getGtb() const {
    return *gtb;
}

void Search::deleteGtb() {
    if (gtb) {
        delete gtb;
    }
    gtb = nullptr;
}

void Search::setMainParam(bool pvsplit, bool smp, int depth, int alpha, int beta) {
    ASSERT(pvsMode == false);
    pvsMode = pvsplit;//TODO eliminare
    memset(&pvLine, 0, sizeof(_TpvLine));
    mainDepth = depth;
    mainAlpha = alpha;
    mainSmp = smp;
    mainBeta = beta;

}

void Search::setPVSplit(const int depth, const int alpha, const int beta, _Tmove *move) {//TODO eliminare
    setMainParam(true, true, depth, alpha, beta);
    oldKeyPVS = chessboard[ZOBRISTKEY_IDX];
    memcpy(&mainMove, move, sizeof(_Tmove));
    makemove(move, true, false);
}


int Search::search(bool smp, int depth, int alpha, int beta) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    if (smp) {
        return getSide() ? search<WHITE, true>(depth, alpha, beta, &pvLine,
                                               Bits::bitCount(getBitBoard<WHITE>() | getBitBoard<BLACK>()), &mainMateIn)
                         : search<BLACK, true>(depth, alpha, beta, &pvLine,
                                               Bits::bitCount(getBitBoard<WHITE>() | getBitBoard<BLACK>()),
                                               &mainMateIn);
    } else {
        return getSide() ? search<WHITE, false>(depth, alpha, beta, &pvLine,
                                                Bits::bitCount(getBitBoard<WHITE>() | getBitBoard<BLACK>()),
                                                &mainMateIn)
                         : search<BLACK, false>(depth, alpha, beta, &pvLine, Bits::bitCount(
                                                        getBitBoard<WHITE>() |
                                                        getBitBoard<BLACK>()),
                                                &mainMateIn);
    }
}


template<int side, bool smp>
int Search::search(int depth, int alpha, int beta, _TpvLine *pline, int N_PIECE, int *mateIn) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    INC(cumulativeMovesCount);
    *mateIn = INT_MAX;
    ASSERT_RANGE(side, 0, 1);
    if (!getRunning()) {
        return 0;
    }
    int score = -_INFINITE;
    /* gtb */
    if (gtb && pline->cmove && maxTimeMillsec > 1000 && gtb->isInstalledPieces(N_PIECE) &&
        depth >= gtb->getProbeDepth()) {
        int v = gtb->getDtm<side, false>(chessboard, (uchar) chessboard[RIGHT_CASTLE_IDX], depth);
        if (abs(v) != INT_MAX) {
            *mateIn = v;
            int res = 0;
            if (v == 0) {
                res = 0;
            } else {
                res = _INFINITE - (abs(v));
                if (v < 0) {
                    res = -res;
                }
            }
            ASSERT_RANGE(res, -_INFINITE, _INFINITE);
            ASSERT(mainDepth >= depth);
            cout << side << " " << (*mateIn) << " " << res << "\n";
            return res;
        }
    }
    u64 oldKey = chessboard[ZOBRISTKEY_IDX];
#ifdef DEBUG_MODE
    double betaEfficiencyCount = 0.0;
#endif
    ASSERT(chessboard[KING_BLACK]);
    ASSERT(chessboard[KING_WHITE]);
    ASSERT(chessboard[KING_BLACK + side]);
    int extension = 0;
    int is_incheck_side = inCheck<side>();
    if (!is_incheck_side && depth != mainDepth) {
        if (checkInsufficientMaterial(N_PIECE)) {
            if (inCheck<side ^ 1>()) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -lazyEval<side>() * 2;
        }
        if (checkDraw(chessboard[ZOBRISTKEY_IDX])) {
            return -lazyEval<side>() * 2;
        }
    }
    if (is_incheck_side) {
        extension++;
    }
    depth += extension;
    if (depth == 0) {
        return quiescence<side, smp>(alpha, beta, -1, N_PIECE, 0);
    }
    //************* hash ****************
    u64 zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^RANDSIDE[side];
    Hash::_Thash *rootHash[2] = {nullptr, nullptr};
    //////////////////////// greater

    Hash::_Thash phasheGreater;

    bool hashGreater = false;

    if (hash->readHash<smp>(rootHash, Hash::HASH_GREATER, zobristKeyR, phasheGreater)) {
        if (phasheGreater.from != phasheGreater.to && phasheGreater.flags & 0x3) {    // hashfEXACT or hashfBETA
            hashGreater = true;
        }
        if (phasheGreater.depth >= depth) {
            INC(hash->probeHash);
            if (!currentPly) {
                if (phasheGreater.flags == Hash::hashfBETA) {
                    incKillerHeuristic(phasheGreater.from, phasheGreater.to, 1);
                }
            } else {
                switch (phasheGreater.flags) {
                    case Hash::hashfEXACT:
                        INC(hash->n_cut_hashE);
                        if (phasheGreater.score >= beta) {
                            return beta;
                        }
                        break;
                    case Hash::hashfBETA:
                        incKillerHeuristic(phasheGreater.from, phasheGreater.to, 1);
                        if (phasheGreater.score >= beta) {
                            INC(hash->n_cut_hashB);
                            return beta;
                        }
                        break;
                    case Hash::hashfALPHA:
                        if (phasheGreater.score <= alpha) {
                            INC(hash->n_cut_hashA);
                            return alpha;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
        INC(hash->cutFailed);
    }

    //////////////////////// always
    Hash::_Thash phasheAlways;
    bool hashAlways = false;


    if (hash->readHash<smp>(rootHash, Hash::HASH_ALWAYS, zobristKeyR, phasheAlways)) {
        if (phasheAlways.from != phasheAlways.to && phasheAlways.flags & 0x3) {    // hashfEXACT or hashfBETA
            hashAlways = true;
        }
        if (phasheAlways.depth >= depth) {
            INC(hash->probeHash);
            if (!currentPly) {
                if (phasheAlways.flags == Hash::hashfBETA) {
                    incKillerHeuristic(phasheAlways.from, phasheAlways.to, 1);
                }
            } else {
                switch (phasheAlways.flags) {
                    case  Hash::hashfEXACT:
                        INC(hash->n_cut_hashE);
                        if (phasheAlways.score >= beta) {
                            return beta;
                        }
                        break;
                    case  Hash::hashfBETA:
                        incKillerHeuristic(phasheAlways.from, phasheAlways.to, 1);
                        if (phasheAlways.score >= beta) {
                            INC(hash->n_cut_hashB);
                            return beta;
                        }
                        break;
                    case  Hash::hashfALPHA:
                        if (phasheAlways.score <= alpha) {
                            INC(hash->n_cut_hashA);
                            return alpha;
                        }
                        break;
                    default:
                        break;
                }
                INC(hash->cutFailed);
            }
        }
        INC(hash->cutFailed);
    }

    ///********** end hash ***************
    if (!(numMoves & 1023)) {
        setRunning(checkTime());
    }
    ++numMoves;
    ///********* null move ***********
    int n_pieces_side;
    _TpvLine line;
    line.cmove = 0;
    if (!is_incheck_side && !nullSearch && depth >= NULLMOVE_DEPTH &&
        (n_pieces_side = getNpiecesNoPawnNoKing<side>()) >= NULLMOVES_MIN_PIECE) {
        nullSearch = true;
        int nullScore = -search<side ^ 1, smp>(
                depth - (NULLMOVES_R1 + (depth > (NULLMOVES_R2 + (n_pieces_side < NULLMOVES_R3 ? NULLMOVES_R4 : 0)))) -
                1, -beta, -beta + 1, &line, N_PIECE, mateIn);
        nullSearch = false;
        if (nullScore >= beta) {
            INC(nNullMoveCut);
            return nullScore;
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
            if (depth == 3 && (matBalance + RAZOR_MARGIN) <= alpha && getNpiecesNoPawnNoKing<side ^ 1>() > 3) {
                INC(nCutRazor);
                depth--;
            } else
                ///**************Futility Pruning at pre-frontier*****
            if (depth == 2 && (futilScore = matBalance + EXT_FUTILY_MARGIN) <= alpha) {
                futilPrune = true;
                score = futilScore;
            } else
                ///**************Futility Pruning at frontier*****
            if (depth == 1 /*&& (futilScore = matBalance + FUTIL_MARGIN) <= alpha */) {
                futilPrune = true;
                score = futilScore;
            }
        }
    }
    /************ end Futility Pruning*************/
    incListId();
    ASSERT_RANGE(KING_BLACK + side, 0, 11);
    ASSERT_RANGE(KING_BLACK + (side ^ 1), 0, 11);
    u64 friends = getBitBoard<side>();
    u64 enemies = getBitBoard<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        score = _INFINITE - (mainDepth - depth + 1);
        return score;
    }
    generateMoves<side>(friends | enemies);
    int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (is_incheck_side) {
            return -_INFINITE + (mainDepth - depth + 1);
        } else {
            return -lazyEval<side>() * 2;
        }
    }
    _Tmove *best = nullptr;
    if (hashGreater) {
        sortHashMoves(listId, phasheGreater);
    } else if (hashAlways) {
        sortHashMoves(listId, phasheAlways);
    }
    INC(totGen);
    _Tmove *move;
    bool checkInCheck = false;
    int countMove = 0;
    char hashf = Hash::hashfALPHA;
    while ((move = getNextMove(&gen_list[listId]))) {
        countMove++;
        INC(betaEfficiencyCount);
        if (!makemove(move, true, checkInCheck)) {
            takeback(move, oldKey, true);
            continue;
        }
        checkInCheck = true;
        if (futilPrune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) &&
            futilScore + PIECES_VALUE[move->capturedPiece] <= alpha && !inCheck<side>()) {
            INC(nCutFp);
            takeback(move, oldKey, true);
            continue;
        }
        //Late Move Reduction
        int val = INT_MAX;
        if (countMove > 4 && !is_incheck_side && depth >= 3 && move->capturedPiece == SQUARE_FREE &&
            move->promotionPiece == NO_PROMOTION) {
            currentPly++;
            val = -search<side ^ 1, smp>(depth - 2, -(alpha + 1), -alpha, &line, N_PIECE, mateIn);
            ASSERT(val != INT_MAX);
            currentPly--;
        }
        if (val > alpha) {
            int doMws = (score > -_INFINITE + MAX_PLY);
            int lwb = max(alpha, score);
            int upb = (doMws ? (lwb + 1) : beta);
            currentPly++;
            val = -search<side ^ 1, smp>(depth - 1, -upb, -lwb, &line,
                                         move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1, mateIn);
            ASSERT(val != INT_MAX);
            currentPly--;
            if (doMws && (lwb < val) && (val < beta)) {
                currentPly++;
                val = -search<side ^ 1, smp>(depth - 1, -beta, -val + 1, &line,
                                             move->capturedPiece == SQUARE_FREE ? N_PIECE : N_PIECE - 1, mateIn);
                currentPly--;
            }
        }
        score = max(score, val);
        takeback(move, oldKey, true);
        move->score = score;
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                ASSERT(move->score == score);
                INC(nCutAB);
                ADD(betaEfficiency, betaEfficiencyCount / (double) listcount * 100.0);
                ASSERT(rootHash[Hash::HASH_GREATER]);
                ASSERT(rootHash[Hash::HASH_ALWAYS]);
                hash->recordHash<smp>(zobristKeyR, getRunning(), rootHash, depth - extension, Hash::hashfBETA,
                                      zobristKeyR, score, move);
                setKillerHeuristic(move->from, move->to, 0x400);
                return score;
            }
            alpha = score;
            hashf = Hash::hashfEXACT;
            best = move;
            move->score = score;    //used in it
            updatePv(pline, &line, move);
        }
    }
    ASSERT(rootHash[Hash::HASH_GREATER]);
    ASSERT(rootHash[Hash::HASH_ALWAYS]);
    hash->recordHash<smp>(zobristKeyR, getRunning(), rootHash, depth - extension, hashf, zobristKeyR, score, best);
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

_Tchessboard &Search::getChessboard() {
    return chessboard;
}

void Search::setChessboard(_Tchessboard &b) {
    memcpy(chessboard, b, sizeof(chessboard));
}

u64 Search::getZobristKey() {
    return chessboard[ZOBRISTKEY_IDX];
}

int Search::getMateIn() {
    return mainMateIn;
}

void Search::setGtb(Tablebase &tablebase) {
    gtb = &tablebase;
}

