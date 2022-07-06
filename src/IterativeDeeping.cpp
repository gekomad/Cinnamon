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

IterativeDeeping::IterativeDeeping() : maxDepth(MAX_PLY), running(false), ponderEnabled(false) {
    setId(-1);
    plyFromRoot = 0;
    SET(checkSmp2, 0);
}

void IterativeDeeping::setMaxDepth(const int d) {
    maxDepth = min(d, MAX_PLY);
}

IterativeDeeping::~IterativeDeeping() {
}

void IterativeDeeping::enablePonder(const bool b) {
    ponderEnabled = b;
}

bool IterativeDeeping::getPonderEnabled() const {
    return ponderEnabled;
}

void IterativeDeeping::run() {

    if (LOCK_TEST_AND_SET(running)) {
        while (running);
    }
    bestmove.clear();
    INC(checkSmp2);
    int timeTaken;
    searchManager.setRunning(2);
    searchManager.setRunningThread(true);

#ifndef JS_MODE
    //Tablebase

    string tb = TB::probeRootTB1(searchManager.getSearch());
    if (!tb.empty()) {
        debug("info string returned move from TB\n")
        _Tmove move;
        searchManager.getMoveFromSan(tb, &move);
        searchManager.makemove(&move);
        cout << "bestmove " << tb << endl;
        ADD(checkSmp2, -1);
        assert(!checkSmp2);
        LOCK_RELEASE(running);
        return;
    }
#endif
    unsigned totMoves;

    int iter_depth = 0;

    searchManager.startClock();
    searchManager.clearHeuristic();
    plyFromRoot++;
    searchManager.setForceCheck(false);

    auto start1 = std::chrono::high_resolution_clock::now();
    bool inMate = false;
    int extension = 0;
    string ponderMove;
    searchManager.init();

    string pvv;
    _Tmove resultMove;

    DEBUG(u64 totMovesPrec = -1)

    while (searchManager.getRunning(0)) {
        totMoves = 0;
        ++iter_depth;
        searchManager.init();

        auto sc = searchManager.search(plyFromRoot, iter_depth);

        searchManager.setRunningThread(1);
        searchManager.setRunning(1);
        if (!searchManager.getRes(resultMove, ponderMove, pvv)) {
            debug("IterativeDeeping cmove == 0. Exit")
            break;
        }

        searchManager.incHistoryHeuristic(resultMove.from, resultMove.to, 0x1000);

        auto end1 = std::chrono::high_resolution_clock::now();
        timeTaken = Time::diffTime(end1, start1) + 1;
        totMoves += searchManager.getTotMoves();

#ifndef NDEBUG
        const int totStoreHash = hash.nRecordHashA + hash.nRecordHashB + hash.nRecordHashE + 1;
        const int percStoreHashA = hash.nRecordHashA * 100 / totStoreHash;
        const int percStoreHashB = hash.nRecordHashB * 100 / totStoreHash;
        const int percStoreHashE = hash.nRecordHashE * 100 / totStoreHash;
        const int totCutHash = hash.n_cut_hashA + hash.n_cut_hashB + hash.n_cut_hashE + 1;
        const int percCutHashA = hash.n_cut_hashA * 100 / totCutHash;
        const int percCutHashB = hash.n_cut_hashB * 100 / totCutHash;
        const int percCutHashE = hash.n_cut_hashE * 100 / totCutHash;

        const unsigned cumulativeMovesCount = searchManager.getCumulativeMovesCount();

        u64 nps = 0;
        if (timeTaken) {
            nps = totMoves * 1000 / timeTaken;
        }
        const int nCutAB = searchManager.getNCutAB();

        const int LazyEvalCuts = searchManager.getLazyEvalCuts();
        const int nCutFp = searchManager.getNCutFp();
        const int nCutRazor = searchManager.getNCutRazor();
        const int nBadCaputure = searchManager.getTotBadCaputure();
        const int nullMoveCut = searchManager.getNullMoveCut();

        cout << "\ninfo string ply: " << iter_depth << endl;
        cout << "info string tot moves: " << totMoves << endl;

        if (nCutAB) cout << "info string beta efficiency: " << (searchManager.getBetaEfficiency()) << "%" << endl;

        if (totMovesPrec != 0xffffffffffffffffULL)
            cout << "info string effective branching factor: " << setiosflags(ios::fixed) << setprecision(2) <<
                 ((double) totMoves / (double) totMovesPrec) << endl;
        totMovesPrec = totMoves;
        cout << "info string millsec: " << timeTaken << "  (" << nps / 1000 << "k nodes per seconds)" << endl;
        cout << "info string alphaBeta cut: " << nCutAB << endl;
        cout << "info string lazy eval cut: " << LazyEvalCuts << endl;
        cout << "info string futility pruning cut: " << nCutFp << endl;
        cout << "info string null move cut: " << nullMoveCut << endl;
        cout << "info string razor cut: " << nCutRazor << endl;
        cout << "info string bad caputure cut: " << nBadCaputure << endl;
        printf("info string hash stored %d%% (alpha=%d%% beta=%d%% exact=%d%%)\n",
               totStoreHash * 100 / (1 + cumulativeMovesCount), percStoreHashA, percStoreHashB, percStoreHashE);

        printf("info string hash cut %d%% (alpha=%d%% beta=%d%% exact=%d%%)\n",
               totCutHash * 100 / (1 + searchManager.getCumulativeMovesCount()), percCutHashA, percCutHashB,
               percCutHashE);
        printf("info string hash write collisions: %d%%\n", hash.collisions * 100 / (totStoreHash + 1));
        printf("info string hash read collisions: %d%%\n", hash.readCollisions * 100 / (hash.readHashCount + 1));

#endif

        bool trace = true;
        if (abs(sc) > _INFINITE - MAX_PLY) {
            const bool b = searchManager.getForceCheck();
            const u64 oldKey = searchManager.getZobristKey(0);
            const int oldEnpassant = searchManager.getEnpassant(0);
            searchManager.setForceCheck(true);
            const bool valid = searchManager.makemove(&resultMove);
            if (!valid) {
                extension++;
                trace = false;
            }
            searchManager.takeback(&resultMove, oldKey, oldEnpassant, true);
            searchManager.setForceCheck(b);
        }
        if (trace) {

            resultMove.capturedPiece = searchManager.getPieceAt(X(resultMove.side), POW2(resultMove.to));
            bestmove = searchManager.decodeBoardinv(&resultMove, resultMove.side);

            if (sc > _INFINITE - MAX_PLY)
                cout << "info depth " << iter_depth << " score mate " << max(1, (_INFINITE - sc) / 2);
            else if (sc < -_INFINITE + MAX_PLY)
                cout << "info depth " << iter_depth << " score mate -" << max(1, (_INFINITE + sc) / 2);
            else cout << "info depth " << iter_depth - extension << " score cp " << sc;

            cout << " time " << timeTaken << " nodes " << totMoves;
            if (timeTaken)cout << " nps " << (int) ((double) totMoves / (double) timeTaken * 1000.0);
            cout << " pv " << pvv << endl;
            DEBUG(GenMoves::verifyPV(searchManager.getSearch().getFen(),pvv))
        }

        if (searchManager.getForceCheck()) {
            searchManager.setForceCheck(inMate);
            searchManager.setRunning(1);
        } else if (iter_depth == 1 && abs(sc) > _INFINITE - MAX_PLY) {
            searchManager.setForceCheck(true);
            searchManager.setRunning(2);

        }
        if (iter_depth >= maxDepth + extension && (searchManager.getRunning(0) != 2 || inMate)) {
            break;
        }

        if (abs(sc) > _INFINITE - MAX_PLY) {
            inMate = true;
        }
    }

    BENCH_PRINT()

    if (bestmove.empty())cout << "bestmove (none)";
    else cout << "bestmove " << bestmove;
    if (ponderEnabled && ponderMove.size()) cout << " ponder " << ponderMove;

    cout << endl;
    ADD(checkSmp2, -1);
    assert(!checkSmp2);
    LOCK_RELEASE(running);
}

int IterativeDeeping::loadFen(const string &fen) {
    return searchManager.loadFen(fen);
}

