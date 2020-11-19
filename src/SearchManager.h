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
#include "namespaces/bits.h"

class SearchManager : public Singleton<SearchManager> {
    friend class Singleton<SearchManager>;

public:

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    ~SearchManager();

    int loadFen(string fen = "");

    int getPieceAt(int side, u64 i);

    u64 getTotMoves();

    void incHistoryHeuristic(int from, int to, int value);

    void startClock();

    string boardToFen();

    string decodeBoardinv(const uchar type, const int a, const int side);

    bool setParameter(String param, int value);

    void clearHeuristic();

    int getForceCheck();

    u64 getZobristKey(int id);

    void setForceCheck(bool a);

    void setRunningThread(bool r);

    string probeRootTB() const;

    void setRunning(int i);

    int getRunning(int i);

    void display();

    string getFen();

    void setMaxTimeMillsec(int i);

    void unsetSearchMoves();

    void setSearchMoves(vector<string> &searchmoves);

    void setPonder(bool i);

    int getSide() const;

    int getScore(int side, const bool trace);

    int getMaxTimeMillsec();

    void setNullMove(bool i);

    void setChess960(bool i);

    bool makemove(_Tmove *i);

    void takeback(_Tmove *move, const u64 oldkey, bool rep);

    void setSide(bool i);

    int getMoveFromSan(String string, _Tmove *ptr) const;

#ifndef JS_MODE

    int printDtmGtb(const bool dtm);

    void printDtmSyzygy();

    void printWdlSyzygy();

#endif

    void pushStackMove();

    void init();

    void setRepetitionMapCount(int i);

    bool setNthread(int);

#if defined(FULL_TEST)

    unsigned SZtbProbeWDL() const;

    u64 getBitmap(const int n, const int side) const {
        return side ? board::getBitmap<WHITE>(threadPool->getPool()[n]->getChessboard())
                    : board::getBitmap<BLACK>(threadPool->getPool()[n]->getChessboard());
    }

    const _Tchessboard &getChessboard(const int n) const {
        return threadPool->getPool()[n]->getChessboard();
    }

    template<int side>
    u64 getPinned(const u64 allpieces, const u64 friends, const int kingPosition) const {
        return board::getPinned<side>(allpieces, friends, kingPosition, threadPool->getPool()[0]->getChessboard());
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

    int search(int mply);

private:

    SearchManager();

    ThreadPool<Search> *threadPool = nullptr;

    _TpvLine lineWin;

    void setMainPly(const int r);

    void startThread(Search &thread, const int depth);

    void stopAllThread();

};

