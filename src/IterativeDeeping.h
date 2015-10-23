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
#include "OpenBook.h"

class IterativeDeeping : public Thread {

public:
    mutex commandMutex;

    virtual ~ IterativeDeeping();

    virtual void run();

    void endRun() { };

    bool getPonderEnabled();

    bool getGtbAvailable();

    bool getUseBook();

    void setUseBook(bool);

    void enablePonder(bool);

    void setMaxDepth(int);

    void loadBook(const string &);

    bool setParameter(String param, int value);

    IterativeDeeping();

private:

#ifdef DEBUG_MODE
    //for staistics
    Hash &hash = Hash::getInstance();
#endif
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    int maxDepth;

    bool useBook;
    Tablebase *tablebase = nullptr;
    OpenBook *openBook = nullptr;
    bool ponderEnabled;
};

