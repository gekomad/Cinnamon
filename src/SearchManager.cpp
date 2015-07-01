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

#include "SearchManager.h"

//int PVSplit(k, mply, -_INFINITE, _INFINITE, &mateIn[k]) {
//    if (mply<5) {
//        searchManager[???]->search(depth, alpha, beta, mateIn, threadID1);
//        searchManager[???]->start();
//        return searchManager[??]->search();
//    }
//
//    succnode = GetFirstSucc(curnode );
//    score = PVSplit(curnode, alpha, beta );
//    if(score > beta) {
//        return beta;
//    }
//    if (score > alpha) {
//        alpha = score;
//    }
//    while (HasMoreSuccessors(cur node )) {
//        succ
//        node = GetNextSucc(cur
//        node );
//        score = AlphaBeta(succ        node, alpha, beta );
//        if (score > beta) {
//            return beta;
//        }
//        if (score > alpha) {
//            alpha = score;
//        }
//    }
//
//    return alpha;
//}

//SearchManager::~SearchManager() {
//    for (int i = 0; i < N_THREAD; i++) {
//        delete searchManager[i];
//        searchManager[i] = nullptr;
//    }
//}

int SearchManager::loadFen(string fen) {
    int res = searchPool[0]->loadFen(fen);
    ASSERT_RANGE(res, 0, 1)
    for (int i = 1; i < N_THREAD; i++) {
        searchPool[i]->setChessboard(searchPool[0]->getChessboard());
    }
    return res;
}

bool SearchManager::getRes(_Tmove &resultMove, string &ponderMove, string &pvv) {
    if (threadWin == -1) {
        return false;
    }
    pvv.clear();
    string pvvTmp;
    _TpvLine &line1 = searchPool[threadWin]->getPvLine();
    ASSERT(line1.cmove);
    for (int t = 0; t < line1.cmove; t++) {
        pvvTmp.clear();
        pvvTmp += Search::decodeBoardinv(line1.argmove[t].type, line1.argmove[t].from, searchPool[threadWin]->getSide());
        if (pvvTmp.length() != 4) {
            pvvTmp += Search::decodeBoardinv(line1.argmove[t].type, line1.argmove[t].to, searchPool[threadWin]->getSide());
        }
        pvv.append(pvvTmp);
        if (t == 1) {
            ponderMove.assign(pvvTmp);
        }
        pvv.append(" ");
    };
    memcpy(&resultMove, line1.argmove, sizeof(_Tmove));
    return true;
}

SearchManager::SearchManager() {
    for (int i = 0; i < N_THREAD; i++) {
        searchPool[i] = new Search();
    }
    threadWin = -1;
    if (!searchPoolObserver) {
        searchPoolObserver = true;
        searchPool[0]->registerObservers([this]() {
            if (getRunning(0)) {
                lock_guard<mutex> lock(mutexSearch);
                if (threadWin == -1) {
                    int t = searchPool[0]->getValue();
                    if (t > val - VAL_WINDOW && t < val + VAL_WINDOW) {
                        val = t;
                        threadWin = 0;
                        ASSERT(searchPool[threadWin]->getPvLine().cmove);
                        for (Search *s:searchPool) {
                            s->setRunningThread(0);
                        }
                    }
                }
            }
        });
        searchPool[1]->registerObservers([this]() {
            if (getRunning(1)) {
                lock_guard<mutex> lock(mutexSearch);
                if (threadWin == -1) {

                    int t = searchPool[1]->getValue();
                    if (t > val - VAL_WINDOW * 2 && t < val + VAL_WINDOW * 2) {
                        val = t;
                        threadWin = 1;
                        ASSERT(searchPool[threadWin]->getPvLine().cmove);
                        for (Search *s:searchPool) {
                            s->setRunningThread(0);
                        }
                    }
                }
            }
        });
        searchPool[2]->registerObservers([this]() {
            if (getRunning(2)) {
                lock_guard<mutex> lock(mutexSearch);
                if (threadWin == -1) {

                    int t = searchPool[2]->getValue();
                    if (t > val - VAL_WINDOW * 4 && t < val + VAL_WINDOW * 4) {
                        val = t;
                        threadWin = 2;
                        ASSERT(searchPool[threadWin]->getPvLine().cmove);
                        for (Search *s:searchPool) {
                            s->setRunningThread(0);
                        }
                    }
                }
            }
        });
        searchPool[3]->registerObservers([this]() {
            if (getRunning(3)) {
                lock_guard<mutex> lock(mutexSearch);
                if (threadWin == -1) {
                    val = searchPool[3]->getValue();
                    threadWin = 3;
                    ASSERT(searchPool[threadWin]->getPvLine().cmove);
                    for (Search *s:searchPool) {
                        s->setRunningThread(0);
                    }
                }
            }
        });
    }
}

void SearchManager::parallelSearch(int mply) {
    threadWin = -1;
    setMainPly(mply);
    if (mply == 1) {
        int k = 3;
        setRunning(2);
        startThread(k, mply, -_INFINITE, _INFINITE);
        join(k);
        val = getValue(k);

    } else {
        for (int k = 0; k < 3; k++) {
            if (getRunning(k)) {
                int window = VAL_WINDOW * pow(2, k - 1);
                startThread(k, mply, val - window, val + window);
            }
        }

        int k = 3;

        if (getRunning(k)) {
            startThread(k, mply, -_INFINITE, _INFINITE);
        }

        joinAll();
    }
}

