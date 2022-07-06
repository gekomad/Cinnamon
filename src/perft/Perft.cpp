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

#include "Perft.h"

int Perft::count;
_ThashPerft **Perft::hash = nullptr;
bool Perft::dumping;

void Perft::dump() {
    if (depthHashFile > perftRes.depth) return;
    if (dumping || dumpFile.empty() || !hash) return;
    dumping = true;
    cout << endl << "Dump hash table in " << dumpFile << " file..." << flush;
    ofstream f;
    string tmpFile = dumpFile + ".tmp";
    f.open(tmpFile, ios_base::out | ios_base::binary);
    if (!f.is_open()) {
        cout << "error create file " << tmpFile << endl;
        return;
    }

    sleepAll(true);
    f << NAME;
    f.put(10);
    f << "1"; //version
    f.put(10);
    f << fen;
    f.put(10);
    f.write(reinterpret_cast<char *>(&perftRes.depth), sizeof(int));
    f.write(reinterpret_cast<char *>(&perftRes.nCpu), sizeof(int));
    f.write(reinterpret_cast<char *>(&mbSize), sizeof(u64));
    for (int i = 1; i <= perftRes.depth; i++) {
        f.write(reinterpret_cast<char *>(hash[i]), perftRes.sizeAtDepth[i] * sizeof(_ThashPerft));
    }
    f.close();
    rename(tmpFile.c_str(), dumpFile.c_str());
    cout << "ok" << endl;
    sleepAll(false);
    dumping = false;
}

bool Perft::load() {
    if (dumpFile.empty()) {
        return false;
    }
    ifstream f;
    string fen1;
    string perftVersion;
    int nCpuHash;

    if (!FileUtil::fileExists(dumpFile)) {
        return false;
    }
    f.open(dumpFile, ios_base::in | ios_base::binary);
    cout << endl << "load hash table from " << dumpFile << " file.." << endl;

    getline(f, fen1);//name
    getline(f, perftVersion);
    getline(f, fen1);
    cout << " Fen: " << fen1 << endl;
    cout << " Perft version: " << perftVersion << endl;

    cout << " Depth: " << perftRes.depth << endl;
    cout << flush;
    f.read(reinterpret_cast<char *>(&depthHashFile), sizeof(int));
//    if (depthHash > perftRes.depth) {
//        fatal("File wrong, depth < hash depth")
//        f.close();
//        std::exit(1);
//    }
    f.read(reinterpret_cast<char *>(&nCpuHash), sizeof(int));
    f.read(reinterpret_cast<char *>(&mbSize), sizeof(u64));
    cout << " Hash size (MB): " << mbSize << endl;
    cout << flush;
    alloc();
    if (fen.empty()) {
        fen = fen1;
    }
    if (!perftRes.nCpu) {
        perftRes.nCpu = nCpuHash;
    }
//    cout << " #cpu: " << perftRes.nCpu << endl;

    for (int i = 1; i <= depthHashFile; i++) {
        f.read(reinterpret_cast<char *>(hash[i]), perftRes.sizeAtDepth[i] * sizeof(_ThashPerft));
    }
    f.close();
    cout << "loaded" << endl;
    return true;
}

Perft::~Perft() {
    dealloc();
}

void Perft::dealloc() const {
    if (hash) {
        for (int i = 1; i <= perftRes.depth; i++) {
            free(hash[i]);
        }
        free(hash);
        hash = nullptr;
    }
}

void Perft::alloc() {
    dealloc();
    hash = (_ThashPerft **) calloc(perftRes.depth + 1, sizeof(_ThashPerft *));
    _assert(hash)
    const u64 k = 1024 * 1024 * (u64)mbSize / (u64) POW2(perftRes.depth);
    for (int i = 1; i <= perftRes.depth; i++) {
        perftRes.sizeAtDepth[i] = k * POW2(i - 1) / sizeof(_ThashPerft);
        hash[i] = (_ThashPerft *) calloc(perftRes.sizeAtDepth[i], sizeof(_ThashPerft));
        _assert(hash[i])

        DEBUG(cout << "alloc hash[" << i << "] " << perftRes.sizeAtDepth[i] * sizeof(_ThashPerft) << endl)

    }
}

