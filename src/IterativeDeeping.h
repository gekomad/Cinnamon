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
#include "util/String.h"
#include "SearchManager.h"
#include "threadPool/Thread.h"
#include "db/OpenBook.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iomanip>

class IterativeDeeping: public Thread<IterativeDeeping> {

public:

    IterativeDeeping();

    virtual ~ IterativeDeeping();

    string go();

    void run();

    void endRun() { };

    bool getPonderEnabled() const;

    bool getGtbAvailable() const;

    bool getUseBook() const;

    void setUseBook(const bool b);

    void enablePonder(const bool);

    void setMaxDepth(const int);

    void loadBook(const string);

    bool setParameter(String param, int value);

    int loadFen(const string fen = "");

    bool setNthread(const int i);

    int getRunning() const {
        return running;
    }

    const string &getBestmove() const {
        return bestmove;
    }

private:

#ifdef DEBUG_MODE
    //for statistics

    atomic_int checkSmp2;
#endif
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    int maxDepth;
    string bestmove;

    volatile long running;
    OpenBook *openBook = nullptr;
    bool ponderEnabled;

    string getSYZYGYbestmove(const int side) {
        return searchManager.getSYZYGYbestmove(side);
    }

};

