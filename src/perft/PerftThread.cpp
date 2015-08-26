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

shared_timed_mutex PerftThread::MUTEX_BUCKET[N_MUTEX_BUCKET];
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
u64 PerftThread::search(const int depthx) {
    checkWait();
    if (depthx == 0) {
        return 1;
    }
    u64 zobristKeyR;
    u64 n_perft = 0;
    _ThashPerft *phashe = nullptr;

    if (useHash) {
        zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ RANDSIDE[side];
        if (smp) MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].lock_shared();
        phashe = &(tPerftRes->hash[depthx][zobristKeyR % tPerftRes->sizeAtDepth[depthx]]);
        if (zobristKeyR == phashe->key) {
            u64 res = phashe->nMoves;
            if (smp)MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].unlock_shared();
            return res;
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
        return 0;
    }
    generateMoves<side>(friends | enemies);
    listcount = getListSize();
    if (!listcount) {
        decListId();
        return 0;
    }
    for (int ii = 0; ii < listcount; ii++) {
        move = getMove(ii);
        u64 keyold = chessboard[ZOBRISTKEY_IDX];
        makemove(move, false, false);
        n_perft += search<side ^ 1, useHash, smp>(depthx - 1);
        takeback(move, keyold, false);
    }
    decListId();
    if (useHash) {
        if (smp) MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].lock();
        phashe->nMoves = n_perft;
        phashe->key = zobristKeyR;
        if (smp)MUTEX_BUCKET[zobristKeyR % N_MUTEX_BUCKET].unlock();
    }
    return n_perft;
}

void  PerftThread::endRun() {
    tPerftRes->totMoves += tot;
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
        u64 n_perft = 0;
        move = getMove(ii);
        makemove(move, false, false);
        bool fhash = tPerftRes->hash != nullptr ? true : false;
        bool side = (chessboard[SIDETOMOVE_IDX] ^ 1);
        bool smp = tPerftRes->nCpu == 1 ? false : true;

        if (fhash) {
            if (side == WHITE) {
                if (smp) {
                    n_perft = search<WHITE, true, SMP_YES>(tPerftRes->depth - 1);
                } else {//smp == false
                    n_perft = search<WHITE, true, SMP_NO>(tPerftRes->depth - 1);
                }
            } else {
                if (smp) {
                    n_perft = search<BLACK, true, SMP_YES>(tPerftRes->depth - 1);
                } else {//smp == false
                    n_perft = search<BLACK, true, SMP_NO>(tPerftRes->depth - 1);
                }
            }
        } else {//no hash
            if (side == WHITE) {
                n_perft = search<WHITE, false, SMP_NO>(tPerftRes->depth - 1);
            } else {
                n_perft = search<BLACK, false, SMP_NO>(tPerftRes->depth - 1);
            }
        }

        takeback(move, keyold, false);
        char y;
        char x = FEN_PIECE[chessboard[SIDETOMOVE_IDX] ? getPieceAt<WHITE>(POW2[move->from]) : getPieceAt<BLACK>(
                POW2[move->from])];
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
                cout << "\t" << decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]) << "\t" << n_perft <<
                " ";
            } else {
                cout << "\t" << x << decodeBoardinv(move->type, move->from, chessboard[SIDETOMOVE_IDX]) << y <<
                decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]) << "\t" << n_perft << " ";
            }
        }
        cout << flush;
        tot += n_perft;
    }
    decListId();

}

PerftThread::~PerftThread() {
}

