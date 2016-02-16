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

Hash *SearchManager::hash;

SearchManager::SearchManager() {
    SET(checkSmp1, 0);
    hash = &Hash::getInstance();

    setNthread(1);//TODO 1

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

void SearchManager::search(const int mply) {
    if (getNthread() > 1 && mply > 3) {//TODO
        lazySMP(mply);
    } else {
        singleSearch(mply);
    }
}

void SearchManager::singleSearch(const int mply) {
    debug("start singleSearch -------------------------------");
    lineWin.cmove = -1;
    setMainPly(mply);
    ASSERT(!getBitCount());
    threadPool[0]->setMainParam(SMP_NO, mply);
    threadPool[0]->run();
    valWindow = threadPool[0]->getValWindow();
    if (threadPool[0]->getRunning()) {
        memcpy(&lineWin, &threadPool[0]->getPvLine(), sizeof(_TpvLine));
        for (int ii = 1; ii < getNthread(); ii++) {
            threadPool[ii]->setValWindow(valWindow);
        }
    }
    debug("end singleSearch -------------------------------");
}

void SearchManager::lazySMP(const int mply) {
    ASSERT (mply > 1);
    lineWin.cmove = -1;
    setMainPly(mply);
    ASSERT(!getBitCount());

    debug("start lazySMP --------------------------");

    for (int ii = 0; ii < getNthread(); ii++) {
        Search &idThread1 = getNextThread();
        idThread1.setRunning(1);
        startThread(SMP_YES, idThread1, mply + (ii % 2));
    }
    joinAll();
    debug("end lazySMP ---------------------------");

    ASSERT(!getBitCount());
    if (lineWin.cmove <= 0) {
        singleSearch(mply);
    }
}

void SearchManager::receiveObserverSearch(const int threadID) {
    ASSERT(getNthread() > 1);
    spinlockSearch.lock();
    INC(checkSmp1);

    if (getRunning(threadID) && lineWin.cmove == -1) {
        stopAllThread();
        memcpy(&lineWin, &threadPool[threadID]->getPvLine(), sizeof(_TpvLine));
        mateIn = threadPool[threadID]->getMateIn();
        ASSERT(mateIn == INT_MAX);

        debug("win", threadID);
        ASSERT(lineWin.cmove);
    }
    ADD(checkSmp1, -1);
    ASSERT(!checkSmp1);
    spinlockSearch.unlock();
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

int SearchManager::loadFen(string fen) {
    int res = threadPool[0]->loadFen(fen);

    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < threadPool.size(); i++) {
        threadPool[i]->setChessboard(threadPool[0]->getChessboard());
    }
    return res;
}

void SearchManager::startThread(const bool smpMode, Search &thread, const int depth) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " isrunning: ", getRunning(thread.getId()));

    thread.setMainParam(smpMode, depth);

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
    u64 i = 0;
    for (Search *s:threadPool) {
        i += s->getTotMoves();
    }
    return i;
}

void SearchManager::incKillerHeuristic(int from, int to, int value) {
    for (Search *s:threadPool) {
        s->incKillerHeuristic(from, to, value);
    }
}

int SearchManager::getHashSize() {
    return hash->getHashSize();
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

int SearchManager::getScore(int side, const bool trace) {
    int N_PIECE = 0;
#ifdef DEBUG_MODE
    N_PIECE = Bits::bitCount(threadPool[0]->getBitBoard<WHITE>() | threadPool[0]->getBitBoard<BLACK>());
#endif
    return threadPool[0]->getScore(side, N_PIECE, -_INFINITE, _INFINITE, trace);
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

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) {
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
    if (!ThreadPool::setNthread(nthread))return false;
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
