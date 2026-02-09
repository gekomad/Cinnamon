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
#ifdef TUNING

#include "Tune.h"

class Stockfish : Tune, public Thread<Stockfish> {
  public:
    Stockfish() {
    }

    void shareParameters(double current_error, const map<string, int> &newParams) const;
    void sendParameters(double error, const map<string, int> &newParams);
    void run();
    void endRun() {
    }
    void loadEPD(const string &path);
    void init1(const std::string &path);

  private:
    double currentError;
    double bestError;
    string path;
    const string iniFile = "stockfish.ini";
};
namespace stockfishPool {
static ThreadPool<Stockfish> stockfishPool;

inline void go(const string &path) {
    stockfishPool.getThread(0).loadEPD(path);
    for (int i = 0; i < stockfishPool.getNthread(); i++) {
        Stockfish &p = stockfishPool.getNextThread();
        p.init1(path);
        p.start();
    }
    stockfishPool::stockfishPool.joinAll();
}
} // namespace stockfishPool

#endif
