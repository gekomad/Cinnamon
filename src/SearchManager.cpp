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

SearchManager::SearchManager() {
    SET(checkSmp1, 0);

    setNthread(1);

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

string SearchManager::probeRootTB() {
    return getThread(0).probeRootTB();
}

void SearchManager::search(const int mply) {

    if (getNthread() > 1 && mply > 3) {
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
    getThread(0).setMainParam(mply);
    getThread(0).run();
    valWindow = getThread(0).getValWindow();
    if (getThread(0).getRunning()) {
        memcpy(&lineWin, &getThread(0).getPvLine(), sizeof(_TpvLine));
        for (int ii = 1; ii < getNthread(); ii++) {
            getThread(ii).setValWindow(valWindow);
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
        startThread(idThread1, mply + (ii % 2));
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
        memcpy(&lineWin, &getThread(threadID).getPvLine(), sizeof(_TpvLine));
        mateIn = getThread(threadID).getMateIn();
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
        pvvTmp += Search::decodeBoardinv(lineWin.argmove[t].type, lineWin.argmove[t].from, getThread(0).getSide());
        if (pvvTmp.length() != 4) {
            pvvTmp += Search::decodeBoardinv(lineWin.argmove[t].type, lineWin.argmove[t].to, getThread(0).getSide());
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
    int res = getThread(0).loadFen(fen);

    ASSERT_RANGE(res, 0, 1);
    for (uchar i = 1; i < getPool().size(); i++) {
        getThread(i).setChessboard(getThread(0).getChessboard());
    }
    return res;
}

void SearchManager::startThread(Search &thread, const int depth) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " isrunning: ", getRunning(thread.getId()));

    thread.setMainParam(depth);

    thread.start();
}

void SearchManager::setMainPly(int r) {
    for (Search *s:getPool()) {
        s->setMainPly(r);
    }
}

int SearchManager::getPieceAt(int side, u64 i) {
    return side == WHITE ? getThread(0).getPieceAt<WHITE>(i) : getThread(0).getPieceAt<BLACK>(i);
}

u64 SearchManager::getTotMoves() {
    u64 i = 0;
    for (Search *s:getPool()) {
        i += s->getTotMoves();
    }
    return i;
}

void SearchManager::incKillerHeuristic(int from, int to, int value) {
    for (Search *s:getPool()) {
        s->incKillerHeuristic(from, to, value);
    }
}

int SearchManager::getHashSize() {
    return getThread(0).getHashSize();
}

void SearchManager::startClock() {
    getThread(0).startClock();// static variable
}

string SearchManager::boardToFen() {
    return getThread(0).boardToFen();
}

void SearchManager::clearKillerHeuristic() {
    for (Search *s:getPool()) {
        s->clearKillerHeuristic();
    }
}

void SearchManager::clearAge() {
    getThread(0).clearAge();
}

int SearchManager::getForceCheck() {
    return getThread(0).getForceCheck();
}

u64 SearchManager::getZobristKey(int id) {
    return getThread(id).getZobristKey();
}

void SearchManager::setForceCheck(bool a) {
    getThread(0).setForceCheck(a);
}

void SearchManager::setRunningThread(bool r) {
    getThread(0).setRunningThread(r);
}

void SearchManager::setRunning(int i) {
    for (Search *s:getPool()) {
        s->setRunning(i);
    }
}

int SearchManager::getRunning(int i) {
    return getThread(i).getRunning();
}

void SearchManager::display() {
    getThread(0).display();
}

string SearchManager::getFen() {
    return getThread(0).getFen();
}

void SearchManager::setHashSize(int s) {
    getThread(0).setHashSize(s);
}

void SearchManager::setMaxTimeMillsec(int i) {
    for (Search *s:getPool()) {
        s->setMaxTimeMillsec(i);
    }
}

void SearchManager::unsetSearchMoves() {
    for (Search *s:getPool()) {
        s->unsetSearchMoves();
    }
}

void SearchManager::setSearchMoves(vector<string> &searchMov) {
    _Tmove move;
    vector<int> searchMoves;
    for (std::vector<string>::iterator it = searchMov.begin(); it != searchMov.end(); ++it) {
        getMoveFromSan(*it, &move);
        const int x = move.to | (int) (move.from << 8);
        searchMoves.push_back(x);
    }
    for (Search *s:getPool()) {
        s->setSearchMoves(searchMoves);
    }
}

void SearchManager::setPonder(bool i) {
    for (Search *s:getPool()) {
        s->setPonder(i);
    }
}

int SearchManager::getSide() {
#ifdef DEBUG_MODE
    int t = getThread(0).getSide();
    for (Search *s:getPool()) {
        ASSERT(s->getSide() == t);
    }
#endif
    return getThread(0).getSide();
}

int SearchManager::getScore(int side, const bool trace) {
    int N_PIECE = 0;
#ifdef DEBUG_MODE
    N_PIECE = bitCount(getThread(0).getBitmap<WHITE>() | getThread(0).getBitmap<BLACK>());
#endif
    return getThread(0).getScore(0xffffffffffffffffULL, side, N_PIECE, -_INFINITE, _INFINITE, trace);
}

void SearchManager::clearHash() {
    getThread(0).clearHash();
}

int SearchManager::getMaxTimeMillsec() {
    return getThread(0).getMaxTimeMillsec();
}

void SearchManager::setNullMove(bool i) {
    for (Search *s:getPool()) {
        s->setNullMove(i);
    }
}

bool SearchManager::makemove(_Tmove *i) {
    bool b = false;
    for (Search *s:getPool()) {
        b = s->makemove(i);
    }
    return b;
}

void SearchManager::takeback(_Tmove *move, const u64 oldkey, bool rep) {
    for (Search *s:getPool()) {
        s->takeback(move, oldkey, rep);
    }
}

void SearchManager::setSide(bool i) {
    for (Search *s:getPool()) {
        s->setSide(i);
    }
}

bool SearchManager::getGtbAvailable() const {
    return getThread(0).getGtbAvailable();
}

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) {
#ifdef DEBUG_MODE
    int t = getThread(0).getMoveFromSan(string, ptr);
    for (Search *s:getPool()) {
        ASSERT(s->getMoveFromSan(string, ptr) == t);
    }
#endif
    return getThread(0).getMoveFromSan(string, ptr);
}

GTB &SearchManager::getGtb() const {
    return getThread(0).getGtb();
}

void SearchManager::printDtmGtb() {
    getThread(0).printDtmGtb();
}

void SearchManager::setGtb(GTB &tablebase) {
    for (Search *s:getPool()) {
        s->setGtb(tablebase);
    }
}

void SearchManager::pushStackMove() {
    for (Search *s:getPool()) {
        s->pushStackMove();
    }
}

void SearchManager::init() {
    for (Search *s:getPool()) {
        s->init();
    }
}

void SearchManager::setRepetitionMapCount(int i) {
    for (Search *s:getPool()) {
        s->setRepetitionMapCount(i);
    }
}

void SearchManager::deleteGtb() {
    for (Search *s:getPool()) {
        s->deleteGtb();
    }
}

bool SearchManager::setNthread(int nthread) {
    if (!ThreadPool::setNthread(nthread))return false;
    return true;
}

void SearchManager::stopAllThread() {
    getThread(0).setRunningThread(false);//is static
}

bool SearchManager::setParameter(String param, int value) {
    bool b = false;
    for (Search *s:getPool()) {
        b = s->setParameter(param, value);
    }
    return b;
}


GTB &SearchManager::createGtb() {
    GTB &gtb = GTB::getInstance();
    setGtb(gtb);
    return gtb;
}






