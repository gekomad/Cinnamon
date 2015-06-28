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

    void getRes(_Tmove &resultMove, string &ponderMove, string &pvv);

    int loadFen(string fen) {
        int i;
        for (Search *s:searchPool) {
            i = s->loadFen(fen);
        }
        return i;
    }

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
        searchPool[threadID1]->search(depth, alpha, beta, &SearchPool::line1[threadID1], mateIn, threadID1);
        searchPool[threadID1]->start();
    }

    int getValue(int i) {
        searchPool[i]->getValue();
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

    void resetPVLine(int i) {
        memset(&line1[i], 0, sizeof(_TpvLine));
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
        searchPool[0]->getFen();
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

    //  int loadFen();


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

//    u64 getBitBoard() {
//        //ASSERT(searchPool[0]->getBitBoard() == searchPool[1]->getBitBoard() == searchPool[2]->getBitBoard() == searchPool[3]->getBitBoard());
//        return side == WHITE ? searchPool[0]->getBitBoard<WHITE>() : searchPool[0]->getBitBoard<BLACK>();
//    }

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

    void makemove(_Tmove *i) {
        for (Search *s:searchPool) {
            s->makemove(i);
        }
    }

    void setSide(bool i) {
        for (Search *s:searchPool) {
            s->setSide(i);
        }
    }

    int getMoveFromSan(String string, _Tmove *ptr) {
        ASSERT(searchPool[0]->getMoveFromSan(string, ptr) == searchPool[1]->getMoveFromSan(string, ptr) == searchPool[2]->getMoveFromSan(string, ptr) == searchPool[3]->getMoveFromSan(string, ptr));
        return searchPool[0]->getMoveFromSan(string, ptr);
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
        assert(0);
        //TODO searchPool.deleteGtb();
    }

    void createGtb() {
        assert(0);
        //TODO  searchPool.createGtb();
    }

private:

    int val;
    atomic<int> threadWin;
    _TpvLine line1[N_THREAD];//TODO metterlo dentro search
    bool searchPoolObserver = false;
    Search *searchPool[N_THREAD] = {nullptr};
    mutex mutexSearch;

    SearchPool();
};

#endif
