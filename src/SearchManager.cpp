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
#include "namespaces.h"

void SearchManager::search(int mply) {
    if (nThreads > 1 && mply > 3) {//TODO
//        initAB(mply);
        parallelSearch(mply);
    } else {
        singleSearch(mply);
    }
}

void SearchManager::singleSearch(int mply) {
    lineWin.cmove = -1;
    setMainPly(mply);
    ASSERT(!getBitCount());
    if (mply == 1) {
        threadPool[0]->init();
        debug("val: ", valWindow);

        threadPool[0]->run(false, false, mply, -_INFINITE, _INFINITE);
        valWindow = threadPool[0]->getValue();
        //if (threadPool[0]->getRunning()) {
        memcpy(&lineWin, &threadPool[0]->getPvLine(), sizeof(_TpvLine));
        //}
    } else {
        threadPool[0]->init();

        //Aspiration Windows TODO provare solo a destra o sinistra
        threadPool[0]->run(false, false, mply, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);
        int tmp = threadPool[0]->getValue();
        if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {

            threadPool[0]->run(false, false, mply, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW * 2);
            tmp = threadPool[0]->getValue();
            if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {

                threadPool[0]->run(false, false, mply, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW * 4);
                tmp = threadPool[0]->getValue();
                if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {

                    threadPool[0]->run(false, false, mply, -_INFINITE, _INFINITE);
                    tmp = threadPool[0]->getValue();
                }
            }
        }

        if (threadPool[0]->getRunning()) {
            valWindow = tmp;
            totCountWin += threadPool[0]->getTotMoves();
            memcpy(&lineWin, &threadPool[0]->getPvLine(), sizeof(_TpvLine));
        }
    }
}


void SearchManager::parallelSearch(int mply) {
    lineWin.cmove = -1;
    setMainPly(mply);
    forceMainThread = -1;
    ASSERT(!getBitCount());
    if (mply == 1) {
        Search &idThread1 = getNextThread();
        debug("start loop1 ------------------------------ run threadid: ", idThread1.getId(), "count:", getBitCount());
        idThread1.init();
        debug("val: ", valWindow);
        startThread(false, idThread1, mply, -_INFINITE, _INFINITE);
        idThread1.join();
    } else {
//  Parallel Aspiration Windows
        debug("start loop2 --------------------------count:", getBitCount());
        ASSERT(nThreads);
        ASSERT(!getBitCount());
        for (int ii = 0; ii < std::max(3, getNthread()); ii++) {
            Search &idThread1 = getNextThread();
            idThread1.init();

            int alpha = valWindow - VAL_WINDOW * (int) POW2[ii];
            int beta = valWindow + VAL_WINDOW * (int) POW2[ii];

            idThread1.setRunning(1);
            debug("val: ", valWindow);
            startThread(false, idThread1, mply, alpha, beta);
        }
        debug("end loop2 ---------------------------count:", getBitCount());
        joinAll();
        ASSERT(!getBitCount());
        if (!lineWin.cmove) {
            //LAZY SMP
            cout << "-------------------------\n";
            debug("start loop3 -------------------------------count:", getBitCount());
            //master
            Search &master = getNextThread();
            forceMainThread = master.getId();
            master.init();
            master.setRunning(1);
            startThread(false, master, mply, -_INFINITE, _INFINITE);
            for (int i = 1; i < getNthread() - 1; i++) {
                //helper
                Search &idThread1 = getNextThread();
                idThread1.init();
                idThread1.setRunning(2);
                if ((i % 2) == 0) {
                    startThread(false, idThread1, mply, -_INFINITE, _INFINITE);
                } else {
                    startThread(false, idThread1, mply + 1, -_INFINITE, _INFINITE);
                }
            }
            debug("end loop3 -------------------------------count:", getBitCount());

            master.join();
            stopAllThread();
            joinAll();
            cout << "-----------------s--------\n";
            ASSERT(!getBitCount());
        }
    }
}

//void SearchManager::treeSplit(Search &idThread1, int alpha, const int beta, const int depth, _Tmove *move1) {
//
//    int value[GenMoves::MAX_MOVE];
//    if (depth < 5) {
//        idThread1.display();
////        startThread(idThread1, depth,alphaValue[depth], betaValue[depth]);
//        idThread1.setRunning(1);
//        idThread1.setRunningThread(true);
//        idThread1.search(false, depth, alphaValue[depth], betaValue[depth]);
////        int t = idThread1.getValue();
//
//        memcpy(&lineWin, &idThread1.getPvLine(), sizeof(_TpvLine));
//        mateIn = idThread1.getMateIn();
//        ASSERT(mateIn == INT_MAX);
//        totCountWin += idThread1.getTotMoves();
////        valWindow = getValue(threadID);
////        debug("win", threadID);
//        ASSERT(lineWin.cmove);
//        stopAllThread();
////        idThread1.join();
//        return;
//    }
//    idThread1.incListId();
//idThread1.generateCapturesMoves();
//    int i = 0;
//    _Tmove *move;
//    while ((move = idThread1.getNextMove())) {
//        Search &search = getNextThread();
////        value[i] = -treeSplit(search, -beta, -alpha, depth - 1, move);
//        {
//            lock_guard<mutex> lock(mutexTreeSplit);
//            if (value[i] > alpha) {
//                alpha = value[i];
//            }
//        }
//        if (alpha > beta) {
//            stopAllThread();
//            joinAll();
//            return;
//        }
//
//    }
//}

