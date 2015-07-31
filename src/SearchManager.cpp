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

#include "SearchManager.h"

void SearchManager::parallelSearch(int mply) {
    lineWin.cmove = -1;

    setMainPly(mply);
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    ASSERT(!getBitCount());
    if (mply == 1) {
        nJoined = 0;
        finish = false;
        setNthread(1);
//        activeThread = 1;
        Search &idThread1 = getNextThread();
        debug("start loop1 ------------------------------ run threadid: ", idThread1.getId(), "count:", getBitCount());
        idThread1.init();
//        Search &i = *threadPool[0];

        debug("val: ", valWindow);
        startThread(idThread1, mply, -_INFINITE, _INFINITE);
        countTot=1;
        exitLoop.wait(lk, [this] { return finish == true; });
        idThread1.join();
    } else {
//  Parallel Aspiration
        debug("start loop2 --------------------------count:", getBitCount());
        ASSERT(nThreads);
        ASSERT(!getBitCount());
        //setNthread(nThreads);
        //setNthread(mply < 6 ? 1 : nThreads);
        nJoined = 0;
        finish = false;
        countTot=-1;
        int count=0;
//        activeThread = std::max(3, getNthread());
        for (int ii = 0; ii < std::max(3, getNthread()); ii++) {
            count++;
            Search &idThread1 = getNextThread();
            idThread1.init();
//            Search &idThread1 = *threadPool[0];

            int alpha = valWindow - VAL_WINDOW * (int) POW2[ii];
            int beta = valWindow + VAL_WINDOW * (int) POW2[ii];

            idThread1.setRunning(1);
            // debug("val: ", valWindow);
            startThread(idThread1, mply, alpha, beta);

        }
        debug("end loop2 ---------------------------count:", getBitCount());
        countTot=count;
        exitLoop.wait(lk, [this] { return finish == true; });
        joinAll();
        ASSERT(!getBitCount());
        if (!lineWin.cmove) {
            debug("start loop3 -------------------------------count:", getBitCount());
            nJoined = 0;
            finish = false;
            setNthread(1);
            Search &idThread1 = getNextThread();
            idThread1.init();
            idThread1.setRunning(1);
            //debug("val: ", valWindow);
            startThread(idThread1, mply, -_INFINITE, _INFINITE);//PVS
            countTot=1;
            exitLoop.wait(lk, [this] { return finish == true; });
            idThread1.join();
            debug("end loop3 -------------------------------count:", getBitCount());
        }
        ASSERT(!getBitCount());

    }

}

void SearchManager::receiveObserverSearch(int threadID) {

    lock_guard<mutex> lock(mutexSearch);

    if (getRunning(threadID)) {

        if (lineWin.cmove == -1) {
            int t = threadPool[threadID]->getValue();
            if (t > threadPool[threadID]->getMainAlpha() && t < threadPool[threadID]->getMainBeta()) {

                memcpy(&lineWin, &threadPool[threadID]->getPvLine(), sizeof(_TpvLine));
                totCountWin += threadPool[threadID]->getTotMoves();
                valWindow = getValue(threadID);

                debug("win", threadID);
                ASSERT(lineWin.cmove);
                stopAllThread();
            }
        }
    }
    ++nJoined;

    debug("SearchManager::receiveObserverSearch nJoined:",(int) nJoined, "activeThread:", getNthread(), "count:", getBitCount());
    if ( nJoined == countTot) {
        ASSERT(lineWin.cmove);
        nJoined = 0;
        countTot=-1;
        finish = true;
        debug("SearchManager::receiveObserverSearch notify_one: ", threadID, "count:", getBitCount());
        exitLoop.notify_one();
    }
}

bool SearchManager::getRes(_Tmove &resultMove, string &ponderMove, string &pvv) {
    if (lineWin.cmove == -1) {
        return false;
    }
    pvv.clear();
    string pvvTmp;

    ASSERT(lineWin.cmove);
    for (int t = 0; t < lineWin.cmove; t++) {
        pvvTmp.clear();
        pvvTmp += Search::decodeBoardinv(lineWin.argmove[t].type, lineWin.argmove[t].from, threadPool[0]->getSide());
        if (pvvTmp.length() != 4) {
            pvvTmp += Search::decodeBoardinv(lineWin.argmove[t].type, lineWin.argmove[t].to, threadPool[0]->getSide());
        }
        pvv.append(pvvTmp);
        if (t == 1) {
            ponderMove.assign(pvvTmp);
        }
        pvv.append(" ");
    };
    memcpy(&resultMove, lineWin.argmove, sizeof(_Tmove));

    return true;
}

