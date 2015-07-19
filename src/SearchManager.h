/*
    Cinnamon is a UCI chess engine
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

#pragma once

#include <mutex>
#include <unistd.h>
#include "Search.h"
#include "util/ThreadPool.h"
#include "util/ObserverSearch.h"
#include "util/String.h"

class SearchManager : public Singleton<SearchManager>, public ThreadPool<Search>, public ObserverSearch {
    friend class Singleton<SearchManager>;

public:

    int PVSplit(int PVSplit, const int depth, int alpha, int beta);

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    ~SearchManager();

    int loadFen(string fen = "");

    int getPieceAt(int side, u64 i);

    u64 getTotMoves();

    void incKillerHeuristic(int from, int to, int value);

    int getHashSize();

    int getValue(int i);

    int getMateIn();

    void startClock();

    string boardToFen();

    void clearKillerHeuristic();

    void clearAge();

    int getForceCheck();

    u64 getZobristKey();

    void setForceCheck(bool a);

    void setRunningAllThread(bool r);

    void parallelSearch(int mply);

    void setRunningAll(int r);

    void setRunning(int i);

    int getRunning(int i);

    void join(int i);

    void display();

    string getFen();

    bool setHashSize(int s);

    void setMaxTimeMillsec(int i);

    void setPonder(bool i);

    int getSide();

    int getScore(int side);

    void clearHash();

    int getMaxTimeMillsec();

    void setNullMove(bool i);

    bool makemove(_Tmove *i);

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setSide(bool i);

    bool getGtbAvailable();

    int getMoveFromSan(String string, _Tmove *ptr);

    int printDtm();

    void setGtb(Tablebase &tablebase);

    void pushStackMove();

    void init();

    void setRepetitionMapCount(int i);

    void deleteGtb();

    void receiveObserverPVSplit(int threadID, int value);

    void receiveObserverSearch(int threadID);

    bool setThread(int);

#ifdef DEBUG_MODE

    unsigned getCumulativeMovesCount() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->cumulativeMovesCount;
        }
        return i;
    }

    unsigned getNCutAB() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->nCutAB;
        }
        return i;
    }

    double getBetaEfficiency() {
        double i = 0;
        for (Search *s:searchPool) {
            i += s->betaEfficiency;
        }
        return i;
    }

    unsigned getLazyEvalCuts() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->lazyEvalCuts;
        }
        return i;
    }

    unsigned getNCutFp() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->nCutFp;
        }
        return i;
    }

    unsigned getNCutRazor() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->nCutRazor;
        }
        return i;
    }

    unsigned getNNullMoveCut() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->nNullMoveCut;
        }
        return i;
    }

    unsigned getTotGen() {
        unsigned i = 0;
        for (Search *s:searchPool) {
            i += s->totGen;
        }
        return i;
    }

#endif

private:
    SearchManager();

    void updateAB(int depth, int side, int bound);

    void getWindowRange(int, const int val, int *from, int *to);

    int valWindow;
    int threadWin;
    mutex mutexSearch;

//    mutex mutex1;
    void joinAll();

    void setMainPly(int r);

    void startThread(Search &thread, int depth);

    typedef struct {
        u64 oldKey;
        _Tmove *move;
    } _RollbackValue;
    vector<_RollbackValue *> rollbackValue;

    int alphaValue[MAX_PLY];
    int betaValue[MAX_PLY];

    void registerThreads();
};