void SearchManager::startThread(int threadID1, int depth, int alpha, int beta) {
    searchPool[threadID1]->search(depth, alpha, beta, threadID1);
    searchPool[threadID1]->start();
}

void SearchManager::joinAll() {
    for (Search *s:searchPool) {
        s->join();
    }
}

void SearchManager::setMainPly(int r) {
    for (Search *s:searchPool) {
        s->setMainPly(r);
    }
}

int SearchManager::getPieceAt(int side, int i) {
    return side == WHITE ? searchPool[0]->getPieceAt<WHITE>(i) : searchPool[0]->getPieceAt<BLACK>(i);
}

u64 SearchManager::getTotMoves() {
    u64 i = 0;
    for (Search *s:searchPool) {
        i += s->getTotMoves();
    }
    return i;
}

void SearchManager::incKillerHeuristic(int from, int to, int value) {
    for (Search *s:searchPool) {
        s->incKillerHeuristic(from, to, value);
    }
}

int SearchManager::getHashSize() {
    return searchPool[0]->getHashSize();
}

int SearchManager::getValue(int i) {
    return searchPool[i]->getValue();
}

int SearchManager::getMateIn() {
    return searchPool[threadWin]->getMateIn();
}


void SearchManager::startClock() {
    for (Search *s:searchPool) {
        s->startClock();
    }
}

string SearchManager::boardToFen() {
    return searchPool[0]->boardToFen();
}

void SearchManager::clearKillerHeuristic() {
    for (Search *s:searchPool) {
        s->clearKillerHeuristic();
    }
}

void SearchManager::clearAge() {
    for (Search *s:searchPool) {
        s->clearAge();
    }
}

int SearchManager::getForceCheck() {
    return searchPool[0]->getForceCheck();
}

u64 SearchManager::getZobristKey() {
    return searchPool[0]->getZobristKey();
}


void SearchManager::setForceCheck(bool a) {
    for (Search *s:searchPool) {
        s->setForceCheck(a);
    }
}

void SearchManager::setRunningAllThread(int r) {
    for (Search *s:searchPool) {
        s->setRunningThread(r);
    }
}

void SearchManager::setRunningAll(int r) {
    for (int i = 0; i < N_THREAD; i++) {
        setRunning(r);
    }
}

void SearchManager::setRunning(int i) {
    for (Search *s:searchPool) {
        s->setRunning(i);
    }
}

int SearchManager::getRunning(int i) {
    return searchPool[i]->getRunning();
}

void SearchManager::join(int i) {
    searchPool[i]->join();
}

void SearchManager::display() {
    searchPool[0]->display();
}

string SearchManager::getFen() {
    return searchPool[0]->getFen();
}

bool SearchManager::setHashSize(int s) {
    return searchPool[0]->setHashSize(s);
}

void SearchManager::setMaxTimeMillsec(int i) {
    for (Search *s:searchPool) {
        s->setMaxTimeMillsec(i);
    }
}

void SearchManager::setPonder(bool i) {
    for (Search *s:searchPool) {
        s->setPonder(i);
    }
}

int SearchManager::getSide() {
    ASSERT(searchPool[0]->getSide() == searchPool[1]->getSide() == searchPool[2]->getSide() == searchPool[3]->getSide());
    return searchPool[0]->getSide();
}

int SearchManager::getScore(int side) {
    ASSERT(searchPool[0]->getScore(side) == searchPool[1]->getScore(side) == searchPool[2]->getScore(side) == searchPool[3]->getScore(side));
    return searchPool[0]->getScore(side);
}

void SearchManager::clearHash() {
    searchPool[0]->clearHash();
}

int SearchManager::getMaxTimeMillsec() {
    return searchPool[0]->getMaxTimeMillsec();
}

void SearchManager::setNullMove(bool i) {
    for (Search *s:searchPool) {
        s->setNullMove(i);
    }
}

bool SearchManager::makemove(_Tmove *i) {
    bool b = false;
    for (Search *s:searchPool) {
        b = s->makemove(i);
    }
    return b;
}

void SearchManager::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    for (Search *s:searchPool) {
        s->takeback(move, oldkey, rep);
    }
}

void SearchManager::setSide(bool i) {
    for (Search *s:searchPool) {
        s->setSide(i);
    }
}

bool SearchManager::getGtbAvailable() {
    return searchPool[0]->getGtbAvailable();
}

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) {
    ASSERT(searchPool[0]->getMoveFromSan(string, ptr) == searchPool[1]->getMoveFromSan(string, ptr) == searchPool[2]->getMoveFromSan(string, ptr) == searchPool[3]->getMoveFromSan(string, ptr));
    return searchPool[0]->getMoveFromSan(string, ptr);
}

int SearchManager::printDtm() {
    return searchPool[0]->printDtm();
}


void SearchManager::setGtb(Tablebase &tablebase) {
    for (Search *s:searchPool) {
        s->setGtb(tablebase);
    }
}

void SearchManager::pushStackMove() {
    for (Search *s:searchPool) {
        s->pushStackMove();
    }
}

void SearchManager::init() {
    for (Search *s:searchPool) {
        s->init();
    }
}

void SearchManager::setRepetitionMapCount(int i) {
    for (Search *s:searchPool) {
        s->setRepetitionMapCount(i);
    }
}

void SearchManager::deleteGtb() {
    for (Search *s:searchPool) {
        s->deleteGtb();
    }
}

