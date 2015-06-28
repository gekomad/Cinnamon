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

#include "SearchPool.h"


SearchPool::~SearchPool() {
    for (int i = 0; i < N_THREAD; i++) {
        delete searchPool[i];
        searchPool[i] = nullptr;
    }
}

bool SearchPool::getRes(_Tmove &resultMove, string &ponderMove, string &pvv) {
    if (threadWin == -1) {
        return false;
    }
    pvv.clear();
    string pvvTmp;
    //ASSERT(threadWin != -1 || threadWin == -1 && !searchPool[0]->getRunning() && !searchPool[1]->getRunning() && !searchPool[2]->getRunning() && !searchPool[3]->getRunning());
    //ASSERT_RANGE(threadWin, 0, N_THREAD - 1);
    ASSERT(line1[threadWin].cmove);
    for (int t = 0; t < line1[threadWin].cmove; t++) {
        pvvTmp.clear();
        pvvTmp += Search::decodeBoardinv(line1[threadWin].argmove[t].type, line1[threadWin].argmove[t].from, searchPool[threadWin]->getSide());
        if (pvvTmp.length() != 4) {
            pvvTmp += Search::decodeBoardinv(line1[threadWin].argmove[t].type, line1[threadWin].argmove[t].to, searchPool[threadWin]->getSide());
        }
        pvv.append(pvvTmp);
        if (t == 1) {
            ponderMove.assign(pvvTmp);
        }
        pvv.append(" ");
    };
    memcpy(&resultMove, line1[threadWin].argmove, sizeof(_Tmove));
    return true;
}

SearchPool::SearchPool() {
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
                        ASSERT(line1[0].cmove);
                        val = t;
                        threadWin = 0;
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
                        ASSERT(line1[1].cmove);
                        val = t;
                        threadWin = 1;
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
                        ASSERT(line1[2].cmove);
                        val = t;
                        threadWin = 2;
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

                    ASSERT(line1[3].cmove);
                    val = searchPool[3]->getValue();
                    threadWin = 3;
                    for (Search *s:searchPool) {
                        s->setRunningThread(0);
                    }
                }
            }
        });
    }
}
