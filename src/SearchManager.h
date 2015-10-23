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

#pragma once

#include <mutex>
#include <unistd.h>
#include "Search.h"
#include "threadPool/ThreadPool.h"
#include <condition_variable>
#include "ObserverSearch.h"
#include "util/String.h"
#include <algorithm>
#include <future>
#include "namespaces/def.h"

class SearchManager : public Singleton<SearchManager>, public ThreadPool<Search>, public ObserverSearch {
    friend class Singleton<SearchManager>;

public:
    STATIC_CONST int VAL_WINDOW = 50;

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv, int *mateIn);

    ~SearchManager();

    Tablebase &getGtb();

    Tablebase &createGtb();

    int loadFen(const string &fen = "");

    int getPieceAt(int side, u64 i);

    u64 getTotMoves();

    void incKillerHeuristic(int from, int to, int value);

    int getHashSize();

    int getValue(int i);

    void startClock();

    string boardToFen();

    bool setParameter(String param, int value);

    void clearKillerHeuristic();

    void clearAge();

    int getForceCheck();

    u64 getZobristKey(int id);

    void setForceCheck(bool a);

    void setRunningThread(bool r);

    void search(int mply);


    void setRunning(int i);

    int getRunning(int i);

    void display();

    string getFen();

    void setHashSize(int s);

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

    int getMoveFromSan(const String &string, _Tmove *ptr);

    int printDtm();

    void setGtb(Tablebase &tablebase);

    void pushStackMove();

    void init();

    void setRepetitionMapCount(int i);

    void deleteGtb();

    void receiveObserverSearch(int threadID);

    bool setNthread(int);

#ifdef DEBUG_MODE


    unsigned getCumulativeMovesCount() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->cumulativeMovesCount;
        }
        return i;
    }

    unsigned getNCutAB() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->nCutAB;
        }
        return i;
    }

    double getBetaEfficiency() {
        double i = 0;
        for (Search *s:threadPool) {
            i += s->betaEfficiency;
        }
        return i;
    }

    unsigned getLazyEvalCuts() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->lazyEvalCuts;
        }
        return i;
    }

    unsigned getNCutFp() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->nCutFp;
        }
        return i;
    }

    unsigned getNCutRazor() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->nCutRazor;
        }
        return i;
    }

    unsigned getNNullMoveCut() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->nNullMoveCut;
        }
        return i;
    }

    unsigned getTotGen() {
        unsigned i = 0;
        for (Search *s:threadPool) {
            i += s->totGen;
        }
        return i;
    }

#endif

private:
    static Hash *hash;

    SearchManager();

    void parallelSearch(int mply);

    void singleSearch(int mply);

    int mateIn;
    int valWindow;
    _TpvLine lineWin;
    u64 totCountWin;
    Mutex mutexSearch;

    void setMainPly(int r);

    void startThread(bool smpMode, Search &thread, int depth, int, int);

    int nThreads;

    void stopAllThread();

};

