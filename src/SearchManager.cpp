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
#include "namespaces/board.h"

using namespace _logger;
GTB *SearchManager::gtb;
SearchManager::SearchManager() {
    //SET(checkSmp1, 0);
    threadPool = new ThreadPool<Search>();
    setNthread(1);

    IniFile iniFile("cinnamon.ini");

    while (true) {
        pair <string, string> *parameters = iniFile.get();
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

string SearchManager::probeRootTB() {
    return threadPool->getThread(0).probeRootTB();
}

void SearchManager::search(const int mply) {

    constexpr int SkipStep[64] =
        {0, 1, 2, 3, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1,
         1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3};

    debug("start singleSearch -------------------------------");
    lineWin.cmove = -1;
    setMainPly(mply);
    ASSERT(bitCount(threadPool->getBitCount()) < 2);
    debug("start lazySMP --------------------------");

    for (int ii = 1; ii < threadPool->getNthread(); ii++) {
        Search &helperThread = threadPool->getNextThread();
        if (helperThread.getId() == 0)continue;

        helperThread.setRunning(1);
        startThread(helperThread, mply + SkipStep[ii]);
    }

    debug("end lazySMP ---------------------------");
    Search &mainThread = threadPool->getThread(0);
    mainThread.setMainParam(mply);
    mainThread.run();

    mainThread.getValWindow();

    if (mainThread.getRunning()) {
        memcpy(&lineWin, &mainThread.getPvLine(), sizeof(_TpvLine));
    }
    stopAllThread();
    threadPool->joinAll();
    debug("end singleSearch -------------------------------");
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
        pvvTmp +=
            Search::decodeBoardinv(lineWin.argmove[t].type,
                                   lineWin.argmove[t].from,
                                   threadPool->getThread(0).getSide());
        if (pvvTmp.length() != 4) {
            pvvTmp += Search::decodeBoardinv(lineWin.argmove[t].type,
                                             lineWin.argmove[t].to,
                                             threadPool->getThread(0).getSide());
        }
        pvv.append(pvvTmp);
        if (t == 1) {
            ponderMove.assign(pvvTmp);
        }
        pvv.append(" ");
    }
    memcpy(&resultMove, lineWin.argmove, sizeof(_Tmove));

    return true;
}

SearchManager::~SearchManager() {
}

int SearchManager::loadFen(string fen) {
    int res = threadPool->getThread(0).loadFen(fen);

    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < threadPool->getPool().size(); i++) {
        threadPool->getThread(i).setChessboard(threadPool->getThread(0).getChessboard());
    }
    return res;
}

void SearchManager::startThread(Search &thread, const int depth) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " isrunning: ", getRunning(thread.getId()));

    thread.setMainParam(depth);

    thread.start();
}

void SearchManager::setMainPly(int r) {
    for (Search *s:threadPool->getPool()) {
        s->setMainPly(r);
    }
}

int SearchManager::getPieceAt(int side, u64 i) {
    return side == WHITE ? threadPool->getThread(0).getPieceAt<WHITE>(i)
                         : threadPool->getThread(0).getPieceAt<BLACK>(i);
}

u64 SearchManager::getTotMoves() {
    u64 i = 0;
    for (Search *s:threadPool->getPool()) {
        i += s->getTotMoves();
    }
    return i;
}

void SearchManager::incHistoryHeuristic(int from, int to, int value) {
    for (Search *s:threadPool->getPool()) {
        s->incHistoryHeuristic(from, to, value);
    }
}

int SearchManager::getHashSize() {
    return hash.getHashSize();
}

void SearchManager::startClock() {
    threadPool->getThread(0).startClock();// static variable
}

string SearchManager::boardToFen() {
    return threadPool->getThread(0).boardToFen();
}

void SearchManager::clearHistoryHeuristic() {
    for (Search *s:threadPool->getPool()) {
        s->clearHistoryHeuristic();
    }
}

void SearchManager::clearAge() {
    hash.clearAge();
}

int SearchManager::getForceCheck() {
    return threadPool->getThread(0).getForceCheck();
}

u64 SearchManager::getZobristKey(int id) {
    return threadPool->getThread(id).getZobristKey();
}

void SearchManager::setForceCheck(bool a) {
    threadPool->getThread(0).setForceCheck(a);
}