void Perft::setParam(const string &fen1, int depth1, const int nCpu2, const int mbSize1, const string &dumpFile1,
                     const bool is960) {
    memset(static_cast<void *>(&perftRes), 0, sizeof(_TPerftRes));
    if (depth1 <= 0)depth1 = 1;
    mbSize = mbSize1;
    perftRes.depth = depth1;
    fen = fen1;
    dumpFile = dumpFile1;
    perftRes.nCpu = nCpu2;
    count = 0;
    dumping = false;
    chess960 = is960;
    setNthread(getNthread()); //reinitialize threads
}

void Perft::run() {

    if (!load()) {
        hash = nullptr;
        if (mbSize) {
            alloc();
        }
    }

    if (fen.empty()) {
        fen = STARTPOS;
    }
    if (!perftRes.depth) {
        perftRes.depth = 1;
    }
    if (!perftRes.nCpu) {
        perftRes.nCpu = 1;
    }
    auto *p = new PerftThread();
    p->setChess960(chess960);
    if (!fen.empty()) {
        p->loadFen(fen);
    }
    p->setPerft(true);
    uchar side = p->sideToMove;

    p->display();
    cout << "fen:\t\t\t" << fen << endl;
    cout << "depth:\t\t\t" << perftRes.depth << endl;
    cout << "#cpu:\t\t\t" << perftRes.nCpu << endl;
    cout << "cache size:\t\t" << mbSize << endl;
    cout << "dump file:\t\t" << dumpFile << endl;
    cout << "chess960:\t\t" << (chess960 ? "true" : "false") << endl;
    cout << endl << Time::getLocalTime() << " start perft test..." << endl;

    Timer t2(minutesToDump * 60);
    if (hash && !dumpFile.empty()) {
        signal(SIGINT, Perft::ctrlChandler);
        cout << "dump hash table in " << dumpFile << " every " << minutesToDump << " minutes" << endl;
        t2.registerObservers([this]() {
            dump();
        });
        t2.start();
    }

    cout << endl << endl << setw(6) << "move" << setw(20) << "tot";
    cout << setw(8) << "#";
    cout << endl;

    time.resetAndStart();
    p->incListId();
    u64 friends = side ? board::getBitmap<WHITE>(p->chessboard) : board::getBitmap<BLACK>(p->chessboard);
    u64 enemies = side ? board::getBitmap<BLACK>(p->chessboard) : board::getBitmap<WHITE>(p->chessboard);
    p->generateCaptures(side, enemies, friends);
    p->generateMoves(side, friends | enemies);
    int listcount = p->getListSize();
    count = listcount;
    delete (p);
    p = nullptr;
    assert(perftRes.nCpu > 0);
    int block = listcount / perftRes.nCpu;
    int i, s = 0;
    setNthread(perftRes.nCpu);
    for (i = 0; i < perftRes.nCpu - 1; i++) {
        PerftThread &perftThread = getNextThread();
        perftThread.setParam(fen, s, s + block, &perftRes, chess960);
        s += block;
    }

    PerftThread &perftThread = getNextThread();
    perftThread.setParam(fen, s, listcount, &perftRes, chess960);
    startAll();
    joinAll();
}

void Perft::endRun() {
    time.stop();
    const double t = time.getMill() / 1000.0;
    cout << endl << endl << "Perft moves: " << perftRes.totMoves;

    if (t) {
        if (t > 60 * 60) cout << " in " << (t / 60.0) << " minutes";
        else cout << " in " << t << " seconds";

        if ((perftRes.totMoves / t) / 1000.0 <= 1000.0)
            cout << " (" << round((perftRes.totMoves / t) / 1000.0) << " K nodes per seconds" << ")";
        cout << " (" << round((perftRes.totMoves / t) / 1000000.0) << " M nodes per seconds" << ")";
    }
    cout << endl;
    dump();
    cout << Time::getLocalTime() << endl;

    cerr << flush;

    BENCH_PRINT()

}
