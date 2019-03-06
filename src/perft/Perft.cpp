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

bool Perft::dumping;

void Perft::dump() {
    if (dumping || dumpFile.empty() || !perftRes.hash) {
        return;
    }
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
    f.put(10);
    f << fen;
    f.put(10);
    f.write(reinterpret_cast<char *>(&perftRes.depth), sizeof(int));
    f.write(reinterpret_cast<char *>(&perftRes.nCpu), sizeof(int));
    f.write(reinterpret_cast<char *>(&mbSize), sizeof(u64));
    for (int i = 1; i <= perftRes.depth; i++) {
        f.write(reinterpret_cast<char *>(perftRes.hash[i]), perftRes.sizeAtDepth[i] * sizeof(_ThashPerft));
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
    int nCpuHash, depthHash;
    u64 mbSizeHash;
    if (!FileUtil::fileExists(dumpFile)) {
        return false;
    }
    f.open(dumpFile, ios_base::in | ios_base::binary);
    cout << endl << "load hash table from " << dumpFile << " file.." << endl;
    string detailType;
    getline(f, detailType);

    getline(f, fen1);
    f.read(reinterpret_cast<char *>(&depthHash), sizeof(int));
    if (depthHash > perftRes.depth) {
        fatal("error depth < hash depth");
        f.close();
        std::exit(0);
    };
    f.read(reinterpret_cast<char *>(&nCpuHash), sizeof(int));
    f.read(reinterpret_cast<char *>(&mbSizeHash), sizeof(u64));

    alloc();
    if (fen.empty()) {
        fen = fen1;
    }
    if (!perftRes.nCpu) {
        perftRes.nCpu = nCpuHash;
    }
    cout << " fen: " << fen << endl;
    cout << " mbSize: " << mbSize << endl;
    cout << " depth: " << perftRes.depth << endl;
    cout << " nCpu: " << perftRes.nCpu << endl;

    for (int i = 1; i <= depthHash; i++) {
        f.read(reinterpret_cast<char *>(perftRes.hash[i]), perftRes.sizeAtDepth[i] * sizeof(_ThashPerft));
    }
    f.close();
    cout << "loaded" << endl;
    return true;
}

Perft::~Perft() {
    if (perftRes.hash) {
        for (int i = 1; i <= perftRes.depth; i++) {
            free(perftRes.hash[i]);
        }
        free(perftRes.hash);
        perftRes.hash = nullptr;
    }
}

void Perft::alloc() {
    perftRes.hash = (_ThashPerft **) calloc(perftRes.depth + 1, sizeof(_ThashPerft *));
    _assert(perftRes.hash);
    const u64 k = 1024 * 1024 * mbSize / POW2[perftRes.depth];
    for (int i = 1; i <= perftRes.depth; i++) {
        perftRes.sizeAtDepth[i] = k * POW2[i - 1] / sizeof(_ThashPerft);
        perftRes.hash[i] = (_ThashPerft *) calloc(perftRes.sizeAtDepth[i], sizeof(_ThashPerft));
        _assert(perftRes.hash[i]);
#ifdef DEBUG_MODE
        cout << "alloc hash[" << i << "] " << perftRes.sizeAtDepth[i] * sizeof(_ThashPerft) << endl;
#endif

    }
}

void Perft::setParam(const string &fen1, int depth1, const int nCpu2, const int mbSize1, const string &dumpFile1) {
    memset(&perftRes, 0, sizeof(_TPerftRes));
    if (depth1 <= 0)depth1 = 1;
    mbSize = mbSize1;
    perftRes.depth = depth1;
    fen = fen1;
    dumpFile = dumpFile1;
    perftRes.nCpu = nCpu2;
    count = 0;
    dumping = false;
    setNthread(getNthread());//reinitialize threads
}

void Perft::run() {

    if (!load()) {
        perftRes.hash = nullptr;
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
    PerftThread *p = new PerftThread();
    if (!fen.empty()) {
        p->loadFen(fen);
    }
    p->setPerft(true);
    int side = p->getSide() ? 1 : 0;
    p->display();
    cout << "fen:\t\t\t" << fen << endl;
    cout << "depth:\t\t\t" << perftRes.depth << endl;
    cout << "#cpu:\t\t\t" << perftRes.nCpu << endl;
    cout << "cache size:\t\t" << mbSize << endl;
    cout << "dump file:\t\t" << dumpFile << endl;
    cout << endl << Time::getLocalTime() << " start perft test..." << endl;

    Timer t2(minutesToDump * 60);
    if (perftRes.hash && !dumpFile.empty()) {
        signal(SIGINT, Perft::ctrlChandler);
        cout << "dump hash table in " << dumpFile << " every " << minutesToDump << " minutes" << endl;
        cout << "type 'dump' to dump now" << endl;
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
    u64 friends = side ? p->getBitmap<WHITE>() : p->getBitmap<BLACK>();
    u64 enemies = side ? p->getBitmap<BLACK>() : p->getBitmap<WHITE>();
    p->generateCaptures(side, enemies, friends);
    p->generateMoves(side, friends | enemies);
    int listcount = p->getListSize();
    count = listcount;
    delete (p);
    p = nullptr;
    ASSERT(perftRes.nCpu > 0);
    int block = listcount / perftRes.nCpu;
    int i, s = 0;
    setNthread(perftRes.nCpu);
    for (i = 0; i < perftRes.nCpu - 1; i++) {
        PerftThread &perftThread = getNextThread();
        perftThread.setParam(fen, s, s + block, &perftRes);
        s += block;
    }
    PerftThread &perftThread = getNextThread();
    perftThread.setParam(fen, s, listcount, &perftRes);
    startAll();
    joinAll();
}

void Perft::endRun() {
    time.stop();
    int t = time.getMill() / 1000;

    cout << endl << endl << "Perft moves: " << perftRes.totMoves;

    cout << " in " << t<<" seconds";

    if (t) {
        cout << " (" << (perftRes.totMoves / t) / 1000 - ((perftRes.totMoves / t) / 1000) % 1000 <<
            "k nodes per seconds" << ")";
    }
    cout << endl;
    dump();
    cout << Time::getLocalTime() << endl;

    cerr << flush;

}
