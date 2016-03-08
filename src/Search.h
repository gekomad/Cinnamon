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
#include "Hash.h"
#include "Eval.h"
#include "namespaces/def.h"
#include <climits>
#include "threadPool/Thread.h"

#ifdef JS_MODE
#include "js/Tablebase.h"
#else

#include "Tablebase.h"

#endif


class Search : public Eval, public Thread<Search> {

public:

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

    Tablebase &getGtb() const;

    void setMainPly(int);

    bool getGtbAvailable();

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

    void setGtb(Tablebase &tablebase);

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
private:

    typedef struct {
        int res;
        bool hashFlag[2];
        Hash::_Thash phasheType[2];
        Hash::_Thash *rootHash[2];
    } _TcheckHash;

    int valWindow = INT_MAX;
    static bool runningThread;
    _TpvLine pvLine;
    static Hash *hash;
    static Tablebase *gtb;
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

    void sortHashMoves(int listId, Hash::_Thash &);

    template<int side, bool smp>
    int quiescence(int alpha, int beta, const char promotionPiece, int, int depth);

    void updatePv(_TpvLine *pline, const _TpvLine *line, const _Tmove *move);

    int mainMateIn;
    int mainDepth;
    bool mainSmp;
    int mainBeta;
    int mainAlpha;

    template<bool type, bool smp>
    FORCEINLINE bool checkHash(const bool quies, const int alpha, const int beta, const int depth, const u64 zobristKeyR, _TcheckHash &checkHashStruct) {
        Hash::_Thash *phashe;

        checkHashStruct.hashFlag[type] = false;
        phashe = &checkHashStruct.phasheType[type];


        if (hash->readHash<smp, type>(checkHashStruct.rootHash, zobristKeyR, phashe)) {
            if (phashe->from != phashe->to && phashe->flags & 0x3) {    // hashfEXACT or hashfBETA
                checkHashStruct.hashFlag[type] = true;
            }
            if (phashe->depth >= depth) {
                INC(hash->probeHash);
                if (!currentPly) {
                    if (phashe->flags == Hash::hashfBETA) {
                        incKillerHeuristic(phashe->from, phashe->to, 1);
                    }
                } else {
                    switch (phashe->flags) {
                        case Hash::hashfEXACT:
                            if (phashe->score >= beta) {
                                INC(hash->n_cut_hashB);
                                checkHashStruct.res = beta;
                                return true;
                            }
                            break;
                        case Hash::hashfBETA:
                            if (!quies)incKillerHeuristic(phashe->from, phashe->to, 1);
                            if (phashe->score >= beta) {
                                INC(hash->n_cut_hashB);
                                checkHashStruct.res = beta;
                                return true;
                            }
                            break;
                        case Hash::hashfALPHA:
                            if (phashe->score <= alpha) {
                                INC(hash->n_cut_hashA);
                                checkHashStruct.res = alpha;
                                return true;
                            }
                            break;
                        default:
                            break;
                    }
                    INC(hash->cutFailed);
                }
                INC(hash->cutFailed);
            }
            INC(hash->cutFailed);
        }
        return false;
    }


};


