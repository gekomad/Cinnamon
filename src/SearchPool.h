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


class SearchPool {
public:
    static const int N_THREAD = 4;

    static SearchPool &getInstance() {
        static SearchPool _instance;
        return _instance;
    }

    bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    int loadFen(string fen);

    template<int side>
    int getPieceAt(int i) {
        return searchPool[0]->getPieceAt<side>(i);
    }

    u64 getTotMoves() {
        u64 i = 0;
        for (Search *s:searchPool) {
            i += s->getTotMoves();
        }
        return i;
    }

    void incKillerHeuristic(int from, int to, int value) {
        for (Search *s:searchPool) {
            s->incKillerHeuristic(from, to, value);
        }
    }

    int getHashSize() {
        return searchPool[0]->getHashSize();
    }

    void startThread(int threadID1, int depth, int alpha, int beta, int *mateIn) {
        searchPool[threadID1]->search(depth, alpha, beta, mateIn, threadID1);
        searchPool[threadID1]->start();
    }

    int getValue(int i) {
        return searchPool[i]->getValue();
    }


    void joinAll() {
        for (Search *s:searchPool) {
            s->join();
        }
    }

    void startClock() {
        for (Search *s:searchPool) {
            s->startClock();
        }
    }

    string boardToFen() {
        return searchPool[0]->boardToFen();
    }

    void clearKillerHeuristic() {
        for (Search *s:searchPool) {
            s->clearKillerHeuristic();
        }
    }


    void clearAge() {
        for (Search *s:searchPool) {
            s->clearAge();
        }
    }

    int getForceCheck() {
        return searchPool[0]->getForceCheck();
    }

    u64 getZobristKey() {
        return searchPool[0]->getZobristKey();
    }


    void setForceCheck(bool a) {
        for (Search *s:searchPool) {
            s->setForceCheck(a);
        }
    }

    void setRunningAllThread(int r) {
        for (Search *s:searchPool) {
            s->setRunningThread(r);
        }
    }

    void resetThread() {
        threadWin = -1;
    }

    void setMainPly(int r) {
        for (Search *s:searchPool) {
            s->setMainPly(r);
        }
    }

    void setRunningAll(int r) {
        for (int i = 0; i < N_THREAD; i++) {
            setRunning(r);
        }
    }


    void setRunning(int i) {
        for (Search *s:searchPool) {
            s->setRunning(i);
        }
    }

    int getRunning(int i) {
        return searchPool[i]->getRunning();
    }

    void join(int i) {
        searchPool[i]->join();
    }

    void display() {
        searchPool[0]->display();
    }

    string getFen() {
        return searchPool[0]->getFen();
    }

    bool setHashSize(int s) {
        return searchPool[0]->setHashSize(s);
    }

    void setMaxTimeMillsec(int i) {
        for (Search *s:searchPool) {
            s->setMaxTimeMillsec(i);
        }
    }

    ~SearchPool();

    void setPonder(bool i) {
        for (Search *s:searchPool) {
            s->setPonder(i);
        }
    }

    int getSide() {
        ASSERT(searchPool[0]->getSide() == searchPool[1]->getSide() == searchPool[2]->getSide() == searchPool[3]->getSide());
        return searchPool[0]->getSide();
    }

    int getScore(int side) {
        ASSERT(searchPool[0]->getScore(side) == searchPool[1]->getScore(side) == searchPool[2]->getScore(side) == searchPool[3]->getScore(side));
        return searchPool[0]->getScore(side);
    }

    void clearHash() {
        searchPool[0]->clearHash();
    }

    int getMaxTimeMillsec() {
        // ASSERT(searchPool[0]->getMaxTimeMillsec() == searchPool[1]->getMaxTimeMillsec() == searchPool[2]->getMaxTimeMillsec() == searchPool[3]->getMaxTimeMillsec());
        return searchPool[0]->getMaxTimeMillsec();
    }

    void setNullMove(bool i) {
        for (Search *s:searchPool) {
            s->setNullMove(i);
        }
    }

    bool makemove(_Tmove *i) {
        bool b = false;
        for (Search *s:searchPool) {
            b = s->makemove(i);
        }
        return b;
    }

    void takeback(_Tmove *move, const u64 oldkey, bool rep) {
        for (Search *s:searchPool) {
            s->takeback(move, oldkey, rep);
        }
    }

    void setSide(bool i) {
        for (Search *s:searchPool) {
            s->setSide(i);
        }
    }

    bool getGtbAvailable() {
        return searchPool[0]->getGtbAvailable();
    }

    int getMoveFromSan(String string, _Tmove *ptr) {
        ASSERT(searchPool[0]->getMoveFromSan(string, ptr) == searchPool[1]->getMoveFromSan(string, ptr) == searchPool[2]->getMoveFromSan(string, ptr) == searchPool[3]->getMoveFromSan(string, ptr));
        return searchPool[0]->getMoveFromSan(string, ptr);
    }

    int printDtm() {
        return searchPool[0]->printDtm();
    }

    void pushStackMove() {
        for (Search *s:searchPool) {
            s->pushStackMove();
        }
    }

    void init() {
        for (Search *s:searchPool) {
            s->init();
        }
    }

    void setRepetitionMapCount(int i) {
        for (Search *s:searchPool) {
            s->setRepetitionMapCount(i);
        }
    }

    void deleteGtb() {
        searchPool[0]->deleteGtb();
    }

    void createGtb() {
        searchPool[0]->createGtb();
    }

private:

    int val;
    int threadWin;
    bool searchPoolObserver = false;
    Search *searchPool[N_THREAD] = {nullptr};
    mutex mutexSearch;

    SearchPool();
};

#endif