int SearchManager::PVSplit(int idThread1, const int depth, int alpha, int beta) {

    if (depth < 5) {
        return threadPool[idThread1]->searchNOparall(depth, alpha, beta);
    }

    threadPool[idThread1]->incListId();
    threadPool[idThread1]->generateCapturesMoves();//TODO return false?
    u64 oldKey = threadPool[idThread1]->chessboard[ZOBRISTKEY_IDX];
    _Tmove *move = threadPool[idThread1]->getNextMove();

    threadPool[idThread1]->makemove(move, true, false);


    int score = -PVSplit(idThread1, depth - 1, -beta, -alpha);
    threadPool[idThread1]->takeback(move, oldKey, true);
    if (score > alpha) alpha = score;
    threadPool[idThread1]->decListId();

    if (score > beta) {
        beta = score;
        return beta;
    }
    while ((move = threadPool[idThread1]->getNextMove())) {
        debug("fuori");
        idThread1 = getNextThread().getId();
        debug("dentro");
        u64 oldKey1 = threadPool[idThread1]->chessboard[ZOBRISTKEY_IDX];

        rollbackValue[idThread1]->oldKey = oldKey1;
        rollbackValue[idThread1]->move = move;

        threadPool[idThread1]->makemove(move, true, false);
        threadPool[idThread1]->setPVSplit(depth, alpha, beta, oldKey1);
        threadPool[idThread1]->start();
    }
    return 0;
}


SearchManager::~SearchManager() {
    for (Search *s:threadPool) {
        delete s;
        s = nullptr;
    }
    for (unsigned i = 0; i < rollbackValue.size(); i++) {
        delete rollbackValue[i];
    }
    rollbackValue.clear();
}

int SearchManager::loadFen(string fen) {
    int res = threadPool[0]->loadFen(fen);
    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < threadPool.size(); i++) {
        threadPool[i]->setChessboard(threadPool[0]->getChessboard());
    }
    return res;
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
//        threadPool[threadID]->takeback(rollbackValue[threadID]->move, rollbackValue[threadID]->oldKey, true);
//        threadPool[threadID]->decListId();
//        releaseThread(threadID);
//        //PVSbeta = score;
//    }
//    releaseThread(threadID);
}


SearchManager::SearchManager() {
    nThreads = getNthread();
    for (unsigned i = 0; i < rollbackValue.size(); i++) {
        delete rollbackValue[i];
    }
    rollbackValue.clear();
    for (Search *s:threadPool) {
        s->registerObserver(this);
        rollbackValue.push_back(new _RollbackValue);
    }
    registerThreads();
}


void SearchManager::startThread(Search &thread, int depth, int alpha, int beta) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " alpha: ", alpha, " beta: ", beta, " isrunning: ", getRunning(thread.getId()));
    ASSERT(alpha >= -_INFINITE);

    thread.search(depth, alpha, beta);
    thread.start();
}

void SearchManager::setMainPly(int r) {
    for (Search *s:threadPool) {
        s->setMainPly(r);
    }
}

int SearchManager::getPieceAt(int side, u64 i) {
    return side == WHITE ? threadPool[0]->getPieceAt<WHITE>(i) : threadPool[0]->getPieceAt<BLACK>(i);
}

u64 SearchManager::getTotMoves() {
//    u64 i = 0;
//    for (Search *s:threadPool) {
//        i += s->getTotMoves();
//    }
//    return i;
    return totCountWin;
}

void SearchManager::incKillerHeuristic(int from, int to, int value) {
    for (Search *s:threadPool) {
        s->incKillerHeuristic(from, to, value);
    }
}

int SearchManager::getHashSize() {
    return threadPool[0]->getHashSize();
}

int SearchManager::getValue(int i) {
    return threadPool[i]->getValue();
}

int SearchManager::getMateIn() {
    assert(0);
    //return threadPool[threadWin]->getMateIn();
}


void SearchManager::startClock() {
    for (Search *s:threadPool) {
        s->startClock();
    }
}

string SearchManager::boardToFen() {
    return threadPool[0]->boardToFen();
}

