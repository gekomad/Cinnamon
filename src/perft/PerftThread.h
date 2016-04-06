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

#include "_TPerftRes.h"
#include "../Search.h"
#include "../threadPool/Thread.h"
#include <iomanip>
#include <atomic>
#include <fstream>
#include <unistd.h>
#include "../util/Timer.h"

class PerftThread : public Thread<PerftThread>, public GenMoves {
public:

    void setParam(const string &fen, const int from, const int to, _TPerftRes *);

    PerftThread();

    virtual ~PerftThread();

    void run();

    void endRun();

    u64 getPartial();

    unsigned perft(const string &fen, const int depth);

    vector<string> getSuccessorsFen(const string &fen1, const int depth);

private:

    static Spinlock SPINLOCK_HASH;

    static Spinlock spinlockPrint;
    u64 tot = 0;

    template<int side, bool useHash, bool smp>
    u64 search(const int depthx);

    int from, to;
    _TPerftRes *tPerftRes;
    u64 partialTot = 0;

    template<int side>
    vector<string> getSuccessorsFen(const int depthx);
};


