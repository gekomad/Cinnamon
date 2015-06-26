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

#ifndef SEARCH_H_
#define SEARCH_H_

#include <sys/timeb.h>
#include "Hash.h"
#include "Eval.h"
#include "namespaces.h"
#include <climits>
#include "Tablebase.h"
#include "Thread.h"


class Search : public Eval, public Thread {

public:
    // int finished;
    Search();

    virtual ~Search();

    void setRunning(int);

    void setPonder(bool);

    void setNullMove(bool);

    void setMaxTimeMillsec(int);

    int getMaxTimeMillsec();

    void startClock();

    int getRunning();

    void createGtb();

    void deleteGtb();

    void search(int depth, int alpha, int beta, _TpvLine *pline, int *mateIn, int threadID1);

    int getValue() {
        return threadValue;
    }

    void registerObservers(function<void(void)> f) {
        observers.push_back(f);
    }

    virtual void run();

    Tablebase &getGtb() const;

    void setMainPly(int);

    bool getGtbAvailable();

    STATIC_CONST int NULLMOVE_DEPTH = 3;
    STATIC_CONST int NULLMOVES_MIN_PIECE = 3;
    STATIC_CONST int NULLMOVES_R1 = 2;    //TODO 1 da CLOP
    STATIC_CONST int NULLMOVES_R2 = 3;
    STATIC_CONST int NULLMOVES_R3 = 2;
    STATIC_CONST int NULLMOVES_R4 = 2;

    void clearAge() {
        hash.clearAge();
    }
    int getHashSize() {
        return hash.getHashSize();
    }
    bool setHashSize(int i) {
        return hash.setHashSize(i);
    }
    void clearHash() {
         hash.clearHash();
    }


protected:

#ifdef DEBUG_MODE
    unsigned cumulativeMovesCount, totGen;
#endif
private:
    Hash &hash;
    Tablebase *gtb = nullptr;
    bool ponder;
    int threadDepth;
    int threadValue = 1;

    int threadAlpha;
    int threadBeta, threadID;
    _TpvLine *threadPline;
    int *threadMateIn;

    int checkTime();

    int mainDepth, maxTimeMillsec;
    bool nullSearch;
    struct timeb startTime;

    bool checkDraw(u64);

    template<int side>
    int search(int depth, int alpha, int beta, _TpvLine *pline, int, int *mateIn);

    bool checkInsufficientMaterial(int);

    void sortHashMoves(int listId, Hash::_Thash *);

    template<int side>
    int quiescence(int alpha, int beta, const char promotionPiece, int, int depth);

    void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);

    vector<function<void(void)>> observers;

    void notifyObservers(void);


};

#endif
