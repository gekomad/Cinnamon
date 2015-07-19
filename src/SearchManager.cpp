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

#include <future>
#include "SearchManager.h"
#include "namespaces.h"


void SearchManager::parallelSearch(int mply) {

    threadWin = -1;

    setMainPly(mply);
//  if (mply < 5) {
    if (mply == 1) {
        int last = getNthread() - 1;
        startThread(*searchPool[last], mply);
        join(last);
        valWindow = getValue(last);
    } else {
//  Parallel Aspiration
        for (Search *s:searchPool) {
            ASSERT(s->getRunning());
            startThread(*s, mply);
        }

        joinAll();

//        if (threadWin == -1) {
//            ThreadPool:
//            init();
//            //  searchMoves.clone(searchPool[0]);
//            //PVSalpha = -_INFINITE;
//            PVSplit(getNextThread(), mply, -_INFINITE, _INFINITE);
//        }
    }
}

int SearchManager::PVSplit(int idThread1, const int depth, int alpha, int beta) {

    if (depth < 5) {
        return searchPool[idThread1]->searchNOparall(depth, alpha, beta);
    }

    searchPool[idThread1]->incListId();
    searchPool[idThread1]->generateCapturesMoves();//TODO return false?
    u64 oldKey = searchPool[idThread1]->chessboard[ZOBRISTKEY_IDX];
    _Tmove *move = searchPool[idThread1]->getNextMove();

    searchPool[idThread1]->makemove(move, true, false);


    int score = -PVSplit(idThread1, depth - 1, -beta, -alpha);
    searchPool[idThread1]->takeback(move, oldKey, true);
    if (score > alpha) alpha = score;
    searchPool[idThread1]->decListId();
    releaseThread(idThread1);

    if (score > beta) {
        beta = score;
        return beta;
    }
    while ((move = searchPool[idThread1]->getNextMove())) {
        cout << "fuori" << endl;
        idThread1 = getNextThread();
        cout << "dentro" << endl;
        u64 oldKey1 = searchPool[idThread1]->chessboard[ZOBRISTKEY_IDX];

        rollbackValue[idThread1]->oldKey = oldKey1;
        rollbackValue[idThread1]->move = move;

        searchPool[idThread1]->makemove(move, true, false);
        searchPool[idThread1]->setPVSplit(depth, alpha, beta, oldKey1);
        searchPool[idThread1]->start();
    }
    return 0;
}

SearchManager::~SearchManager() {
    for (Search *s:searchPool) {
        delete s;
        s = nullptr;
    }
    for (unsigned i = 0; i < rollbackValue.size(); i++) {
        delete rollbackValue[i];
    }
    rollbackValue.clear();
}

int SearchManager::loadFen(string fen) {
    int res = searchPool[0]->loadFen(fen);
    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < searchPool.size(); i++) {
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

void SearchManager::getWindowRange(int threadID, const int val, int *from, int *to) {
    if (threadID == getNthread() - 1) {
        //last
        *from = -_INFINITE;
        *to = _INFINITE;
    } else {
        *from = val - VAL_WINDOW * (int) POW2[threadID];
        *to = val + VAL_WINDOW * (int) POW2[threadID];
    }
}


void SearchManager::updateAB(int depth, int side, int score) {
    if (side > 0) {
        alphaValue[depth] = max(alphaValue[depth], score);
    } else {
        betaValue[depth] = min(betaValue[depth], score);
    }
    if (depth > 0) {
        updateAB(depth - 1, -side, -score);
    }
}

void SearchManager::receiveObserverPVSplit(int threadID, int score) {
    assert(0);

//    lock_guard<mutex> lock1(mutex1);
//    updateAB(depth,side,score);
//    if (score > alphaValue[depth]) alphaValue[depth] = score;
//    if (score > betaValue[depth]) {
//        searchPool[threadID]->takeback(rollbackValue[threadID]->move, rollbackValue[threadID]->oldKey, true);
//        searchPool[threadID]->decListId();
//        releaseThread(threadID);
//        //PVSbeta = score;
//    }
//    releaseThread(threadID);
}

void SearchManager::receiveObserverSearch(int threadID) {
    if (getRunning(threadID)) {
        lock_guard<mutex> lock(mutexSearch);
        if (threadWin == -1) {
            int t = searchPool[threadID]->getValue();

#ifdef DEBUG_MODE
            int from, to;
            getWindowRange(threadID, valWindow, &from, &to);
            ASSERT(from == searchPool[threadID]->getThreadAlpha() && to == searchPool[threadID]->getThreadBeta());
#endif
            if (t > searchPool[threadID]->getThreadAlpha() && t < searchPool[threadID]->getThreadBeta()) {
                valWindow = t;
                threadWin = threadID;
                ASSERT(searchPool[threadWin]->getPvLine().cmove);
                for (Search *s:searchPool) {
                    s->setRunningThread(true);
                }
            }
        }
    }
}

SearchManager::SearchManager() {
    registerThreads();
}


void SearchManager::startThread(Search &thread, int depth) {
    int alpha, beta;
    getWindowRange(thread.getId(), valWindow, &alpha, &beta);
    thread.search(depth, alpha, beta);
    thread.start();
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

int SearchManager::getPieceAt(int side, u64 i) {
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
    for (Search *s:searchPool) {
        s->setRunning(r);
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

bool SearchManager::setThread(int thread) {
    if (thread > 0 && thread <= ThreadPool::MAX_THREAD) {
        ThreadPool::setNthread(thread);
        registerThreads();
        return true;
    }
    return false;
}

void SearchManager::registerThreads() {
    for (unsigned i = 0; i < rollbackValue.size(); i++) {
        delete rollbackValue[i];
    }
    rollbackValue.clear();
    for (Search *s:searchPool) {
        s->registerObserver(this);
        rollbackValue.push_back(new _RollbackValue);
    }
}
