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

GTB *Search::gtb;
SYZYGY *Search::syzygy = nullptr;
bool Search::runningThread;
high_resolution_clock::time_point Search::startTime;

void Search::run() {
    if (getRunning()) {
        if (mainSmp)
            aspirationWindow<SMP_YES>(mainDepth, valWindow);
        else
            aspirationWindow<SMP_NO>(mainDepth, valWindow);
    }
}

void Search::endRun() {
    SearchManager::getInstance().receiveObserverSearch(getId());
}

template<bool smp>
void Search::aspirationWindow(const int depth, const int valWin) {
    valWindow = valWin;
    init();

    if (depth == 1) {
        valWindow = search<SMP_NO>(depth, -_INFINITE - 1, _INFINITE + 1);
    } else {
        ASSERT(INT_MAX != valWindow);
        int tmp = search<smp>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);

        if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
            if (tmp <= valWindow - VAL_WINDOW) {
                tmp = search<smp>(mainDepth, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW);
            } else {
                tmp = search<smp>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2);
            }

            if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                if (tmp <= valWindow - VAL_WINDOW) {
                    tmp = search<smp>(mainDepth, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW);
                } else {
                    tmp = search<smp>(mainDepth, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4);
                }

                if (tmp <= valWindow - VAL_WINDOW || tmp >= valWindow + VAL_WINDOW) {
                    tmp = search<smp>(mainDepth, -_INFINITE - 1, _INFINITE + 1);
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
    gtb = nullptr;
}

