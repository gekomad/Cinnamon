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

#include <iostream>
#include "Times.h"
#include <map>

#ifdef BENCH_MODE
#define BENCH_AUTO_CLOSE(name)  (Bench(Times::getInstance(),name));
#define BENCH_START(name)  (Bench(Times::getInstance(),name));
#define BENCH_SUBPROCESS(name,sub)  (Times::getInstance().subProcess(name,sub));
#define BENCH_STOP(name)  (Times::getInstance().stop(name));
#define BENCH_PRINT()  (Times::getInstance().print());
#else
#define BENCH_AUTO_CLOSE(name)
#define BENCH_START(name)
#define BENCH_SUBPROCESS(name, subProcess)
#define BENCH_STOP(name)
#define BENCH_PRINT()
#endif

using namespace std;

class Bench {
public:
    Bench(Times &time, const string &name) {
        this->time = &time;
        this->name = name;
        time.start(name);
    }

    ~Bench() {
        time->stop(name);
    }
private:
    string name;
    Times *time;
};
