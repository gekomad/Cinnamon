/*
    Cinnamon is a UCI chess engine
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

#include <sys/timeb.h>
#include "Hash.h"
#include "Eval.h"
#include "namespaces.h"
#include <climits>
#include "Tablebase.h"
#include "util/Thread.h"


class Search : public Eval, public Thread {

public:

    Search();

    void clone(const Search *);

    virtual ~Search();

    int PVSplit();

    void setRunning(int);

    void setPVSplit(const int depth, const int beta);

    void setPonder(bool);

    void setNullMove(bool);

    void setMaxTimeMillsec(int);

    int getMaxTimeMillsec();

    void startClock();

    int getRunning();

    void deleteGtb();

    _TpvLine &getPvLine() {
        return pvLine;
    }

    void search(int depth, int alpha, int beta);

    int searchNOparall(int depth, int alpha, int beta);

    int getValue() {
        ASSERT(threadValue != INT_MAX);
        return threadValue;
    }

    void registerObserversSearch(function<void(void)> f) {
        observerSearch = f;
    }

    void registerObserversPVS(function<void(void)> f) {
        observerPVS = f;
    }


    virtual void run();

    int printDtm();

    Tablebase &getGtb() const;

    void setMainPly(int);

    bool getGtbAvailable();

    STATIC_CONST int NULLMOVE_DEPTH = 3;
    STATIC_CONST int NULLMOVES_MIN_PIECE = 3;
    STATIC_CONST int NULLMOVES_R1 = 2;
    STATIC_CONST int NULLMOVES_R2 = 3;
    STATIC_CONST int NULLMOVES_R3 = 2;
    STATIC_CONST int NULLMOVES_R4 = 2;

    void clearAge() {
        hash->clearAge();
    }

    int getHashSize() {
        return hash->getHashSize();
    }

    bool setHashSize(int i) {
        return hash->setHashSize(i);
    }

    void clearHash() {
        hash->clearHash();
    }

    void setChessboard(_Tchessboard &);

    _Tchessboard &getChessboard();

    u64 getZobristKey();

    int getMateIn();

    void setGtb(Tablebase &tablebase);

#ifdef DEBUG_MODE
    unsigned cumulativeMovesCount;
    unsigned totGen;
#endif
private:

    _TpvLine pvLine;
    static Hash *hash;
    static Tablebase *gtb;
    bool ponder;
    int threadDepth;
    int threadValue = INT_MAX;

    int threadAlpha;
    int threadBeta;

    int checkTime();

    int mainDepth, maxTimeMillsec;
    bool nullSearch;
    struct timeb startTime;

    bool checkDraw(u64);

    template<int side>
    int search(int depth, int alpha, int beta, _TpvLine *pline, int N_PIECE, int *mateIn);

    bool checkInsufficientMaterial(int);

    void sortHashMoves(int listId, Hash::_Thash *);

    template<int side>
    int quiescence(int alpha, int beta, const char promotionPiece, int, int depth);

    void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);

    function<void(void)> observerSearch;

    function<void(void)> observerPVS;

    void notifyObserversSearch(void) {
        observerSearch();
    }

    void notifyObserversPVS(void) {
        observerPVS();
    }

    int mainMateIn;
    bool pvsMode = false;
    int PVSdepth;
    int PVSbeta;
};


