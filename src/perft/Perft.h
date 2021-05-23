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

#include "../Search.h"
#include <iomanip>
#include <atomic>
#include <fstream>
#include "../unistd.h"
#include "../util/Timer.h"
#include "PerftThread.h"
#include "../threadPool/ThreadPool.h"
#include "_TPerftRes.h"
#include <csignal>

class Perft : public Thread<Perft>, protected ThreadPool<PerftThread>, public Singleton<Perft> {
    friend class Singleton<Perft>;

public:
    static _ThashPerft **hash;

    void setParam(const string &fen1,
                  int depth1,
                  const int nCpu2,
                  const int mbSize1,
                  const string &dumpFile1,
                  const bool chess960);

    ~Perft();

    void dump();

    void run();

    void endRun();

    static int count;

    u64 getResult() {
        return perftRes.totMoves;
    }

private:
    Perft() : ThreadPool(1) {}

    _TPerftRes perftRes;
    Time time;
    string fen;
    string dumpFile;
    int mbSize;
    bool chess960;
    int depthHashFile;
    void alloc();

    void dealloc() const;

    bool load();

    constexpr static int minutesToDump = Time::HOUR_IN_MINUTES * 10;

    static void ctrlChandler(int s) {
        if (dumping) {
            cout << "dumping hash... " << endl << flush;
            return;
        }
        if (s < 0)cout << s;
        Perft::getInstance().dump();
        cout << "exit" << endl << endl;
        exit(0);
    }

    static bool dumping;

};

