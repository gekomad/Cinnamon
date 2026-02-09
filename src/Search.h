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

#include "Eval.h"
#include "Hash.h"
#include "db/TB.h"
#include "threadPool/Thread.h"
#include "unistd.h"
#include <climits>

typedef struct {
    int cmove;
    _Tmove argmove[MAX_PLY];
} _TpvLine;

class Search : public GenMoves, public Thread<Search> {
  public:
    static constexpr int NULL_DIVISOR = 7;
    static constexpr int NULL_DEPTH = 3;
    static constexpr int VAL_WINDOW = 40;
    static constexpr int REVERSE_FUTIL_MARGIN = 180;
    static constexpr int EXT_FUTIL_MARGIN = 550;
    Eval eval;
#ifndef JS_MODE
    SYZYGY *syzygy = &SYZYGY::getInstance();
#endif
    Search();

    short getScore(const uchar side) {
        return eval.getScore(chessboard, 0, side, -_INFINITE, _INFINITE DEBUG2(, true));
    }

    explicit Search(const Search *s) {
        clone(s);
    }

    void clone(const Search *);

    ~Search() override;

    void setRunning(int) override;

    void setPonder(bool);

    void setNullMove(bool);

    void setMaxTimeMillsec(int);

#ifdef TUNING

    int qSearch(const int depth, const int alpha, const int beta) {
        ASSERT_RANGE(depth, 0, MAX_PLY);
        auto ep = enPassant;

        const auto result = sideToMove ? qsearch<WHITE>(alpha, beta, ep, depth) : qsearch<BLACK>(alpha, beta, ep, depth);
        return sideToMove ? result : -result;
    }

#endif

    int getMaxTimeMillsec() const;

    static void startClock();

    int getRunning() const override;

    const _TpvLine &getPvLine() const {
        return pvLine;
    }

    void setMainParam(int depth);
    void run();
    void endRun() {
    }
    void setMainPly(int, int);

    static void setRunningThread(const bool t) {
        runningThread = t;
    }

    int getValWindow() const {
        return valWindow;
    }
    u64 getZobristKey() const;
    uchar getEnpassant() const {
        return enPassant;
    }

#ifndef NDEBUG
    static unsigned cumulativeMovesCount;
    unsigned totGen;

    unsigned getLazyEvalCuts() const {
        return eval.lazyEvalCuts;
    }

#endif
    void unsetSearchMoves();
    void setSearchMoves(const vector<int> &v);

  private:
    Hash &hash = Hash::getInstance();

    vector<int> searchMovesVector;
    int valWindow = INT_MAX;
    static volatile bool runningThread;
    _TpvLine pvLine;

    bool ponder;

#ifdef BENCH_MODE
    Times *times = &Times::getInstance();
#endif

    template <uchar side, bool searchMoves> void aspirationWindow(const int depth, const int valWindow);

    int checkTime() const;

    int maxTimeMillsec = 5000;
    bool nullSearch;
    static high_resolution_clock::time_point startTime;

    bool checkDraw(u64) const;

    template <uchar side, bool checkMoves> int search(const int depth, int alpha, int beta, _TpvLine *pline, const int N_PIECE);

    template <bool checkMoves> bool checkSearchMoves(const _Tmove *move) const;

    template <uchar side> int qsearch(int alpha, const int beta, const uchar promotionPiece, const int depth);

    static void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);

    int mainDepth;
    int ply;

    template <uchar side> bool badCapure(const _Tmove &move, const u64 allpieces) const;
};
