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

#pragma once

#include <cstring>
#include <string.h>
#include "namespaces/String.h"
#include "SearchManager.h"
#include "threadPool/Thread.h"
#include <stdio.h>
#include <stdlib.h>
#include "unistd.h"
#include <iomanip>

class IterativeDeeping : public Thread<IterativeDeeping> {

public:
    int plyFromRoot;

    IterativeDeeping();

    virtual ~ IterativeDeeping();

    void run();

    void endRun() {};

    bool getPonderEnabled() const;

    void enablePonder(const bool);

    void setMaxDepth(const int);

    int loadFen(const string &fen = "");

    long getRunning() const {
        return running;
    }

#ifdef JS_MODE
    string go() {
        run();
        return bestmove;
    }
#endif

#if defined(FULL_TEST)
    const string &getBestmove() const {
        return bestmove;
    }
#endif

private:

    DEBUG(atomic_int checkSmp2)

    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    int maxDepth;
    string bestmove;
    Hash &hash = Hash::getInstance();
    volatile long running;
    bool ponderEnabled;
#ifdef DEBUG_MODE

    bool verifyPV(string pv) {

        std::string delimiter = " ";

        size_t pos;
        std::string token;
        while ((pos = pv.find(delimiter)) != std::string::npos) {
            token = pv.substr(0, pos);
            //std::cout << token << std::endl;
            _Tmove move;
            int x = !searchManager.getMoveFromSan(token, &move);
            searchManager.setSide(x);
            searchManager.makemove(&move);
            pv.erase(0, pos + delimiter.length());
        }

        return true;
    }

#endif
};

