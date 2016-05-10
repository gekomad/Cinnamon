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

class SearchManager : public Singleton<SearchManager>, public ThreadPool<Search> {
    friend class Singleton<SearchManager>;

public:

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv, int *mateIn);

    ~SearchManager();

    Tablebase &getGtb() const;

    Tablebase &createGtb();

    int loadFen(string fen = "");

    int getPieceAt(const int side, const u64 i) const;

    u64 getTotMoves() const;

    void incKillerHeuristic(const int from, const int to, const int value);

    int getHashSize() const;

    void startClock();

    string boardToFen() const;

    bool setParameter(const String param, const int value);

    void clearKillerHeuristic();

    void setKillerHeuristic(const int from, const int to, const int value);

    void clearAge();

    bool getForceCheck() const;

    u64 getZobristKey(const int id) const;

    void setForceCheck(const bool a);

    void setRunningThread(const bool r);

    void search(const int mply);

    void setRunning(const int i);

    int getRunning(const int i) const;

    void display() const;

    string getFen() const;

    void setHashSize(const int s);

    void setMaxTimeMillsec(const int i);

    void setPonder(const bool i);

    int getSide() const;

    int getScore(const int side, const bool trace) const;

    void clearHash();

    int getMaxTimeMillsec(const int);

    void setNullMove(bool i);

    bool makemove(const _Tmove *i);

    template<bool rep>
    void takeback(const _Tmove *move, const u64 oldkey) {
        for (Search *s:getPool()) {
            s->takeback<rep>(move, oldkey);
        }
    }

    void setSide(const bool i);

    bool getGtbAvailable() const;

    int getMoveFromSan(const String &string, _Tmove *ptr);

    int printDtm() const;

    void setGtb(Tablebase &tablebase);

    void pushStackMove();

    void init();

    void setRepetitionMapCount(const int i);

    void deleteGtb();

    void receiveObserverSearch(const int threadID);

    bool setNthread(int);

//#if defined(DEBUG_MODE) || defined(FULL_TEST)fast_perft
//
//    template<int side>
//    u64 getPin(const u64 allpieces, const u64 friends, const int kingPosition) const {
//         return getThread(0).getPin<side>(allpieces, friends, kingPosition);
//    }
//
//#endif

#ifdef DEBUG_MODE

    unsigned getCumulativeMovesCount() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->cumulativeMovesCount;
        }
        return i;
    }

    unsigned getNCutAB() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->nCutAB;
        }
        return i;
    }

    double getBetaEfficiency() const {
        double i = 0;
        for (Search *s:getPool()) {
            i += s->betaEfficiency;
        }
        return i;
    }

    unsigned getLazyEvalCuts() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->lazyEvalCuts;
        }
        return i;
    }

    unsigned getNCutFp() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->nCutFp;
        }
        return i;
    }

    unsigned getNCutRazor() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->nCutRazor;
        }
        return i;
    }

    unsigned getTotGen() const {
        unsigned i = 0;
        for (Search *s:getPool()) {
            i += s->totGen;
        }
        return i;
    }

    u64 getBitmap(const int n, const int side) const {
        return getPool()[n]->getBitmap(side);
    }

#endif

private:

    SearchManager();

    void lazySMP(const int mply);

    void singleSearch(const int mply);

    int mateIn;
    int valWindow = INT_MAX;
    _TpvLine lineWin;

    Spinlock spinlockSearch;

    void setMainPly(const int r);

    void startThread(const bool smpMode, Search &thread, const int depth);

    void stopAllThread();

#ifdef DEBUG_MODE

    atomic_int checkSmp1;

#endif

};