int Search::printDtm() {
    int side = getSide();
    u64 friends = side == WHITE ? getBitmap<WHITE>() : getBitmap<BLACK>();
    u64 enemies = side == BLACK ? getBitmap<WHITE>() : getBitmap<BLACK>();
    display();
    int res = getGtb().getDtm(side, true, chessboard, chessboard.bit[RIGHT_CASTLE_IDX], 100);

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
        cout << "\n" << decodeBoardinv(move->type, move->from, getSide()) << decodeBoardinv(move->type, move->to, getSide()) << " ";
        res = -getGtb().getDtm(side ^ 1, true, chessboard, chessboard.bit[RIGHT_CASTLE_IDX], 100);
        if (res != -INT_MAX) {
            cout << " res: " << res;
        }
        cout << "\n";
        takeback<false>(move, oldKey);
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

void Search::setNullMove(const bool b) {
    nullSearch = !b;
}

void Search::startClock() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Search::setMainPly(const int m) {
    mainDepth = m;
}

int Search::checkTime() {
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
    deleteGtb();
}

template<int side, bool smp>
int Search::quiescence(int alpha, const int beta, const char promotionPiece, const int nPieces, const int depth) {
    if (!getRunning()) {
        return 0;
    }
    ASSERT(chessboard.bit[KING_BLACK + side]);
    if (!(numMovesq++ & 1023)) {
        setRunning(checkTime());
    }

//    if (!is_incheck_side) {
//        int score = lazyEval<side>();
//        if (score + FUTIL_MARGIN < alpha)
//            return score;
//    }

    int score = getScore(side, nPieces, alpha, beta, false);
    if (score >= beta) {
        return beta;
    }
    ///************* hash ****************
    char hashf = Hash::hashfALPHA;
    u64 zobristKeyR = chessboard.bit[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];

    _TcheckHash checkHashStruct;

    int r;
    if ((r = checkHash<Hash::HASH_GREATER, true>(alpha, beta, depth, zobristKeyR, checkHashStruct)) != INT_MAX) {
        return r;
    };
    if ((r = checkHash<Hash::HASH_ALWAYS, true>(alpha, beta, depth, zobristKeyR, checkHashStruct)) != INT_MAX) {
        return r;
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

    u64 friends = getBitmap<side>();
    u64 enemies = getBitmap<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();

        return _INFINITE - (mainDepth + depth);
    }
    if (!getListSize()) {
        --listId;
        return score;
    }
    _Tmove *move;
    ASSERT(gen_list[listId].size > 0);
    _Tmove *best = &gen_list[listId].moveList[0];
    u64 oldKey = chessboard.bit[ZOBRISTKEY_IDX];
    if (checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3 /*&& checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.from != checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.to*/) {// hashfEXACT or hashfBETA
        sortHashMoves(listId, checkHashStruct.phasheType[Hash::HASH_GREATER]);
    } else if (checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3 /*&& checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.from != checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.to*/) {// hashfEXACT or hashfBETA
        sortHashMoves(listId, checkHashStruct.phasheType[Hash::HASH_ALWAYS]);
    }
    while ((move = getNextMove(&gen_list[listId]))) {
        if (!makemove(move, false, true)) {
            takeback<false>(move, oldKey);
            continue;
        }
/**************Delta Pruning ****************/
        if (fprune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) && fscore + PIECES_VALUE[move->capturedPiece] <= alpha) {
            INC(nCutFp);
            takeback<false>(move, oldKey);
            continue;
        }
/************ end Delta Pruning *************/
        int val = -quiescence<side ^ 1, smp>(-beta, -alpha, move->promotionPiece, nPieces - 1, depth - 1);
        score = max(score, val);
        takeback<false>(move, oldKey);
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                if (getRunning()) {
                    _ThashData data(score, depth, move->from, move->to, 0, Hash::hashfBETA);
                    recordHash(zobristKeyR, data);
                }
                return beta;
            }
            best = move;
            alpha = score;
            hashf = Hash::hashfEXACT;
        }
    }
    if (getRunning()) {
        _ThashData data(score, depth, best->from, best->to, 0, hashf);
        recordHash(zobristKeyR, data);
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

void Search::setMaxTimeMillsec(const int n) {
    maxTimeMillsec = n;
}

int Search::getMaxTimeMillsec() const {
    return maxTimeMillsec;
}

void Search::sortHashMoves(const int listId, const Hash::_ThashData &phashe) {
    for (int r = 0; r < gen_list[listId].size; r++) {
        _Tmove *mos = &gen_list[listId].moveList[r];

        if (phashe.dataS.from == mos->from && phashe.dataS.to == mos->to) {
            mos->score = _INFINITE / 2;
            return;
        }
    }
}

bool Search::checkInsufficientMaterial(const int nPieces) const {
    //regexp: KN?B*KB*
    if (nPieces > 6) {
        return false;
    }
    // KK
    if (nPieces == 2) {
        return true;
    }
    if (!chessboard.bit[PAWN_BLACK] && !chessboard.bit[PAWN_WHITE] && !chessboard.bit[ROOK_BLACK] && !chessboard.bit[ROOK_WHITE] && !chessboard.bit[QUEEN_WHITE] && !chessboard.bit[QUEEN_BLACK]) {
        u64 allBishop = chessboard.bit[BISHOP_BLACK] | chessboard.bit[BISHOP_WHITE];
        u64 allKnight = chessboard.bit[KNIGHT_BLACK] | chessboard.bit[KNIGHT_WHITE];
        if (allBishop || allKnight) {
            //insufficient material to mate
            if (!allKnight) {
                //regexp: KB+KB*
                if ((bitCount(allBishop) == 1) || ((allBishop & BLACK_SQUARES) == allBishop || (allBishop & WHITE_SQUARES) == allBishop)) {
                    return true;
                }
            } else {
                //KNKN*
                if (!allBishop && bitCount(allKnight) < 3) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Search::checkDraw(const u64 key) const {
    int o = 0;
    for (int i = repetitionMapCount - 1; i >= 0; i--) {
        if (repetitionMap[i] == 0) {
            return false;
        }

        //Threefold repetition
        if (repetitionMap[i] == key && ++o > 2) {
            return true;
        }
    }
    return false;
}

bool Search::getGtbAvailable() const {
    return gtb;
}

bool Search::getSYZYGYAvailable() const {
    return syzygy;
}

int Search::getSYZYGYdtm(const int side) {
    if (!syzygy)return -1;
    return syzygy->getDtm(chessboard, side);
}

string Search::getSYZYGYbestmove(const int side) {
    if (!syzygy)return "";
    return syzygy->getBestmove(chessboard, side);
}

GTB &Search::getGtb() const {
    ASSERT(gtb);
    return *gtb;
}

void Search::deleteGtb() {
    gtb = nullptr;
}

void Search::setMainParam(const bool smp, const int depth) {
    memset(&pvLine, 0, sizeof(_TpvLine));
    mainDepth = depth;
    mainSmp = smp;
}

template<bool smp>
int Search::search(const int depth, const int alpha, const int beta) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    return getSide() ? search<WHITE, smp>(depth, alpha, beta, &pvLine, bitCount(getBitmap<WHITE>() | getBitmap<BLACK>()), &mainMateIn) : search<BLACK, smp>(depth, alpha, beta, &pvLine, bitCount(getBitmap<WHITE>() | getBitmap<BLACK>()), &mainMateIn);
}

int Search::getDtm(const int side, _TpvLine *pline, const int depth, const int nPieces) {
    int mateIn = INT_MAX;
    // gtb
    if (gtb && pline->cmove && maxTimeMillsec > 1000 && gtb->isInstalledPieces(nPieces) && depth >= gtb->getProbeDepth()) {
        int v = gtb->getDtm(side, false, chessboard, (uchar) chessboard.bit[RIGHT_CASTLE_IDX], depth);
        if (abs(v) != INT_MAX) {
            mateIn = v;
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
//            cout << side << " " << (*mateIn) << " " << res << "\n";
            return res;
        }
    }
    // syzygy
    if (syzygy) {
        syzygy->getDtm(chessboard, side);
    }
    return mateIn;
}

template<int side, bool smp>
int Search::search(int depth, int alpha, const int beta, _TpvLine *pline, const int nPieces, int *mateIn) {
    ASSERT_RANGE(depth, 0, MAX_PLY);
    INC(cumulativeMovesCount);

    ASSERT_RANGE(side, 0, 1);
    if (!getRunning()) {
        return 0;
    }
    int score = -_INFINITE;
    //Endgame
    *mateIn = getDtm(side, pline, depth, nPieces);
    if (*mateIn != INT_MAX)return *mateIn;

    u64 oldKey = chessboard.bit[ZOBRISTKEY_IDX];
#ifdef DEBUG_MODE
    double betaEfficiencyCount = 0.0;
#endif
    ASSERT(chessboard.bit[KING_BLACK]);
    ASSERT(chessboard.bit[KING_WHITE]);
    ASSERT(chessboard.bit[KING_BLACK + side]);
    int extension = 0;
    u64 friends = getBitmap<side>();
    u64 enemies = getBitmap<side ^ 1>();
    u64 allpieces = friends | enemies;

    int isIncheckSide = inCheck<side>(allpieces);
    if (!isIncheckSide && depth != mainDepth) {
        if (checkInsufficientMaterial(nPieces) || checkDraw(chessboard.bit[ZOBRISTKEY_IDX])) {
            if (inCheck<side ^ 1>(allpieces)) {
                return _INFINITE - (mainDepth - depth + 1);
            }
            return -lazyEval<side>() * 2;
        }
    }
    if (isIncheckSide) {
        extension++;
    }
    depth += extension;
    if (depth == 0) {
        return quiescence<side, smp>(alpha, beta, -1, nPieces, 0);
    }

    //************* hash ****************
    u64 zobristKeyR = chessboard.bit[ZOBRISTKEY_IDX] ^_random::RANDSIDE[side];

    _TcheckHash checkHashStruct;
    int r;
    if ((r = checkHash<Hash::HASH_GREATER, false>(alpha, beta, depth, zobristKeyR, checkHashStruct)) != INT_MAX) {
        return r;
    };
    if ((r = checkHash<Hash::HASH_ALWAYS, false>(alpha, beta, depth, zobristKeyR, checkHashStruct)) != INT_MAX) {
        return r;
    };
    ///********** end hash ***************

    if (!(numMoves & 1023)) {
        setRunning(checkTime());
    }
    ++numMoves;
    ///********* null move ***********

    int nPiecesSide;
    _TpvLine line;
    line.cmove = 0;

    if (!isIncheckSide && !nullSearch && depth >= NULLMOVE_DEPTH && (nPiecesSide = getNpiecesNoPawnNoKing<side>()) >= NULLMOVES_MIN_PIECE) {
        nullSearch = true;
        int nullScore = -search<side ^ 1, smp>(depth - (NULLMOVES_R1 + (depth > (NULLMOVES_R2 + (nPiecesSide < NULLMOVES_R3 ? NULLMOVES_R4 : 0)))) - 1, -beta, -beta + 1, &line, nPieces, mateIn);
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
    if (depth <= 3 && !isIncheckSide) {
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

    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        score = _INFINITE - (mainDepth - depth + 1);
        return score;
    }
    generateMoves<side>(friends | enemies);
    int listcount = getListSize();
    if (!listcount) {
        --listId;
        if (isIncheckSide) {
            return -_INFINITE + (mainDepth - depth + 1);
        } else {
            return -lazyEval<side>() * 2;
        }
    }
    ASSERT(gen_list[listId].size > 0);
    _Tmove *best = &gen_list[listId].moveList[0];
    if (checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.flags & 0x3 /*&& checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.from != checkHashStruct.phasheType[Hash::HASH_GREATER].dataS.to*/) {// hashfEXACT or hashfBETA
        sortHashMoves(listId, checkHashStruct.phasheType[Hash::HASH_GREATER]);
    } else if (checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.flags & 0x3 /* && checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.from != checkHashStruct.phasheType[Hash::HASH_ALWAYS].dataS.to*/) {// hashfEXACT or hashfBETA
        sortHashMoves(listId, checkHashStruct.phasheType[Hash::HASH_ALWAYS]);
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
            takeback<true>(move, oldKey);
            continue;
        }
        checkInCheck = true;
        if (futilPrune && ((move->type & 0x3) != PROMOTION_MOVE_MASK) && futilScore + PIECES_VALUE[move->capturedPiece] <= alpha && !inCheck<side>()) {
            INC(nCutFp);
            takeback<true>(move, oldKey);
            continue;
        }
        //Late Move Reduction
        int val = INT_MAX;
        if (countMove > 4 && !isIncheckSide && depth >= 3 && move->capturedPiece == SQUARE_FREE && move->promotionPiece == NO_PROMOTION) {
            currentPly++;
            val = -search<side ^ 1, smp>(depth - 2, -(alpha + 1), -alpha, &line, nPieces, mateIn);
            ASSERT(val != INT_MAX);
            currentPly--;
        }
        if (val > alpha) {
            int doMws = (score > -_INFINITE + MAX_PLY);
            int lwb = max(alpha, score);
            int upb = (doMws ? (lwb + 1) : beta);
            currentPly++;
            val = -search<side ^ 1, smp>(depth - 1, -upb, -lwb, &line, move->capturedPiece == SQUARE_FREE ? nPieces : nPieces - 1, mateIn);
            ASSERT(val != INT_MAX);
            currentPly--;
            if (doMws && (lwb < val) && (val < beta)) {
                currentPly++;
                val = -search<side ^ 1, smp>(depth - 1, -beta, -val + 1, &line, move->capturedPiece == SQUARE_FREE ? nPieces : nPieces - 1, mateIn);
                currentPly--;
            }
        }
        score = max(score, val);
        takeback<true>(move, oldKey);
        move->score = score;
        if (score > alpha) {
            if (score >= beta) {
                decListId();
                ASSERT(move->score == score);
                INC(nCutAB);
                ADD(betaEfficiency, betaEfficiencyCount / (double) listcount * 100.0);

                if (getRunning()) {
                    _ThashData data(score, depth - extension, move->from, move->to, 0, Hash::hashfBETA);
                    recordHash(zobristKeyR, data);
                }
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
    if (getRunning()) {
        _ThashData data(score, depth - extension, best->from, best->to, 0, hashf);
        recordHash(zobristKeyR, data);
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

void Search::setChessboard(const _Tchessboard &b) {
    memcpy(&chessboard, &b, sizeof(_Tchessboard));
}

u64 Search::getZobristKey() const {
    return chessboard.bit[ZOBRISTKEY_IDX];
}

int Search::getMateIn() {
    return mainMateIn;
}

void Search::setGtb(GTB &tablebase) {
    gtb = &tablebase;
}

void Search::setSYZYGY(SYZYGY &tablebase) {
    syzygy = &tablebase;
}

bool Search::setParameter(String param, const int value) {
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
    } else if (param == "NO_PAWNS") {
        NO_PAWNS = value;
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
    } else if (param == "BISHOP_NEAR_KING") {
        BISHOP_NEAR_KING = value;
    } else if (param == "HALF_OPEN_FILE_Q") {
        HALF_OPEN_FILE_Q = value;
    } else if (param == "KNIGHT_TRAPPED") {
        KNIGHT_TRAPPED = value;
    } else if (param == "OPEN_FILE") {
        OPEN_FILE = value;
    } else if (param == "OPEN_FILE_Q") {
        OPEN_FILE_Q = value;
    } else if (param == "PAWN_7H") {
        PAWN_7H = value;
    } else if (param == "PAWN_CENTER") {
        PAWN_CENTER = value;
    } else if (param == "PAWN_IN_RACE") {
        PAWN_IN_RACE = value;
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
    } else if (param == "UNDEVELOPED") {
        UNDEVELOPED = value;
    } else if (param == "UNDEVELOPED_BISHOP") {
        UNDEVELOPED_BISHOP = value;
    } else if (param == "VAL_WINDOW") {
        VAL_WINDOW = value;
    } else if (param == "UNPROTECTED_PAWNS") {
        UNPROTECTED_PAWNS = value;
    } else if (param == "ENEMIES_PAWNS_ALL") {
        ENEMIES_PAWNS_ALL = value;
    } else if (param == "NULLMOVE_DEPTH") {
        NULLMOVE_DEPTH = value;
    } else if (param == "NULLMOVES_MIN_PIECE") {
        NULLMOVES_MIN_PIECE = value;
    } else if (param == "NULLMOVES_R1") {
        NULLMOVES_R1 = value;
    } else if (param == "NULLMOVES_R2") {
        NULLMOVES_R2 = value;
    } else if (param == "NULLMOVES_R3") {
        NULLMOVES_R3 = value;
    } else if (param == "NULLMOVES_R4") {
        NULLMOVES_R4 = value;
    } else {
        res = false;
    }
    return res;
#else
    cout << param << " " << value << "\n";
    return false;
#endif
}


