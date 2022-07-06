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

using namespace _logger;

ThreadPool<Search> *SearchManager::threadPool;
_TpvLine SearchManager::lineWin;

SearchManager::SearchManager() {
    threadPool = new ThreadPool<Search>(1);
}

#if defined(FULL_TEST)

unsigned SearchManager::SZtbProbeWDL() const {
    return SYZYGY::getInstance().SZtbProbeWDL(threadPool->getThread(0).chessboard, threadPool->getThread(0).sideToMove);
}

#endif

int SearchManager::search(const int plyFromRoot, const int iter_depth) {

    constexpr int SkipStep[64] =
            {0, 1, 2, 3, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2,
             1,
             1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3};

    lineWin.cmove = -1;
    setMainPly(plyFromRoot, iter_depth);
    assert(bitCount(threadPool->getBitCount()) < 2);

    for (int ii = 1; ii < threadPool->getNthread(); ii++) {
        Search &helperThread = threadPool->getNextThread();
        if (helperThread.getId() == 0)continue;

        helperThread.setRunning(1);
        startThread(helperThread, iter_depth + SkipStep[ii]);
    }

    Search &mainThread = threadPool->getThread(0);
    mainThread.setMainParam(iter_depth);
    mainThread.run();

    auto res = mainThread.getValWindow();

    if (mainThread.getRunning()) {
        memcpy(&lineWin, &mainThread.getPvLine(), sizeof(lineWin));
    }
    stopAllThread();
    threadPool->joinAll();
    return res;
}

bool SearchManager::getRes(_Tmove &resultMove, string &ponderMove, string &pvv) {
    if (lineWin.cmove < 1) {
        return false;
    }

    pvv.clear();
    string pvvTmp;

    assert(lineWin.cmove);
    for (int t = 0; t < lineWin.cmove; t++) {
        pvvTmp.clear();
        pvvTmp += decodeBoardinv(&lineWin.argmove[t], threadPool->getThread(0).sideToMove);
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

int SearchManager::loadFen(const string &fen) {
    int res = -1;

    for (uchar i = 0; i < threadPool->getPool().size(); i++) {
        res = threadPool->getThread(i).loadFen(fen);
        ASSERT_RANGE(res, 0, 1)
    }
    return res;
}

void SearchManager::startThread(Search &thread, const int depth) {

    debug("startThread: ", thread.getId(), " depth: ", depth, " isrunning: ", getRunning(thread.getId()))

    thread.setMainParam(depth);

    thread.start();
}

void SearchManager::setMainPly(const int ply, const int iter_depth) {
    for (Search *s:threadPool->getPool()) {
        s->setMainPly(ply, iter_depth);
    }
}

int SearchManager::getPieceAt(const uchar side, const u64 i) {
    return side == WHITE ? board::getPieceAt<WHITE>(i, threadPool->getThread(0).chessboard)
                         : board::getPieceAt<BLACK>(i, threadPool->getThread(0).chessboard);
}

u64 SearchManager::getTotMoves() {
    u64 i = 0;
    for (Search *s:threadPool->getPool()) {
        i += s->getTotMoves();
    }
    return i;
}

void SearchManager::incHistoryHeuristic(const int from, const int to, const int value) {
    for (Search *s:threadPool->getPool()) {
        s->incHistoryHeuristic(from, to, value);
    }
}

void SearchManager::startClock() {
    threadPool->getThread(0).startClock();// static variable
}

void SearchManager::clearHeuristic() {
    for (Search *s:threadPool->getPool()) {
        s->clearHeuristic();
    }
}

int SearchManager::getForceCheck() {
    return threadPool->getThread(0).getForceCheck();
}

u64 SearchManager::getZobristKey(const int id) {
    return threadPool->getThread(id).getZobristKey();
}

void SearchManager::setForceCheck(const bool a) {
    for (Search *s:threadPool->getPool()) {
        s->setForceCheck(a);
    }
}

void SearchManager::setRunningThread(const bool r) {
    Search::setRunningThread(r);
}

void SearchManager::setRunning(const int i) {
    for (Search *s:threadPool->getPool()) {
        s->setRunning(i);
    }
}

int SearchManager::getRunning(const int i) {
    return threadPool->getThread(i).getRunning();
}

void SearchManager::display() {
    threadPool->getThread(0).display();
}

void SearchManager::setMaxTimeMillsec(const int i) {
    for (Search *s:threadPool->getPool()) {
        s->setMaxTimeMillsec(i);
    }
}

void SearchManager::unsetSearchMoves() {
    for (Search *s:threadPool->getPool()) {
        s->unsetSearchMoves();
    }
}

void SearchManager::setSearchMoves(const vector<string> &searchMov) {
    _Tmove move;
    vector<int> searchMoves;
    for (auto it = searchMov.begin(); it != searchMov.end(); ++it) {
        getMoveFromSan(*it, &move);
        const int x = move.to | (int) (move.from << 8);
        searchMoves.push_back(x);
    }
    for (Search *s:threadPool->getPool()) {
        s->setSearchMoves(searchMoves);
    }
}

void SearchManager::setPonder(const bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setPonder(i);
    }
}

int SearchManager::getSide() {
#ifndef NDEBUG
    int t = threadPool->getThread(0).sideToMove;
    for (Search *s:threadPool->getPool()) {
        assert(s->sideToMove == t);
    }
#endif
    return threadPool->getThread(0).sideToMove;
}

int SearchManager::getScore(const uchar side) {
    return threadPool->getThread(0).getScore(side);
}

int SearchManager::getMaxTimeMillsec() {
    return threadPool->getThread(0).getMaxTimeMillsec();
}

void SearchManager::setNullMove(const bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setNullMove(i);
    }
}

void SearchManager::setChess960(const bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setChess960(i);
    }
}

bool SearchManager::makemove(const _Tmove *i) {
    bool b = false;
    for (Search *s:threadPool->getPool()) {
        b = s->makemove(i, true);
    }
    return b;
}

string SearchManager::decodeBoardinv(const _Tmove *move, const uchar side) {
    return threadPool->getThread(0).decodeBoardinv(move, side);
}

void SearchManager::takeback(const _Tmove *move, const u64 oldkey, const uchar oldEnpassant, const bool rep) {
    for (Search *s:threadPool->getPool()) {
        s->takeback(move, oldkey, oldEnpassant, rep);
    }
}

void SearchManager::setSide(const bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setSide(i);
    }
}

#ifndef JS_MODE

int SearchManager::printDtmGtb(const bool dtm) {
    return TB::printDtmWdlGtb(threadPool->getThread(0), dtm);
}

void SearchManager::printDtmSyzygy() {
    TB::printDtzSyzygy(threadPool->getThread(0));
}

void SearchManager::printWdlSyzygy() {
    TB::printWdlSyzygy(threadPool->getThread(0));
}

#endif

int SearchManager::getMoveFromSan(const string &string, _Tmove *ptr) {
#ifndef NDEBUG
    int t = threadPool->getThread(0).getMoveFromSan(string, ptr);
    for (Search *s:threadPool->getPool()) {
        assert(s->getMoveFromSan(string, ptr) == t);
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

void SearchManager::setRepetitionMapCount(const int i) {
    for (Search *s:threadPool->getPool()) {
        s->setRepetitionMapCount(i);
    }
}

bool SearchManager::setNthread(const int nthread) {
    return threadPool->setNthread(nthread);
}

void SearchManager::stopAllThread() {
    Search::setRunningThread(false);
}



