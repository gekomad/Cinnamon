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

#include <sys/timeb.h>
#include <unistd.h>
#include "Hash.h"
#include "Eval.h"
#include "namespaces/def.h"
#include <climits>
#include "threadPool/Thread.h"
#include "db/GTB.h"
#include "db/syzygy/SYZYGY.h"

class Search : public Eval, public Thread<Search>, public Hash {

public:

    typedef struct {
        _ThashData phasheType[2];
    } _TcheckHash;

    Search();

    Search(const Search *s) { clone(s); }

    void clone(const Search *);

    virtual ~Search();

    void setRunning(int);

    void setPonder(bool);

    void setNullMove(bool);

    void setMaxTimeMillsec(int);

    bool setParameter(String param, int value);

    int getMaxTimeMillsec();

    void startClock();

    int getRunning();

    void deleteGtb();

    _TpvLine &getPvLine() {
        return pvLine;
    }

    void setMainParam(const bool smp, const int depth);

    int search(bool smp, int depth, int alpha, int beta);

    void run();

    void endRun();

    int printDtm();

    GTB &getGtb() const;

    void setMainPly(int);

    bool getGtbAvailable();

    bool getSYZYGYAvailable() const;

    string getSYZYGYbestmove(const int side);

    int getSYZYGYdtm(const int side);

    STATIC_CONST int NULLMOVE_DEPTH = 3;
    STATIC_CONST int NULLMOVES_MIN_PIECE = 3;
    STATIC_CONST int NULLMOVES_R1 = 2;
    STATIC_CONST int NULLMOVES_R2 = 3;
    STATIC_CONST int NULLMOVES_R3 = 2;
    STATIC_CONST int NULLMOVES_R4 = 2;
    STATIC_CONST int VAL_WINDOW = 50;

    void setRunningThread(bool t) {
        runningThread = t;
    }

    bool getRunningThread() const {
        return runningThread;
    }

    void setGtb(GTB &tablebase);

    void setValWindow(int valWin) {
        Search::valWindow = valWin;
    }

    int getValWindow() const {
        return valWindow;
    }

    void setChessboard(_Tchessboard &);

    _Tchessboard &getChessboard();

    u64 getZobristKey();

    int getMateIn();

#ifdef DEBUG_MODE
    unsigned cumulativeMovesCount;
    unsigned totGen;

#endif

    void setSYZYGY(SYZYGY &syzygy);

private:


    int valWindow = INT_MAX;
    static bool runningThread;
    _TpvLine pvLine;
    static GTB *gtb;
    static SYZYGY *syzygy;
    bool ponder;

    void aspirationWindow(const int depth, const int valWindow);

    int checkTime();

    int maxTimeMillsec = 5000;
    bool nullSearch;
    static high_resolution_clock::time_point startTime;

    bool checkDraw(u64);

    template<int side, bool smp>
    int search(int depth, int alpha, int beta, _TpvLine *pline, int N_PIECE, int *mateIn);

    bool checkInsufficientMaterial(int);

    void sortFromHash(const int listId, const Hash::_ThashData &phashe);

    template<int side, bool smp>
    int quiescence(int alpha, int beta, const char promotionPiece, int, int depth);

    void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);
    int getDtm(const int side, _TpvLine *pline, const int depth, const int nPieces) const;
    int mainMateIn;
    int mainDepth;
    bool mainSmp;
    int mainBeta;
    int mainAlpha;

    inline int checkHash(const int type, const bool quies, const int alpha, const int beta, const int depth,
                         const u64 zobristKeyR,
                         _TcheckHash &checkHashStruct) {

        _ThashData *phashe = &checkHashStruct.phasheType[type];
        if (phashe->dataU = readHash(type, zobristKeyR)) {
            if (phashe->dataS.depth >= depth) {
                INC(probeHash);
                if (!currentPly) {
                    if (phashe->dataS.flags == Hash::hashfBETA) {
                        incKillerHeuristic(phashe->dataS.from, phashe->dataS.to, 1);
                    }
                } else {
                    switch (phashe->dataS.flags) {
                        case Hash::hashfEXACT:
                            if (phashe->dataS.score >= beta) {
                                INC(n_cut_hashB);
                                return beta;
                            }
                            break;
                        case Hash::hashfBETA:
                            if (!quies)incKillerHeuristic(phashe->dataS.from, phashe->dataS.to, 1);
                            if (phashe->dataS.score >= beta) {
                                INC(n_cut_hashB);
                                return beta;
                            }
                            break;
                        case Hash::hashfALPHA:
                            if (phashe->dataS.score <= alpha) {
                                INC(n_cut_hashA);
                                return alpha;
                            }
                            break;
                        default:
                            fatal("error checkHash");
                            break;
                    }
                }
            }
        }
        INC(cutFailed);
        return INT_MAX;

    }
};


