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

#include "IterativeDeeping.h"

IterativeDeeping::IterativeDeeping() : maxDepth(MAX_PLY), running(false), openBook(nullptr), ponderEnabled(false) {
    setUseBook(false);
    SET(checkSmp2, 0);
}

void IterativeDeeping::setMaxDepth(const int d) {
    maxDepth = min(d, _board::MAX_PLY);
}

bool IterativeDeeping::getGtbAvailable() const {
    return searchManager.getGtbAvailable();
}

IterativeDeeping::~IterativeDeeping() {
}

void IterativeDeeping::enablePonder(const bool b) {
    ponderEnabled = b;
}

bool IterativeDeeping::getPonderEnabled() const {
    return ponderEnabled;
}

bool IterativeDeeping::getUseBook() const {
    return openBook;
}

void IterativeDeeping::loadBook(const string f) {
    openBook = OpenBook::getInstance(f);
}

void IterativeDeeping::setUseBook(const bool b) {
    if (!openBook && b) {
        openBook = OpenBook::getInstance("cinnamon.bin");
        return;
    }
    if (!b && openBook) {
        openBook->dispose();
        openBook = nullptr;
    }
}

string IterativeDeeping::go() {
    run();
    return getBestmove();
}

void IterativeDeeping::run() {

    if (LOCK_TEST_AND_SET(running)) {
        while (running);
    }

    INC(checkSmp2);
    int timeTaken = 0;
    searchManager.setRunning(2);
    searchManager.setRunningThread(true);
    int mply = 0;

    string b = getSYZYGYbestmove(searchManager.getSide());
    if (!b.empty()) {
        cout << "bestmove " << b;
        cout << endl;
        ADD(checkSmp2, -1);
        ASSERT(!checkSmp2);
        LOCK_RELEASE(running);
        return;
    }
    if (openBook) {
        ASSERT(openBook);
        string obMove = openBook->search(searchManager.boardToFen());
        if (!obMove.empty()) {
            _Tmove move;
            searchManager.getMoveFromSan(obMove, &move);
            searchManager.makemove(&move);
            cout << "bestmove " << obMove << endl;
            ADD(checkSmp2, -1);
            ASSERT(!checkSmp2);
            LOCK_RELEASE(running);
            return;
        }
    }
    int sc = 0;
    unsigned totMoves;

    mply = 0;

    searchManager.startClock();
    searchManager.clearKillerHeuristic();
    searchManager.clearAge();
    searchManager.setForceCheck(false);

    auto time = Time();
    time.start();
    bool inMate = false;
    int extension = 0;
    string ponderMove;
    searchManager.init();
    int mateIn = INT_MAX;
    string pvv;
    _Tmove resultMove;
    int nodesPerSecond = 100000;
#ifdef DEBUG_MODE
    u64 totMovesPrec = -1;
#endif
    while (searchManager.getRunning(0)) {
        totMoves = 0;
        ++mply;
        searchManager.init();

        searchManager.search(mply, nodesPerSecond);

        searchManager.setRunningThread(1);
        searchManager.setRunning(1);
        if (!searchManager.getRes(resultMove, ponderMove, pvv, &mateIn)) {
            debug("IterativeDeeping cmove == 0, exit");
            break;
        }

        searchManager.incKillerHeuristic(resultMove.from, resultMove.to, 0x800);

        time.stop();
        timeTaken = time.getMill();
        totMoves += searchManager.getTotMoves();
        if(timeTaken)nodesPerSecond = (totMoves * 1000) / timeTaken;

        sc = resultMove.score;
        if (resultMove.score > _INFINITE - MAX_PLY) {
            sc = 0x7fffffff;
        }
#ifdef DEBUG_MODE
        int totStoreHash = searchManager.getPool()[0]->nRecordHashA + searchManager.getPool()[0]->nRecordHashB +
            searchManager.getPool()[0]->nRecordHashE + 1;
        int percStoreHashA = searchManager.getPool()[0]->nRecordHashA * 100 / totStoreHash;
        int percStoreHashB = searchManager.getPool()[0]->nRecordHashB * 100 / totStoreHash;
        int percStoreHashE = searchManager.getPool()[0]->nRecordHashE * 100 / totStoreHash;
        int totCutHash = searchManager.getPool()[0]->n_cut_hashA + searchManager.getPool()[0]->n_cut_hashB + 1;
        int percCutHashA = searchManager.getPool()[0]->n_cut_hashA * 100 / totCutHash;
        int percCutHashB = searchManager.getPool()[0]->n_cut_hashB * 100 / totCutHash;
        cout << "\ninfo string ply: " << mply << endl;
        cout << "info string tot moves: " << totMoves << endl;
        unsigned cumulativeMovesCount = searchManager.getCumulativeMovesCount();
        cout << "info string hash stored " << totStoreHash * 100 / (1 + cumulativeMovesCount) << "% (alpha=" <<
            percStoreHashA << "% beta=" << percStoreHashB << "% exact=" << percStoreHashE << "%)" << endl;

        cout << "info string cut hash " << totCutHash * 100 / (1 + searchManager.getCumulativeMovesCount()) <<
            "% (alpha=" << percCutHashA << "% beta=" << percCutHashB << "%)" << endl;

        u64 nps = 0;
        if (timeTaken) {
            nps = totMoves * 1000 / timeTaken;
        }
        int nCutAB = searchManager.getNCutAB();
        double betaEfficiency = searchManager.getBetaEfficiency();
        int LazyEvalCuts = searchManager.getLazyEvalCuts();
        int nCutFp = searchManager.getNCutFp();
        int nCutRazor = searchManager.getNCutRazor();

        int collisions = searchManager.getPool()[0]->collisions;
        int nNullMoveCut = searchManager.getPool()[0]->cutFailed;
        unsigned totGen = searchManager.getTotGen();
        if (nCutAB) {
            cout << "info string beta efficiency: " << (int) (betaEfficiency / totGen * 10) << "%" << endl;
        }
        if (totMovesPrec != -1)
            cout << "info string effective branching factor: " << setiosflags(ios::fixed) << setprecision(2) <<
                ((double) totMoves / (double) totMovesPrec) << endl;
        totMovesPrec = totMoves;
        cout << "info string millsec: " << timeTaken << "  (" << nps / 1000 << "k nodes per seconds)" << endl;
        cout << "info string alphaBeta cut: " << nCutAB << endl;
        cout << "info string lazy eval cut: " << LazyEvalCuts << endl;
        cout << "info string futility pruning cut: " << nCutFp << endl;
        cout << "info string razor cut: " << nCutRazor << endl;
        cout << "info string null move cut: " << nNullMoveCut << endl;

        cout << "info string hash collisions : " << collisions * 100 / totStoreHash << "%" << endl;
#endif
        ///is a valid move?
        bool trace = true;
        if (abs(sc) > _INFINITE - MAX_PLY) {
            const bool b = searchManager.getForceCheck();
            const u64 oldKey = searchManager.getZobristKey(0);
            searchManager.setForceCheck(true);
            const bool valid = searchManager.makemove(&resultMove);
            if (!valid) {
                extension++;
                trace = false;
            }
            searchManager.takeback(&resultMove, oldKey, true);
            searchManager.setForceCheck(b);
        }
        if (trace) {

            resultMove.capturedPiece = searchManager.getPieceAt(resultMove.side ^ 1, POW2[resultMove.to]);
            bestmove = Search::decodeBoardinv(resultMove.type, resultMove.from, resultMove.side);
            if (!(resultMove.type & (Search::KING_SIDE_CASTLE_MOVE_MASK | Search::QUEEN_SIDE_CASTLE_MOVE_MASK))) {
                bestmove += Search::decodeBoardinv(resultMove.type, resultMove.to, resultMove.side);
                if (resultMove.promotionPiece != -1) {
                    bestmove += tolower(FEN_PIECE[(uchar) resultMove.promotionPiece]);
                }
            }

            if (abs(sc) > _INFINITE - MAX_PLY) {
                cout << "info score mate 1 depth " << mply;
            } else {
                cout << "info score cp " << sc << " depth " << mply - extension;
            }
            cout << " nodes " << totMoves << " time " << timeTaken;
            if (timeTaken)cout << " knps " << (totMoves / timeTaken);
            cout << " pv " << pvv << endl;
        }

        if (searchManager.getForceCheck()) {
            searchManager.setForceCheck(inMate);
            searchManager.setRunning(1);

        } else if (abs(sc) > _INFINITE - MAX_PLY) {
            searchManager.setForceCheck(true);
            searchManager.setRunning(2);

        }
        if (mply >= maxDepth + extension && (searchManager.getRunning(0) != 2 || inMate)) {
            break;
        }

        if (abs(sc) > _INFINITE - MAX_PLY) {
            inMate = true;
        }
    }

#ifdef BENCH_MODE

    cout << "info string pawnTime eval avg: " << Eval::pawnTime.avgAndReset() << " ns." << endl;
    cout << "info string bishopTime eval avg: " << Eval::bishopTime.avgAndReset() << " ns." << endl;
    cout << "info string knightTime eval avg: " << Eval::knightTime.avgAndReset() << " ns." << endl;
    cout << "info string rookTime eval avg: " << Eval::rookTime.avgAndReset() << " ns." << endl;
    cout << "info string queenTime eval avg: " << Eval::queenTime.avgAndReset() << " ns." << endl;
    cout << "info string kingTime eval avg: " << Eval::kingTime.avgAndReset() << " ns." << endl;
    cout << "info string evalTime TOT avg: " << Eval::evalTime.avgAndReset() << " ns." << endl;

#endif

    cout << "bestmove " << bestmove;
    if (ponderEnabled && ponderMove.size()) {
        cout << " ponder " << ponderMove;
    }

    cout << endl;
    ADD(checkSmp2, -1);
    ASSERT(!checkSmp2);
    LOCK_RELEASE(running);
}

int IterativeDeeping::loadFen(const string fen) {
    return searchManager.loadFen(fen);
}

bool IterativeDeeping::setNthread(const int i) {
    return searchManager.setNthread(i);
}
