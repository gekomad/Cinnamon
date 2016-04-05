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
#include "Perft.h"

Spinlock PerftThread::SPINLOCK_HASH;
Spinlock PerftThread::spinlockPrint;

PerftThread::PerftThread() { perftMode = true; }

void PerftThread::setParam(const string &fen1, const int from1, const int to1, _TPerftRes *perft1) {

    loadFen(fen1);
    this->tPerftRes = perft1;
    this->from = from1;
    this->to = to1;
}

unsigned PerftThread::perft(const string &fen, const int depth) {
    loadFen(fen);
    if (getSide()) return search<WHITE, false, false>(depth);
    return search<BLACK, false, false>(depth);
}

vector<string> PerftThread::getSuccessorsFen(const string &fen1, const int depth) {
    loadFen(fen1);
    if (getSide()) return getSuccessorsFen<WHITE>(depth);
    return getSuccessorsFen<BLACK>(depth);
}


template<int side>
vector<string> PerftThread::getSuccessorsFen(const int depthx) {
    if (depthx == 0) {
        vector<string> a;
        a.push_back(boardToFen());
        return a;
    }

    vector<string> n_perft;

    int listcount;
    _Tmove *move;
    incListId();
    u64 friends = getBitmap<side>();
    u64 enemies = getBitmap<side ^ 1>();
    bool b = generateCaptures<side>(enemies, friends);
    ASSERT(!b);
    generateMoves<side>(friends | enemies);
    listcount = getListSize();
    if (!listcount) {
        decListId();
        vector<string> a;
        return a;
    }
    for (int ii = 0; ii < listcount; ii++) {
        move = getMove(ii);
        u64 keyold = chessboard[ZOBRISTKEY_IDX];
        makemove(move, false, false);
        setSide(side ^ 1);
        vector<string> bb = getSuccessorsFen<side ^ 1>(depthx - 1);
        n_perft.insert(n_perft.end(), bb.begin(), bb.end());
        takeback(move, keyold, false);
        setSide(side ^ 1);
    }
    decListId();

    return n_perft;
}


template<int side, bool useHash, bool smp>
u64 PerftThread::search(const int depthx) {
    checkWait();
    if (depthx == 0) {
        partialTot++;
        return 1;
    }
    u64 zobristKeyR;
    u64 n_perft = 0;
    _ThashPerft *phashe = nullptr;

    if (useHash) {
        zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
        if (smp)SPINLOCK_HASH.lock();
        phashe = &(tPerftRes->hash[depthx][zobristKeyR % tPerftRes->sizeAtDepth[depthx]]);
        if (zobristKeyR == phashe->key) {
            partialTot += phashe->nMoves;
            u64 r = phashe->nMoves;
            if (smp)SPINLOCK_HASH.unlock();
            return r;
        }
        if (smp)SPINLOCK_HASH.unlock();
    }
    int listcount;
    _Tmove *move;
    incListId();
    u64 friends = getBitmap<side>();
    u64 enemies = getBitmap<side ^ 1>();
    bool b = generateCaptures<side>(enemies, friends);
    ASSERT(!b);
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
        if (smp) SPINLOCK_HASH.lock();
        phashe->key = zobristKeyR;
        phashe->nMoves = n_perft;
        if (smp) SPINLOCK_HASH.unlock();
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
    u64 friends = chessboard[SIDETOMOVE_IDX] ? getBitmap<WHITE>() : getBitmap<BLACK>();
    u64 enemies = chessboard[SIDETOMOVE_IDX] ? getBitmap<BLACK>() : getBitmap<WHITE>();
    generateCaptures(chessboard[SIDETOMOVE_IDX], enemies, friends);
    generateMoves(chessboard[SIDETOMOVE_IDX], friends | enemies);

    makeZobristKey();
    u64 keyold = chessboard[ZOBRISTKEY_IDX];
    for (int ii = from; ii <= to - 1; ii++) {
        u64 n_perft = 0;
        move = getMove(ii);
        makemove(move, false, false);
        bool fhash = tPerftRes->hash != nullptr ? true : false;
        bool side = (chessboard[SIDETOMOVE_IDX] ^ 1);
        bool smp = tPerftRes->nCpu == 1 ? false : true;

        if (fhash) {
            if (side == WHITE) {
                n_perft = smp ? search<WHITE, USE_HASH_YES, true>(tPerftRes->depth - 1) : search<WHITE, USE_HASH_YES, false>(tPerftRes->depth - 1);
            } else {
                n_perft = smp ? search<BLACK, USE_HASH_YES, true>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_YES, false>(tPerftRes->depth - 1);
            }
        } else {//no hash
            if (side == WHITE) {
                n_perft = smp ? search<WHITE, USE_HASH_NO, true>(tPerftRes->depth - 1) : search<WHITE, USE_HASH_NO, false>(tPerftRes->depth - 1);
            } else {
                n_perft = smp ? search<BLACK, USE_HASH_NO, true>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_NO, false>(tPerftRes->depth - 1);
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

        if (fhash)spinlockPrint.lock();
        cout << "\n";
        string h;
        if ((decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX])).length() > 2) {
            //castle
            h = decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]);
        } else {
            h = h + x + decodeBoardinv(move->type, move->from, chessboard[SIDETOMOVE_IDX]) + y + decodeBoardinv(move->type, move->to, chessboard[SIDETOMOVE_IDX]);
        }
        cout << setw(6) << h;
        cout << setw(20) << n_perft;
        cout << setw(8) << (Perft::count--);
        if (fhash) spinlockPrint.unlock();

        cout << flush;
        tot += n_perft;
    }
    decListId();
}

PerftThread::~PerftThread() {
}

u64 PerftThread::getPartial() {
    return partialTot;
}