void SearchManager::clearKillerHeuristic() {
    for (Search *s:threadPool) {
        s->clearKillerHeuristic();
    }
}

void SearchManager::clearAge() {
    for (Search *s:threadPool) {
        s->clearAge();
    }
}

int SearchManager::getForceCheck() {
    return threadPool[0]->getForceCheck();
}

u64 SearchManager::getZobristKey() {
    return threadPool[0]->getZobristKey();
}

void SearchManager::setForceCheck(bool a) {
    for (Search *s:threadPool) {
        s->setForceCheck(a);
    }
}

void SearchManager::setRunningThread(bool r) {
    threadPool[0]->setRunningThread(r);// is static
}

void SearchManager::setRunningAll(int r) {
    for (Search *s:threadPool) {
        s->setRunning(r);
    }
}

void SearchManager::setRunning(int i) {
    for (Search *s:threadPool) {
        s->setRunning(i);
    }
}

int SearchManager::getRunning(int i) {
    return threadPool[i]->getRunning();
}

void SearchManager::display() {
    threadPool[0]->display();
}

string SearchManager::getFen() {
    return threadPool[0]->getFen();
}

bool SearchManager::setHashSize(int s) {
    return threadPool[0]->setHashSize(s);
}

void SearchManager::setMaxTimeMillsec(int i) {
    for (Search *s:threadPool) {
        s->setMaxTimeMillsec(i);
    }
}

void SearchManager::setPonder(bool i) {
    for (Search *s:threadPool) {
        s->setPonder(i);
    }
}

int SearchManager::getSide() {
#ifdef DEBUG_MODE
    int t = threadPool[0]->getSide();
    for (Search *s:threadPool) {
        ASSERT(s->getSide() == t);
    }
#endif
    return threadPool[0]->getSide();
}

int SearchManager::getScore(int side) {
#ifdef DEBUG_MODE
    int t = threadPool[0]->getScore(side);
    for (Search *s:threadPool) {
        ASSERT(s->getScore(side) == t);
    }
#endif
    return threadPool[0]->getScore(side);
}

void SearchManager::clearHash() {
    threadPool[0]->clearHash();
}

int SearchManager::getMaxTimeMillsec() {
    return threadPool[0]->getMaxTimeMillsec();
}

void SearchManager::setNullMove(bool i) {
    for (Search *s:threadPool) {
        s->setNullMove(i);
    }
}

bool SearchManager::makemove(_Tmove *i) {
    bool b = false;
    for (Search *s:threadPool) {
        b = s->makemove(i);
    }
    return b;
}

void SearchManager::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    for (Search *s:threadPool) {
        s->takeback(move, oldkey, rep);
    }
}

void SearchManager::setSide(bool i) {
    for (Search *s:threadPool) {
        s->setSide(i);
    }
}

bool SearchManager::getGtbAvailable() {
    return threadPool[0]->getGtbAvailable();
}

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) {
#ifdef DEBUG_MODE
    int t = threadPool[0]->getMoveFromSan(string, ptr);
    for (Search *s:threadPool) {
        ASSERT(s->getMoveFromSan(string, ptr) == t);
    }
#endif
    return threadPool[0]->getMoveFromSan(string, ptr);
}

int SearchManager::printDtm() {
    return threadPool[0]->printDtm();
}

void SearchManager::setGtb(Tablebase &tablebase) {
    for (Search *s:threadPool) {
        s->setGtb(tablebase);
    }
}

void SearchManager::pushStackMove() {
    for (Search *s:threadPool) {
        s->pushStackMove();
    }
}

void SearchManager::init() {
    totCountWin = 0;
    for (Search *s:threadPool) {
        s->init();
    }
}

void SearchManager::setRepetitionMapCount(int i) {
    for (Search *s:threadPool) {
        s->setRepetitionMapCount(i);
    }
}

void SearchManager::deleteGtb() {
    for (Search *s:threadPool) {
        s->deleteGtb();
    }
}

bool SearchManager::setThread(int nthread) {
    if (nthread > 0 && nthread <= ThreadPool::MAX_THREAD) {
        ThreadPool::setNthread(nthread);
        nThreads = nthread;
        registerThreads();
        return true;
    }
    return false;
}


void SearchManager::stopAllThread() {
    threadPool[0]->setRunningThread(false);//is static
}

