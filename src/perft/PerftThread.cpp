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

#include "PerftThread.h"
#include "../namespaces.h"

SharedMutex PerftThread::MUTEX_BUCKET[N_MUTEX_BUCKET];
mutex PerftThread::mutexPrint;

PerftThread::PerftThread() { }

void PerftThread::setParam(string fen1, int from1, int to1, _TPerftRes *perft1) {
    perftMode = true;
    loadFen(fen1);

    this->tPerftRes = perft1;
    this->from = from1;
    this->to = to1;
}

template<int side, bool useHash, bool smp>
void PerftThread::search(_TsubRes &n_perft, const int depthx, const int isCapture) {
    checkWait();
//    _TsubRes n_perft = {0, 0};
    if (depthx == 0) {
        n_perft.totMoves = 1;
        n_perft.totCapture = isCapture;
        return;
    }
    u64 zobristKeyR;

    _ThashPerft *phashe = nullptr;

    if (useHash) {
        zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ RANDSIDE[side];
        if (smp) MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].lock_shared();
        phashe = &(tPerftRes->hash[depthx][zobristKeyR % tPerftRes->sizeAtDepth[depthx]]);
        if (zobristKeyR == phashe->key) {
            u64 res = phashe->nMoves;
            if (smp)MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].unlock_shared();
            return;
        }
        if (smp)MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].unlock_shared();
    }
    int listcount;
    _Tmove *move;
    incListId();
    u64 friends = getBitBoard<side>();
    u64 enemies = getBitBoard<side ^ 1>();
    if (generateCaptures<side>(enemies, friends)) {
        decListId();
        n_perft.totMoves = 0;
        n_perft.totCapture = 0;
        return;
    }
    generateMoves<side>(friends | enemies);
    listcount = getListSize();
    if (!listcount) {
        decListId();
        return;
    }
    for (int ii = 0; ii < listcount; ii++) {
        move = getMove(ii);
        u64 keyold = chessboard[ZOBRISTKEY_IDX];
        makemove(move, false, false);
        _TsubRes x={0,0};
        int isCapture = move->capturedPiece == SQUARE_FREE ? 0 : 1;
        search<side ^ 1, useHash, smp>(x, depthx - 1, isCapture);
        n_perft.totCapture += x.totCapture;
        n_perft.totMoves += x.totMoves;
        takeback(move, keyold, false);
    }
    decListId();
    if (useHash) {
        if (smp) MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].lock();
//        phashe->nMoves = n_perft;
        phashe->key = zobristKeyR;
        if (smp)MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].unlock();
    }
    return;
}

void  PerftThread::endRun() {
    tPerftRes->totMoves += tot1;
    tPerftRes->totCapture += totCapture;
}

void PerftThread::run() {
    init();
    _Tmove *move;
    incListId();
    resetList();
    u64 friends = chessboard[SIDETOMOVE_IDX] ? getBitBoard<WHITE>() : getBitBoard<BLACK>();
    u64 enemies = chessboard[SIDETOMOVE_IDX] ? getBitBoard<BLACK>() : getBitBoard<WHITE>();
    generateCaptures(chessboard[SIDETOMOVE_IDX], enemies, friends);
    generateMoves(chessboard[SIDETOMOVE_IDX], friends | enemies);

    makeZobristKey();
    u64 keyold = chessboard[ZOBRISTKEY_IDX];
    for (int ii = to - 1; ii >= from; ii--) {
        _TsubRes n_perft={0,0};
        move = getMove(ii);
        makemove(move, false, false);
        bool fhash = tPerftRes->hash != nullptr ? true : false;
        bool side = (chessboard[SIDETOMOVE_IDX] ^ 1);
        bool smp = tPerftRes->nCpu == 1 ? false : true;

        if (fhash) {
            if (side == WHITE) {
                if (smp) {
                    search<WHITE, USE_HASH_YES, SMP_YES>(n_perft, tPerftRes->depth - 1, 0);
                } else {//smp == false
                    search<WHITE, USE_HASH_YES, SMP_NO>(n_perft, tPerftRes->depth - 1, 0);
                }
            } else {
                if (smp) {
                    search<BLACK, USE_HASH_YES, SMP_YES>(n_perft, tPerftRes->depth - 1, 0);
                } else {//smp == false
                    search<BLACK, USE_HASH_YES, SMP_NO>(n_perft, tPerftRes->depth - 1, 0);
                }
            }
        } else {//no hash
            if (side == WHITE) {
                search<WHITE, USE_HASH_NO, SMP_NO>(n_perft, tPerftRes->depth - 1, 0);
            } else {
                search<BLACK, USE_HASH_NO, SMP_NO>(n_perft, tPerftRes->depth - 1, 0);
            }
        }

        takeback(move, keyold, false);
        char y;
        char x = FEN_PIECE[chessboard[SIDETOMOVE_IDX] ? getPieceAt<WHITE>(POW2[move->from]) : getPieceAt<BLACK>(POW2[move->from])];
        if (x == 'p' || x == 'P') {
            x = ' ';
        }
        if (move->capturedPiece != SQUARE_FREE) {
            y = '*';
        } else {
            y = '-';
        }
        {
            lock_guard<mutex> lock(mutexPrint);
            cout << endl << "#" << ii + 1 << " cpuID# " << getId();
            if ((decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX])).length() > 2) {
                cout << "\t" << decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]) << "\t" << n_perft.totMoves << " " << n_perft.totCapture << " ";
            } else {
                cout << "\t" << x << decodeBoardinv(move->type, move->from, chessboard[SIDETOMOVE_IDX]) << y << decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]) << "\t" <<  n_perft.totMoves << " " << n_perft.totCapture << " ";
            }
        }
        cout << flush;
        tot1 += n_perft.totMoves;
        totCapture += n_perft.totCapture;
    }
    decListId();

}

PerftThread::~PerftThread() {
}

