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

#include <fstream>
#include <iomanip>
#include "../Search.h"
#include "../threadPool/Thread.h"
#include "../util/Timer.h"
#include "_TPerftRes.h"

class PerftThread : public Thread<PerftThread>, public GenMoves {
 public:
  void setParam(const string &fen, int from, int to, _TPerftRes *, bool is960);

  PerftThread();

  ~PerftThread() override;

  void run();

  void endRun() const;

  unsigned perft(const string &fen, int depth);

  vector<string> getSuccessorsFen(const string &fen1, int depth);

 private:
  static Spinlock spinlockPrint;
  u64 tot = 0;

  template <uchar side, bool useHash>
  u64 search(const int depthx);

  int from, to;
  _TPerftRes *tPerftRes;

  template <uchar side>
  vector<string> getSuccessorsFen(const int depthx);
};
