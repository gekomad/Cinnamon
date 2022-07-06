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

#include "Search.h"
#include "threadPool/ThreadPool.h"
#include "namespaces/String.h"
#include "util/IniFile.h"
#include "db/TB.h"

class SearchManager : private Singleton<SearchManager> {
    friend class Singleton<SearchManager>;

public:

    ~SearchManager();

    static bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    static int loadFen(const string &fen = "");

    static int getPieceAt(const uchar side, const u64 i);

    static u64 getTotMoves();

    static void incHistoryHeuristic(const int from, const int to, const int value);

    static void startClock();

    Search &getSearch(int i = 0) {
        return threadPool->getThread(i);
    }


    static string decodeBoardinv(const _Tmove *, const uchar side);

#ifdef TUNING

    const _Tchessboard &getChessboard() {
        return threadPool->getThread(0).chessboard;
    }

    static void setParameter(const string &param, const int value, const int phase) {
        for (Search *s: threadPool->getPool()) {
            s->setParameter(param, value, phase);
        }
    }

    static int getParameter(const string &param, const int phase) {
        return threadPool->getThread(0).getParameter(param, phase);
    }

    int getQscore() const {
        return threadPool->getThread(0).qSearch(15, -_INFINITE, _INFINITE);
    }

#endif

    static void clearHeuristic();

    static int getForceCheck();

    static u64 getZobristKey(const int id);

    static u64 getEnpassant(const int id) {
        return threadPool->getThread(id).getEnpassant();
    }

    static void setForceCheck(const bool a);

    static void setRunningThread(const bool r);

    static void setRunning(const int i);

    static int getRunning(const int i);

    static void display();

    static void setMaxTimeMillsec(const int i);

    static void unsetSearchMoves();

    static void setSearchMoves(const vector<string> &searchmoves);

    static void setPonder(bool i);

    static int getSide();

    static int getScore(const uchar side);

    static int getMaxTimeMillsec();

    static void setNullMove(const bool i);

    static void setChess960(const bool i);

    static bool makemove(const _Tmove *i);

    static void takeback(const _Tmove *move, const u64 oldkey, const uchar oldEnpassant, const bool rep);

    static void setSide(const bool i);

    static int getMoveFromSan(const string &string, _Tmove *ptr);

#ifndef JS_MODE

    static int printDtmGtb(const bool dtm);

    static void printDtmSyzygy();

    static void printWdlSyzygy();

#endif

    static void pushStackMove();

    static void init();

    static void setRepetitionMapCount(const int i);

    static bool setNthread(const int);

#if defined(FULL_TEST)

    unsigned SZtbProbeWDL() const;

    u64 getBitmap(const int n, const uchar side) const {
        return side ? board::getBitmap<WHITE>(threadPool->getPool()[n]->chessboard)
                    : board::getBitmap<BLACK>(threadPool->getPool()[n]->chessboard);
    }

    const _Tchessboard &getChessboard(const int n = 0) const {
        return threadPool->getPool()[n]->chessboard;
    }

    template<uchar side>
    u64 getPinned(const u64 allpieces, const u64 friends, const int kingPosition) const {
        return board::getPinned<side>(allpieces, friends, kingPosition, threadPool->getPool()[0]->chessboard);
    }

#endif

#ifndef NDEBUG

    static unsigned getCumulativeMovesCount() {
        return Search::cumulativeMovesCount;
    }

    static unsigned getNCutAB() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutAB;
        }
        return i;
    }

    static double getBetaEfficiency() {
        double b = 0;
        unsigned count = 0;
        for (Search *s:threadPool->getPool()) {
            b += s->betaEfficiency;
            count += s->betaEfficiencyCount;
        }
        return b / count;
    }

    static unsigned getLazyEvalCuts() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->getLazyEvalCuts();
        }
        return i;
    }

    static unsigned getNCutFp() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutFp;
        }
        return i;
    }

    static unsigned getNCutRazor() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutRazor;
        }
        return i;
    }

    static unsigned getTotBadCaputure() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nCutBadCaputure;
        }
        return i;
    }

    static unsigned getNullMoveCut() {
        unsigned i = 0;
        for (Search *s:threadPool->getPool()) {
            i += s->nNullMoveCut;
        }
        return i;
    }

#endif

    static int search(const int ply, const int iter_depth);

private:

    SearchManager();

    static ThreadPool<Search> *threadPool;

    static _TpvLine lineWin;

    static void setMainPly(const int ply, const int r);

    static void startThread(Search &thread, const int depth);

    static void stopAllThread();

};