//int SearchManager::PVSplit(Search &idThread1, const int depth) {
//    ASSERT_RANGE(depth, 0, MAX_PLY);
//    if (depth < 5) {
//        idThread1.display();
////        startThread(idThread1, depth,alphaValue[depth], betaValue[depth]);
//        idThread1.setRunning(1);
//        idThread1.setRunningThread(true);
//        idThread1.search(false, depth, alphaValue[depth], betaValue[depth]);
////        int t = idThread1.getValue();
//
//        memcpy(&lineWin, &idThread1.getPvLine(), sizeof(_TpvLine));
//        mateIn = idThread1.getMateIn();
//        ASSERT(mateIn == INT_MAX);
//        totCountWin += idThread1.getTotMoves();
////        valWindow = getValue(threadID);
////        debug("win", threadID);
//        ASSERT(lineWin.cmove);
//        stopAllThread();
////        idThread1.join();
//    }
//
//    idThread1.incListId();
//    idThread1.generateCapturesMoves();//TODO return false?
//    u64 oldKey = idThread1.chessboard[ZOBRISTKEY_IDX];
//    _Tmove *move = idThread1.getNextMove();
//
//    idThread1.makemove(move, true, false);
//
//
//    int score = -PVSplit(idThread1, depth - 1);
//    idThread1.takeback(move, oldKey, true);
////    if (score > alphaValue[depth]) updateAB(depth, idThread1.getSide(), score);
//    idThread1.decListId();
//
//    if (score > betaValue[depth]) {
////        updateAB(depth, idThread1.getSide(), score);
//        return score;
//    }
//    while ((move = idThread1.getNextMove())) {
//        Search &idThread2 = getNextThread();
//        idThread2.setPVSplit(depth, alphaValue[depth], betaValue[depth], move);
//        idThread2.start();
//    }
//    return 0;
//}

void SearchManager::receiveObserverSearch(int threadID) {

    lock_guard<mutex> lock(mutexSearch);
    if (forceMainThread != -1 && forceMainThread != threadID) {
        return;
    }
    if (threadPool[threadID]->getMainSmp()) {
//        updateAB(threadPool[threadID]->getMainDepth(), getSide(), threadPool[threadID]->getValue());
    } else {
        if (getRunning(threadID)) {
            if (lineWin.cmove == -1) {
                int t = threadPool[threadID]->getValue();
                if (t > threadPool[threadID]->getMainAlpha() && t < threadPool[threadID]->getMainBeta()) {
                    memcpy(&lineWin, &threadPool[threadID]->getPvLine(), sizeof(_TpvLine));
                    mateIn = threadPool[threadID]->getMateIn();
                    ASSERT(mateIn == INT_MAX);
                    totCountWin += threadPool[threadID]->getTotMoves();
                    valWindow = getValue(threadID);
                    debug("win", threadID);
                    ASSERT(lineWin.cmove);
                    stopAllThread();
                }
            }
        }
    }
}

bool SearchManager::getRes(_Tmove &resultMove, string &ponderMove, string &pvv, int *mateIn1) {
    if (lineWin.cmove < 1) {
        return false;
    }

    *mateIn1 = mateIn;
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

SearchManager::~SearchManager() {
//    for (unsigned i = 0; i < rollbackValue.size(); i++) {
//        delete rollbackValue[i];
//    }
//    rollbackValue.clear();
}

int SearchManager::loadFen(string fen) {
    int res = threadPool[0]->loadFen(fen);
    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < threadPool.size(); i++) {
        threadPool[i]->setChessboard(threadPool[0]->getChessboard());
    }
    return res;
}

//void SearchManager::initAB(int depth) {
//    for (int i = 0; i <= depth; i++) {
//        alphaValue[i] = -_INFINITE;
//        betaValue[i] = _INFINITE;
//    }
//}

//void SearchManager::updateAB(int depth, bool side, int score) {
//    lock_guard<mutex> lock1(mutexPvs);
//    while (depth) {
//        if (side == BLACK) {
//            alphaValue[depth] = max(alphaValue[depth], score);
//        } else {
//            betaValue[depth] = min(betaValue[depth], score);
//        }
////        if (depth <= 0)break;
////        updateAB(depth - 1, side ^ 1, -score);
//        depth--;
//        side ^= 1;
//        score--;
//
//    }
//}

SearchManager::SearchManager() {
    nThreads = getNthread();
//    for (unsigned i = 0; i < rollbackValue.size(); i++) {
//        delete rollbackValue[i];
//    }
//    rollbackValue.clear();
    for (Search *s:threadPool) {
        s->registerObserver(this);
//        rollbackValue.push_back(new _RollbackValue);
    }
}


void SearchManager::startThread(bool smpMode, Search &thread, int depth, int alpha, int beta) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " alpha: ", alpha, " beta: ", beta, " isrunning: ",
          getRunning(thread.getId()));
    ASSERT(alpha >= -_INFINITE);

    thread.setMainParam(false, smpMode, depth, alpha, beta);
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
        s->startClock();//TODO static
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
    threadPool[0]->clearAge();
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
    if (nthread > 0 && nthread <= 8) {
        ThreadPool::setNthread(nthread);
        nThreads = nthread;
        return true;
    }
    return false;
}


void SearchManager::stopAllThread() {
    threadPool[0]->setRunningThread(false);//is static
}

