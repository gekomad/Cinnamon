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

#ifndef SEARCHPOOL_H_
#define SEARCHPOOL_H_

#include <mutex>
#include "Search.h"


class SearchManager : public Singleton<SearchManager> {
    friend class Singleton<SearchManager>;

public:
    int PVSplit(int mply, int alpha, int beta);

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    ~SearchManager();

    int loadFen(string fen);

    int getPieceAt(int side, int i);

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

    void setRunningAllThread(int r);

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

#ifdef DEBUG_MODE

    unsigned getCumulativeMovesCount() {
        return searchPool[0]->cumulativeMovesCount;
    }

    int getNCutAB() {
        return searchPool[0]->nCutAB;
    }

    double getBetaEfficiency() {
        return searchPool[0]->betaEfficiency;
    }

    int getLazyEvalCuts() {
        return searchPool[0]->lazyEvalCuts;
    }

    int getNCutFp() {
        return searchPool[0]->nCutFp;
    }

    int getNCutRazor() {
        return searchPool[0]->nCutRazor;
    }

    int getNNullMoveCut() {
        return searchPool[0]->nNullMoveCut;
    }

    unsigned getTotGen() {
        return searchPool[0]->totGen;
    }

    int getNCutInsufficientMaterial() {
        return searchPool[0]->nCutInsufficientMaterial;
    }


#endif
private:
    SearchManager();

    static const int N_THREAD = 4;
    int val;
    int threadWin;
    bool searchPoolObserver = false;
    // Search *searchPool[N_THREAD] = {nullptr};
    vector<Search*> searchPool;
    mutex mutexSearch;

    void joinAll();

    void setMainPly(int r);


    void startThread(int threadID1, int depth, int alpha, int beta);
};

#endif
