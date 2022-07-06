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

PerftThread::PerftThread() { perftMode = true; }

void PerftThread::setParam(const string &fen1, const int from1, const int to1, _TPerftRes *perft1, const bool is960) {
    chess960 = is960;
    loadFen(fen1);
    this->tPerftRes = perft1;
    this->from = from1;
    this->to = to1;
}

unsigned PerftThread::perft(const string &fen, const int depth) {
    loadFen(fen);
    if (sideToMove) return search<WHITE, false>(depth);
    return search<BLACK, false>(depth);
}

vector<string> PerftThread::getSuccessorsFen(const string &fen1, const int depth) {
    loadFen(fen1);
    if (sideToMove) return getSuccessorsFen<WHITE>(depth);
    return getSuccessorsFen<BLACK>(depth);
}

template<uchar side>
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
    u64 friends = board::getBitmap<side>(chessboard);
    u64 enemies = board::getBitmap<X(side)>(chessboard);
    generateCaptures<side>(enemies, friends);
    generateMoves<side>(friends | enemies);
    listcount = getListSize();
    if (!listcount) {
        decListId();
        vector<string> a;
        return a;
    }
    for (int ii = 0; ii < listcount; ii++) {
        move = getMove(ii);
        const u64 keyold = chessboard[ZOBRISTKEY_IDX];
        const uchar oldEnpassant = enPassant;
        makemove(move, false);
        setSide(X(side));
        vector<string> bb = getSuccessorsFen<X(side)>(depthx - 1);
        n_perft.insert(n_perft.end(), bb.begin(), bb.end());
        takeback(move, keyold, oldEnpassant, false);
        setSide(X(side));
    }
    decListId();

    return n_perft;
}


template<uchar side, bool useHash>
u64 PerftThread::search(const int depthx) {
    checkWait();
    if (depthx == 0) return 1;

    if (depthx == 1) {
        const u64 friends = board::getBitmap<side>(chessboard);
        const u64 enemies = board::getBitmap<X(side)>(chessboard);
        incListId();
        generateCaptures<side>(enemies, friends);
        generateMoves<side>(enemies | friends);

        const int listcount = getListSize();
        decListId();
        return listcount;
    }
    u64 zobristKeyR;
    u64 nPerft = 0;
    _ThashPerft *phashe = nullptr;

    if (useHash) {
        zobristKeyR = chessboard[ZOBRISTKEY_IDX] ^ _random::RANDSIDE[side];
        phashe = &(Perft::hash[depthx][zobristKeyR % tPerftRes->sizeAtDepth[depthx]]);
        const auto k = phashe->key;
        const auto d = phashe->nMoves;
        if (zobristKeyR == (k ^ d)) {
            return d;
        }
    }

    incListId();
    const u64 friends = board::getBitmap<side>(chessboard);
    const u64 enemies = board::getBitmap<X(side)>(chessboard);
    generateCaptures<side>(enemies, friends);
    generateMoves<side>(friends | enemies);
    const int listcount = getListSize();
    _Tmove *move;
    for (int ii = 0; ii < listcount; ii++) {
        move = getMove(ii);
        const u64 keyold = chessboard[ZOBRISTKEY_IDX];
        const uchar oldEnpassant = enPassant;
        makemove(move, false);
        nPerft += search<X(side), useHash>(depthx - 1);
        takeback(move, keyold, oldEnpassant, false);
    }
    decListId();
    if (useHash) {
        phashe->key = (zobristKeyR ^ nPerft);
        phashe->nMoves = nPerft;
    }
    return nPerft;
}

void PerftThread::endRun() {
    tPerftRes->totMoves += tot;
}

void PerftThread::run() {
    init();
    _Tmove *move;
    incListId();
    resetList();
    const u64 friends = sideToMove ? board::getBitmap<WHITE>(chessboard) : board::getBitmap<BLACK>(
            chessboard);
    const u64 enemies = sideToMove ? board::getBitmap<BLACK>(chessboard) : board::getBitmap<WHITE>(
            chessboard);
    generateCaptures(sideToMove, enemies, friends);
    generateMoves(sideToMove, friends | enemies);

    makeZobristKey();
    const u64 keyold = chessboard[ZOBRISTKEY_IDX];
    const uchar oldEnpassant = enPassant;
    for (int ii = from; ii <= to - 1; ii++) {
        u64 n_perft;
        move = getMove(ii);
        makemove(move, false);
        bool fhash = Perft::hash != nullptr;
        bool side = X(sideToMove);

        if (fhash) {
            n_perft = side == WHITE ? search<WHITE, USE_HASH_YES>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_YES>(
                    tPerftRes->depth - 1);
        } else {
            n_perft = side == WHITE ? search<WHITE, USE_HASH_NO>(tPerftRes->depth - 1) : search<BLACK, USE_HASH_NO>(
                    tPerftRes->depth - 1);
        }

        takeback(move, keyold, oldEnpassant, false);

        char x = FEN_PIECE[sideToMove ? board::getPieceAt<WHITE>(POW2(move->from), chessboard)
                                      : board::getPieceAt<BLACK>(POW2(move->from), chessboard)];
        x = toupper(x);
        if (x == 'P') x = ' ';

        if (fhash)spinlockPrint.lock();
        cout << endl;
        string h = decodeBoardinv(move, sideToMove, true);

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

