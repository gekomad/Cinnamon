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

Spinlock PerftThread::spinlockPrint;

PerftThread::PerftThread() { }

void PerftThread::setParam(const string &fen1, const int from1, const int to1, _TPerftRes *perft1) {

    loadFen(fen1);
    this->tPerftRes = perft1;
    this->from = from1;
    this->to = to1;
}

unsigned PerftThread::perft(const string &fen, const int depth) {
    loadFen(fen);
    if (getSide()) return search<WHITE, false>(depth);
    return search<BLACK, false>(depth);
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
        u64 keyold = chessboard.bit[ZOBRISTKEY_IDX];
        makemove(move, false, false);
        setSide(side ^ 1);
        vector<string> bb = getSuccessorsFen<side ^ 1>(depthx - 1);
        n_perft.insert(n_perft.end(), bb.begin(), bb.end());
        takeback<false>(move, keyold);
        setSide(side ^ 1);
    }
    decListId();

    return n_perft;
}


template<int side, bool useHash>
u64 PerftThread::search(const int depthx) {
    checkWait();
    if (depthx == 0) {
        return 1;
    }
    u64 zobristKeyR;
    u64 n_perft = 0;
    _ThashPerft *phashe = nullptr;

    if (useHash) {
        zobristKeyR = chessboard.bit[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
        phashe = &(tPerftRes->hash[depthx][zobristKeyR % tPerftRes->sizeAtDepth[depthx]]);

        u64 k = phashe->key;
        u64 n = phashe->nMoves;

        if (zobristKeyR == (k ^ n)) {
            return n;
        }
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
        u64 keyold = chessboard.bit[ZOBRISTKEY_IDX];
        makemove(move, false, false);
        n_perft += search<side ^ 1, useHash>(depthx - 1);
        takeback<false>(move, keyold);
    }
    decListId();
    if (useHash) {
        phashe->key = (zobristKeyR ^ n_perft);
        phashe->nMoves = n_perft;
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
    u64 friends = chessboard.bit[SIDETOMOVE_IDX] ? getBitmap<WHITE>() : getBitmap<BLACK>();
    u64 enemies = chessboard.bit[SIDETOMOVE_IDX] ? getBitmap<BLACK>() : getBitmap<WHITE>();
    generateCaptures(chessboard.bit[SIDETOMOVE_IDX], enemies, friends);
    generateMoves(chessboard.bit[SIDETOMOVE_IDX], friends | enemies);

    makeZobristKey();
    u64 keyold = chessboard.bit[ZOBRISTKEY_IDX];
    for (int ii = from; ii <= to - 1; ii++) {
        u64 n_perft = 0;
        move = getMove(ii);
        makemove(move, false, false);
        bool fhash = tPerftRes->hash != nullptr;
        bool side = (chessboard.bit[SIDETOMOVE_IDX] ^ 1);

        if (fhash) {
            n_perft = side == WHITE ? search<WHITE, USE_HASH_YES>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_YES>(tPerftRes->depth - 1);
        } else {//no hash
            n_perft = side == WHITE ? search<WHITE, USE_HASH_NO>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_NO>(tPerftRes->depth - 1);
        }

        takeback<false>(move, keyold);
        char y;
        char x = FEN_PIECE[chessboard.bit[SIDETOMOVE_IDX] ? getPieceAt<WHITE>(POW2[move->from]) : getPieceAt<BLACK>(POW2[move->from])];
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
        if ((decodeBoardinv(move->type, move->to, chessboard.bit[SIDETOMOVE_IDX])).length() > 2) {
            //castle
            h = decodeBoardinv(move->type, move->to, chessboard.bit[SIDETOMOVE_IDX]);
        } else {
            h = h + x + decodeBoardinv(move->type, move->from, chessboard.bit[SIDETOMOVE_IDX]) + y + decodeBoardinv(move->type, move->to, chessboard.bit[SIDETOMOVE_IDX]);
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
