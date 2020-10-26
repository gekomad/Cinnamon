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
    setId(-1);
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
    bestmove.clear();
    INC(checkSmp2);
    int timeTaken = 0;
    searchManager.setRunning(2);
    searchManager.setRunningThread(true);

    //openbook
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

    //Tablebase

    string tb = searchManager.probeRootTB();
    if (!tb.empty()) {
        debug("info string returned move from TB\n");
        _Tmove move;
        searchManager.getMoveFromSan(tb, &move);
        searchManager.makemove(&move);
        cout << "bestmove " << tb << endl;
        ADD(checkSmp2, -1);
        ASSERT(!checkSmp2);
        LOCK_RELEASE(running);
        return;
    }

    unsigned totMoves;

    int mply = 0;

    searchManager.startClock();
    searchManager.clearHeuristic();
    auto hash = Hash::getInstance();
    hash.clearAge();
    searchManager.setForceCheck(false);

    auto start1 = std::chrono::high_resolution_clock::now();
    bool inMate = false;
    int extension = 0;
    string ponderMove;
    searchManager.init();
    int mateIn = INT_MAX;
    string pvv;
    _Tmove resultMove;

#ifdef DEBUG_MODE
    u64 totMovesPrec = -1;
#endif
    while (searchManager.getRunning(0)) {
        totMoves = 0;
        ++mply;
        searchManager.init();

        auto sc =searchManager.search(mply);

        searchManager.setRunningThread(1);
        searchManager.setRunning(1);
        if (!searchManager.getRes(resultMove, ponderMove, pvv, &mateIn)) {
            debug("IterativeDeeping cmove == 0. Exit");
            break;
        }

        searchManager.incHistoryHeuristic(resultMove.s.from, resultMove.s.to, 0x800);

        auto end1 = std::chrono::high_resolution_clock::now();
        timeTaken = Time::diffTime(end1, start1) + 1;
        totMoves += searchManager.getTotMoves();


        if (sc > _INFINITE - MAX_PLY) {
            sc = 0x7fffffff;
        }
#ifdef DEBUG_MODE
        int totStoreHash = hash.nRecordHashA + hash.nRecordHashB + hash.nRecordHashE + 1;
        int percStoreHashA = hash.nRecordHashA * 100 / totStoreHash;
        int percStoreHashB = hash.nRecordHashB * 100 / totStoreHash;
        int percStoreHashE = hash.nRecordHashE * 100 / totStoreHash;
        int totCutHash = hash.n_cut_hashA + hash.n_cut_hashB + 1;
        int percCutHashA = hash.n_cut_hashA * 100 / totCutHash;
        int percCutHashB = hash.n_cut_hashB * 100 / totCutHash;
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

        int collisions = hash.collisions;
        unsigned readCollisions = hash.readCollisions;
        int nNullMoveCut = hash.cutFailed;
        unsigned totGen = searchManager.getTotGen();
        if (nCutAB) {
            cout << "info string beta efficiency: " << (int) (betaEfficiency / totGen * 10) << "%" << endl;
        }
        if (totMovesPrec != 0xffffffffffffffffULL)
            cout << "info string effective branching factor: " << setiosflags(ios::fixed) << setprecision(2) <<
                 ((double) totMoves / (double) totMovesPrec) << endl;
        totMovesPrec = totMoves;
        cout << "info string millsec: " << timeTaken << "  (" << nps / 1000 << "k nodes per seconds)" << endl;
        cout << "info string alphaBeta cut: " << nCutAB << endl;
        cout << "info string lazy eval cut: " << LazyEvalCuts << endl;
        cout << "info string futility pruning cut: " << nCutFp << endl;
        cout << "info string razor cut: " << nCutRazor << endl;
        cout << "info string null move cut: " << nNullMoveCut << endl;

        cout << "info string hash write collisions : " << collisions * 100 / totStoreHash << "%" << endl;
        cout << "info string hash read collisions : " << readCollisions * 100 / totStoreHash << "%" << endl;
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

            resultMove.s.capturedPiece = searchManager.getPieceAt(resultMove.s.side ^ 1, POW2[resultMove.s.to]);
            bestmove = searchManager.decodeBoardinv(resultMove.s.type, resultMove.s.from, resultMove.s.side);
            if (!(resultMove.s.type & (KING_SIDE_CASTLE_MOVE_MASK | QUEEN_SIDE_CASTLE_MOVE_MASK))) {
                bestmove += searchManager.decodeBoardinv(resultMove.s.type, resultMove.s.to, resultMove.s.side);
                if (resultMove.s.promotionPiece != -1) {
                    bestmove += tolower(FEN_PIECE[(uchar) resultMove.s.promotionPiece]);
                }
            }

            if (sc > _INFINITE - MAX_PLY) {
                cout << "info depth " << mply << " score mate 1";
            } else {
                cout << "info depth " << mply - extension << " score cp " << sc;
            }
            cout << " time " << timeTaken << " nodes " << totMoves;
            if (timeTaken)cout << " nps " << (int) ((double) totMoves / (double) timeTaken * 1000.0);
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

    Times *times = &Times::getInstance();
    times->print();


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
