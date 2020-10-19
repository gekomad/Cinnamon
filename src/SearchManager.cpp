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

SearchManager::SearchManager() {

    threadPool = new ThreadPool<Search>(1);

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

#if defined(FULL_TEST)
unsigned SearchManager::SZtbProbeWDL() const {
    return threadPool->getThread(0).SZtbProbeWDL();
}
#endif

string SearchManager::probeRootTB() const {
    _Tmove bestMove;
    if (threadPool->getThread(0).probeRootTB(&bestMove)) {
        string best = string(board::decodeBoardinv(bestMove.s.type, bestMove.s.from, getSide())) +
                      string(board::decodeBoardinv(bestMove.s.type, bestMove.s.to, getSide()));

        if (bestMove.s.promotionPiece != GenMoves::NO_PROMOTION)
            best += tolower(bestMove.s.promotionPiece);

        return best;
    } else
        return "";
}

int SearchManager::search(const int mply) {

    constexpr int SkipStep[64] =
            {0, 1, 2, 3, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2, 1, 1, 2, 3, 0, 1, 1, 2,
             1,
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

    auto res = mainThread.getValWindow();

    if (mainThread.getRunning()) {
        memcpy(&lineWin, &mainThread.getPvLine(), sizeof(_TpvLine));
    }
    stopAllThread();
    threadPool->joinAll();
    debug("end singleSearch -------------------------------");
    return res;
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
                board::decodeBoardinv(lineWin.argmove[t].s.type,
                                      lineWin.argmove[t].s.from,
                                      board::getSide(threadPool->getThread(0).getChessboard()));
        if (pvvTmp.length() != 4) {
            pvvTmp += board::decodeBoardinv(lineWin.argmove[t].s.type,
                                            lineWin.argmove[t].s.to,
                                            board::getSide(threadPool->getThread(0).getChessboard()));
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
    int res = -1;
    for (uchar i = 0; i < threadPool->getPool().size(); i++) {
        res = threadPool->getThread(i).loadFen(fen);
        ASSERT_RANGE(res, 0, 1);
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
    return side == WHITE ? board::getPieceAt<WHITE>(i, threadPool->getThread(0).getChessboard())
                         : board::getPieceAt<BLACK>(i, threadPool->getThread(0).getChessboard());
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

void SearchManager::startClock() {
    threadPool->getThread(0).startClock();// static variable
}

string SearchManager::boardToFen() {
    return board::boardToFen(threadPool->getThread(0).getChessboard());
}

void SearchManager::clearHeuristic() {
    for (Search *s:threadPool->getPool()) {
        s->clearHeuristic();
    }
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
    board::display(threadPool->getThread(0).getChessboard());
}

string SearchManager::getFen() {
    return threadPool->getThread(0).getFen();
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

void SearchManager::setSearchMoves(vector<string> &searchMov) {
    _Tmove move;
    vector<int> searchMoves;
    for (std::vector<string>::iterator it = searchMov.begin(); it != searchMov.end(); ++it) {
        getMoveFromSan(*it, &move);
        const int x = move.s.to | (int) (move.s.from << 8);
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

int SearchManager::getSide() const {
#ifdef DEBUG_MODE
    int t = board::getSide(threadPool->getThread(0).getChessboard());
    for (Search *s:threadPool->getPool()) {
        ASSERT(board::getSide(s->getChessboard()) == t)
    }
#endif
    return board::getSide(threadPool->getThread(0).getChessboard());
}

int SearchManager::getScore(int side, const bool trace) {
    return threadPool->getThread(0).getScore(0xffffffffffffffffULL, side, -_INFINITE, _INFINITE, trace);
}

int SearchManager::getMaxTimeMillsec() {
    return threadPool->getThread(0).getMaxTimeMillsec();
}

void SearchManager::setNullMove(bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setNullMove(i);
    }
}

void SearchManager::setChess960(bool i) {
    for (Search *s:threadPool->getPool()) {
        s->setChess960(i);
    }
}

bool SearchManager::makemove(_Tmove *i) {
    bool b = false;
    for (Search *s:threadPool->getPool()) {
        b = s->makemove(i, true, false);
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

int SearchManager::printDtmGtb(const bool dtm) {
    return threadPool->getThread(0).printDtmWdlGtb(dtm);
}

void SearchManager::printDtmSyzygy() {
    threadPool->getThread(0).printDtzSyzygy();
}

void SearchManager::printWdlSyzygy() {
    threadPool->getThread(0).printWdlSyzygy();
}

#endif

int SearchManager::getMoveFromSan(String string, _Tmove *ptr) const {
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




