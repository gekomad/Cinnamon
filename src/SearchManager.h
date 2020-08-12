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

#include <unistd.h>
#include "Search.h"
#include "threadPool/ThreadPool.h"
#include <condition_variable>
#include "util/String.h"
#include "util/IniFile.h"
#include <algorithm>
#include <future>
#include "namespaces/def.h"

class SearchManager: public Singleton<SearchManager>
#ifdef FULL_TEST
        , public ThreadPool<Search>
#endif
{
    friend class Singleton<SearchManager>;

public:

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv, int *mateIn);
    static GTB *gtb;

    ~SearchManager();
#ifndef JS_MODE
    static GTB *getGtb() {
        return gtb;
    }

    GTB &createGtb();

#endif

    int loadFen(string fen = "");

    int getPieceAt(int side, u64 i);

    u64 getTotMoves();

    void incHistoryHeuristic(int from, int to, int value);

    int getHashSize();

    void startClock();

    string boardToFen();

    bool setParameter(String param, int value);

    void clearHistoryHeuristic();

    void clearAge();

    int getForceCheck();

    u64 getZobristKey(int id);

    void setForceCheck(bool a);

    void setRunningThread(bool r);

    string probeRootTB();
    void setRunning(int i);

    int getRunning(int i);

    void display();

    string getFen();

    void setHashSize(int s);

    void setMaxTimeMillsec(int i);
    void unsetSearchMoves();
    void setSearchMoves(vector <string> &searchmoves);
    void setPonder(bool i);

    int getSide();

    int getScore(int side, const bool trace);

    void clearHash();

    int getMaxTimeMillsec();

    void setNullMove(bool i);

    bool makemove(_Tmove *i);

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setSide(bool i);

    bool getGtbAvailable() const;

    int getMoveFromSan(String string, _Tmove *ptr);
#ifndef JS_MODE
    void printDtmGtb();

#endif
    void pushStackMove();

    void init();

    void setRepetitionMapCount(int i);

    void deleteGtb();

    bool setNthread(int);

#if defined(FULL_TEST)
    u64 getBitmap(const int n, const int side) const {
        return getPool()[n]->getBitmap(side);
    }

    template<int side>
    u64 getPinned(const u64 allpieces, const u64 friends, const int kingPosition) const {
        return getThread(0).getPinned<side>(allpieces, friends, kingPosition);
    }
#endif

#ifdef DEBUG_MODE

    unsigned getCumulativeMovesCount() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->cumulativeMovesCount;
        }
        return i;
    }

    unsigned getNCutAB() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutAB;
        }
        return i;
    }

    double getBetaEfficiency() {
        double i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->betaEfficiency;
        }
        return i;
    }

    unsigned getLazyEvalCuts() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->lazyEvalCuts;
        }
        return i;
    }

    unsigned getNCutFp() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutFp;
        }
        return i;
    }

    unsigned getNCutRazor() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutRazor;
        }
        return i;
    }

    unsigned getTotGen() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->totGen;
        }
        return i;
    }

#endif
    const Hash *getHash() const {
        return &hash;
    }
    void search(int mply);
private:

    Hash hash;
    SearchManager();
    ThreadPool<Search> *threadPool = nullptr;

    int mateIn;

    _TpvLine lineWin;

    void setMainPly(const int r);

    void startThread(Search &thread, const int depth);

    void stopAllThread();

};