void SearchManager::setRunningThread(bool r) {
    threadPool->getThread(0).setRunningThread(r);
}

void SearchManager::setRunning(int i) {
    for (Search *s:threadPool->getPool()) {
        s->setRunning(i);
    }
}

int SearchManager::getRunning(int i) {
    return threadPool->getThread(i).getRunning();
}

void SearchManager::display() {
    threadPool->getThread(0).display();
}

string SearchManager::getFen() {
    return threadPool->getThread(0).getFen();
}

void SearchManager::setHashSize(int s) {
    hash.setHashSize(s);
}

void SearchManager::setMaxTimeMillsec(int i) {
    for (Search *s:threadPool->getPool()) {
        s->setMaxTimeMillsec(i);
    }
}

void SearchManager::unsetSearchMoves() {
    for (Search *s:threadPool->getPool()) {
        s->unsetSearchMoves();
    }
}

void SearchManager::setSearchMoves(vector <string> &searchMov) {
    _Tmove move;
    vector<int> searchMoves;
    for (std::vector<string>::iterator it = searchMov.begin(); it != searchMov.end(); ++it) {
        getMoveFromSan(*it, &move);
        const int x = move.to | (int) (move.from << 8);
        searchMoves.push_back(x);
    }
    for (Search *s:threadPool->getPool()) {
        s->setSearchMoves(searchMoves);
    }
}

void SearchManager::setPonder(bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setPonder(i);
    }
}

int SearchManager::getSide() {
#ifdef DEBUG_MODE
    int t = threadPool->getThread(0).getSide();
    for (Search *s:threadPool->getPool()) {
        ASSERT(s->getSide() == t);
    }
#endif
    return threadPool->getThread(0).getSide();
}

int SearchManager::getScore(int side, const bool trace) {
    int N_PIECE = 0;
#ifdef DEBUG_MODE
    N_PIECE = bitCount(threadPool->getThread(0).getBitmap<WHITE>() | threadPool->getThread(0).getBitmap<BLACK>());
#endif
    return threadPool->getThread(0).getScore(0xffffffffffffffffULL, side, N_PIECE, -_INFINITE, _INFINITE, trace);
}

void SearchManager::clearHash() {
    hash.clearHash();
}

int SearchManager::getMaxTimeMillsec() {
    return threadPool->getThread(0).getMaxTimeMillsec();
}

void SearchManager::setNullMove(bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setNullMove(i);
    }
}

bool SearchManager::makemove(_Tmove *i) {
    bool b = false;
    for (Search *s:threadPool->getPool()) {
        b = s->makemove(i);
    }
    return b;
}

void SearchManager::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    for (Search *s:threadPool->getPool()) {
        s->takeback(move, oldkey, rep);
    }
}

void SearchManager::setSide(bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setSide(i);
    }
}
#ifndef JS_MODE

void SearchManager::printDtmGtb() {
    threadPool->getThread(0).printDtmGtb();
}

void SearchManager::deleteGtb() {
    gtb = nullptr;
}

GTB &SearchManager::createGtb() {
    gtb = &GTB::getInstance();
    return *gtb;
}

#endif

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) {
#ifdef DEBUG_MODE
    int t = threadPool->getThread(0).getMoveFromSan(string, ptr);
    for (Search *s:threadPool->getPool()) {
        ASSERT(s->getMoveFromSan(string, ptr) == t);
    }
#endif
    return threadPool->getThread(0).getMoveFromSan(string, ptr);
}

void SearchManager::pushStackMove() {
    for (Search *s:threadPool->getPool()) {
        s->pushStackMove();
    }
}

void SearchManager::init() {
    for (Search *s:threadPool->getPool()) {
        s->init();
    }
}

void SearchManager::setRepetitionMapCount(int i) {
    for (Search *s:threadPool->getPool()) {
        s->setRepetitionMapCount(i);
    }
}


bool SearchManager::setNthread(int nthread) {
    if (!threadPool->setNthread(nthread))return false;
    for (Search *s:threadPool->getPool()) {
        s->setHash(&hash);
    }
    return true;
}

void SearchManager::stopAllThread() {
    threadPool->getThread(0).setRunningThread(false);//is static
}

bool SearchManager::setParameter(String param, int value) {
    bool b = false;
    for (Search *s:threadPool->getPool()) {
        b = s->setParameter(param, value);
    }
    return b;
}



