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
#include "util/IniFile.h"

Hash *SearchManager::hash;

SearchManager::SearchManager() : ThreadPool(1) {//TODO 1
    nThreads = getNthread();
    hash = &Hash::getInstance();
    setNthread(nThreads);
    IniFile iniFile("cinnamon.ini");

    while (true) {
        pair<string, string> *parameters = iniFile.get();
        if (!parameters) {
            break;
        }
        string param = parameters->first;
        int value = stoi(parameters->second);
        cout << param << endl;
        cout << value << endl;

        if (param == "threads") {
            setNthread(value);
        } else {
            if (!setParameter(param, value)) {
                cout << "error parameter " << param << " not defined\n";
            };
        }
    }

}

void SearchManager::search(int mply) {
    if (nThreads > 1 && mply > 3) {//TODO
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
        debug( "val: ", valWindow);

        threadPool[0]->run(SMP_NO, mply, -_INFINITE, _INFINITE);
        valWindow = threadPool[0]->getValue();
        totCountWin += threadPool[0]->getTotMoves();
        memcpy(&lineWin, &threadPool[0]->getPvLine(), sizeof(_TpvLine));

    } else {
        threadPool[0]->init();

        //Aspiration Windows
        threadPool[0]->run(SMP_NO, mply, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW);
        int tmp = threadPool[0]->getValue();
        if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {

            if (tmp <= threadPool[0]->getMainAlpha()) {
                threadPool[0]->run(SMP_NO, mply, valWindow - VAL_WINDOW * 2, valWindow + VAL_WINDOW);
            } else {
                threadPool[0]->run(SMP_NO, mply, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 2);
            }
            tmp = threadPool[0]->getValue();
            if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {

                if (tmp <= threadPool[0]->getMainAlpha()) {
                    threadPool[0]->run(SMP_NO, mply, valWindow - VAL_WINDOW * 4, valWindow + VAL_WINDOW);
                } else {
                    threadPool[0]->run(SMP_NO, mply, valWindow - VAL_WINDOW, valWindow + VAL_WINDOW * 4);
                }
                tmp = threadPool[0]->getValue();
                if (tmp <= threadPool[0]->getMainAlpha() || tmp >= threadPool[0]->getMainBeta()) {
                    threadPool[0]->run(SMP_NO, mply, -_INFINITE, _INFINITE);
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
    ASSERT(!getBitCount());

    if (mply == 1) {
        Search &idThread1 = getNextThread();
        debug( "start loop1 ------------------------------ run threadid: ", idThread1.getId());
        debug( "val: ", valWindow);
        startThread(SMP_NO, idThread1, mply, -_INFINITE, _INFINITE);
        idThread1.join();
    } else {
//  Parallel Aspiration Windows
        debug( "start loop2 --------------------------");
        ASSERT(nThreads);
        ASSERT(!getBitCount());
        ASSERT(lineWin.cmove <= 0);
        for (int ii = 0; ii < std::max(3, getNthread()); ii++) {

            int alpha = valWindow - VAL_WINDOW * (int) POW2[ii];
            int beta = valWindow + VAL_WINDOW * (int) POW2[ii];

            if (alpha <= -_INFINITE || beta >= _INFINITE) {
                break;
            }

            Search &idThread1 = getNextThread();
            idThread1.setRunning(1);
            debug( "val: ", valWindow);
            startThread(SMP_YES, idThread1, mply, alpha, beta);
        }
        debug( "end loop2 ---------------------------");
        joinAll();
        ASSERT(!getBitCount());
        if (lineWin.cmove <= 0) {

            debug( "start loop3 -------------------------------");
//            for (int i = 0; i < getNthread(); i++) {
            Search &idThread1 = getNextThread();
            idThread1.setRunning(1);
            startThread(SMP_NO, idThread1, mply, -_INFINITE, _INFINITE);
//            }
            debug( "end loop3 -------------------------------");
            idThread1.join();
        }
    }
}

void SearchManager::receiveObserverSearch(int threadID) {
	mutexSearch.lock();    
    if (getRunning(threadID)) {
        if (lineWin.cmove == -1) {
            int t = threadPool[threadID]->getValue();
            if (t > threadPool[threadID]->getMainAlpha() && t < threadPool[threadID]->getMainBeta()) {
                memcpy(&lineWin, &threadPool[threadID]->getPvLine(), sizeof(_TpvLine));
                mateIn = threadPool[threadID]->getMateIn();
                ASSERT(mateIn == INT_MAX);
                totCountWin += threadPool[threadID]->getTotMoves();
                valWindow = getValue(threadID);
                debug( "win", threadID);
                ASSERT(lineWin.cmove);
                stopAllThread();
            }
        }
    }
	mutexSearch.unlock();
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
}

int SearchManager::loadFen(const string &fen) {
    int res = threadPool[0]->loadFen(fen);

    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < threadPool.size(); i++) {
        threadPool[i]->setChessboard(threadPool[0]->getChessboard());
    }
    return res;
}

void SearchManager::startThread(bool smpMode, Search &thread, int depth, int alpha, int beta) {

    debug( "startThread: ", thread.getId(), " depth: ", depth, " alpha: ", alpha, " beta: ", beta, " isrunning: ", getRunning(thread.getId()));
    ASSERT(alpha >= -_INFINITE);

    thread.setMainParam(smpMode, depth, alpha, beta);
    thread.init();
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
    return hash->getHashSize();
}

int SearchManager::getValue(int i) {
    return threadPool[i]->getValue();
}

void SearchManager::startClock() {
    threadPool[0]->startClock();// static variable
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
    hash->clearAge();
}

int SearchManager::getForceCheck() {
    return threadPool[0]->getForceCheck();// static variable
}

u64 SearchManager::getZobristKey(int id) {
    return threadPool[id]->getZobristKey();
}

void SearchManager::setForceCheck(bool a) {
    threadPool[0]->setForceCheck(a);    // static variable
}

void SearchManager::setRunningThread(bool r) {
    threadPool[0]->setRunningThread(r);// static variable
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

void SearchManager::setHashSize(int s) {
    hash->setHashSize(s);
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
    //int N_PIECE;
    //if (side == WHITE) {
    //    N_PIECE = threadPool[0]->getScore(Bits::bitCount(threadPool[0]->getBitBoard<WHITE>()), side);
    //} else {
    //    N_PIECE = threadPool[0]->getScore(Bits::bitCount(threadPool[0]->getBitBoard<BLACK>()), side);
    //}
#ifdef DEBUG_MODE
    int t = threadPool[0]->getScore(side);
    for (Search *s:threadPool) {
        ASSERT(s->getScore(side) == t);
    }
#endif
    return threadPool[0]->getScore(side);
}

void SearchManager::clearHash() {
    hash->clearHash();
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

int SearchManager::getMoveFromSan(const String &string, _Tmove *ptr) {
#ifdef DEBUG_MODE
    int t = threadPool[0]->getMoveFromSan(string, ptr);
    for (Search *s:threadPool) {
        ASSERT(s->getMoveFromSan(string, ptr) == t);
    }
#endif
    return threadPool[0]->getMoveFromSan(string, ptr);
}

Tablebase &SearchManager::getGtb() {
    return threadPool[0]->getGtb();
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

bool SearchManager::setNthread(int nthread) {
    ThreadPool::setNthread(nthread);
    nThreads = nthread;
    for (Search *s:threadPool) {
        s->registerObserver(this);
    }
    return true;
}

void SearchManager::stopAllThread() {
    threadPool[0]->setRunningThread(false);//is static
}

bool SearchManager::setParameter(String param, int value) {
    bool b = false;
    for (Search *s:threadPool) {
        b = s->setParameter(param, value);
    }
    return b;
}


Tablebase &SearchManager::createGtb() {
    Tablebase &gtb = Tablebase::getInstance();
    setGtb(gtb);
    return gtb;
}
