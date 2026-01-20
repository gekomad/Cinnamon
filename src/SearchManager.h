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

#include "Search.h"
#include "db/TB.h"
#include "namespaces/String.h"
#include "threadPool/ThreadPool.h"
#include "util/IniFile.h"

class SearchManager {
 public:

  SearchManager(const shared_ptr<Hash> &hash) {
    this->hash = hash;
    threadPool = unique_ptr<ThreadPool<Search>>(new ThreadPool<Search>(1));
    for (Search *s : threadPool->getPool()) {
      s->hash = hash.get();
    }
  }
  bool getRes(_Tmove &resultMove, string &ponderMove, string &pvv) const;

  int loadFen(const string &fen = "") const;

  int getPieceAt(const uchar side, const u64 i) const;

  u64 getTotMoves() const;

  void incHistoryHeuristic(const int from, const int to, const int value) const;

  void startClock() const;

  Search &getSearch(const int i = 0) const { return threadPool->getThread(i); }

  string decodeBoardinv(const _Tmove *) const;

#ifdef TUNING

  const _Tchessboard &getChessboard() { return threadPool->getThread(0).chessboard; }

  void setParameter(const string &param, const int value, const int phase) {
    for (Search *s : threadPool->getPool()) {
      s->setParameter(param, value, phase);
    }
  }

  int getParameter(const string &param, const int phase) { return threadPool->getThread(0).getParameter(param, phase); }

  int getQscore() const { return threadPool->getThread(0).qSearch(15, -_INFINITE, _INFINITE); }

#endif

  void clearHeuristic() const;

  int getForceCheck() const;

  u64 getZobristKey(const int id) const;

  u64 getEnpassant(const int id) const { return threadPool->getThread(id).getEnpassant(); }

  void setForceCheck(const bool a) const;

  static void setRunningThread(const bool r);

  void setRunning(const int i) const;

  int getRunning(const int i) const;

  void display() const;

  void setMaxTimeMillsec(const int i) const;

  void unsetSearchMoves() const;

  void setSearchMoves(const vector<string> &searchmoves) const;

  void setPonder(bool i) const;

  int getSide() const;

  int getScore(const uchar side) const;

  int getMaxTimeMillsec() const;

  void setNullMove(const bool i) const;

  void setChess960(const bool i) const;

  bool makemove(const _Tmove *i) const;
  void updateFenString() const;
  void takeback(const _Tmove *move, const u64 oldkey, const uchar oldEnpassant, const bool rep) const;

  void setSide(const bool i) const;

  int getMoveFromSan(const string &string, _Tmove *ptr) const;

#ifndef JS_MODE

  int printDtmGtb(const bool dtm) const;

  void printDtmSyzygy() const;

  void printWdlSyzygy() const;

#endif

  void pushStackMove() const;

  void init() const;

  void setRepetitionMapCount(const int i) const;

  bool setNthread(const int) const;

#if defined(FULL_TEST)

  unsigned SZtbProbeWDL() const;

  u64 getBitmap(const int n, const uchar side) const {
    return side ? board::getBitmap<WHITE>(threadPool->getPool()[n]->chessboard)
                : board::getBitmap<BLACK>(threadPool->getPool()[n]->chessboard);
  }

  const _Tchessboard &getChessboard(const int n = 0) const { return threadPool->getPool()[n]->chessboard; }

  template <uchar side>
  u64 getPinned(const u64 allpieces, const u64 friends, const int kingPosition) const {
    return board::getPinned<side>(allpieces, friends, kingPosition, threadPool->getPool()[0]->chessboard);
  }

#endif

#ifdef DEBUG_MODE

  static unsigned getCumulativeMovesCount() { return Search::cumulativeMovesCount; }

  unsigned getNCutAB() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->nCutAB;
    }
    return i;
  }

  double getBetaEfficiency() const {
    double b = 0;
    unsigned count = 0;
    for (const Search *s : threadPool->getPool()) {
      b += s->betaEfficiency;
      count += s->betaEfficiencyCount;
    }
    return b / count;
  }

  unsigned getLazyEvalCuts() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->getLazyEvalCuts();
    }
    return i;
  }

  unsigned getNCutFp() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->nCutFp;
    }
    return i;
  }

  unsigned getNCutRazor() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->nCutRazor;
    }
    return i;
  }

  unsigned getTotBadCaputure() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->nCutBadCaputure;
    }
    return i;
  }

  unsigned getNullMoveCut() const {
    unsigned i = 0;
    for (const Search *s : threadPool->getPool()) {
      i += s->nNullMoveCut;
    }
    return i;
  }

#endif

  int search(int ply, int iter_depth);

 private:
  unique_ptr<ThreadPool<Search>> threadPool;
  shared_ptr<Hash> hash;
  _TpvLine lineWin;

  void setMainPly(int ply, int iter_depth) const;

  void startThread(Search &thread, int depth);

  void stopAllThread();
};
