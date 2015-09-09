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
#include "_TPerftRes.h"

void Perft::dump() {
    if (dumpFile.empty() || !perftRes.hash) {
        return;
    }

    cout << endl << "Dump hash table in " << dumpFile << " file..." << flush;
    ofstream f;
    string tmpFile = dumpFile + ".tmp";
    f.open(tmpFile, ios_base::out | ios_base::binary);
    if (!f.is_open()) {
        cout << "error create file " << tmpFile << endl;
        return;
    }

    sleepAll(true);
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
    getline(f, fen1);
    f.read(reinterpret_cast<char *>(&depthHash), sizeof(int));
    if (depthHash > perftRes.depth) {
        cout << "error depth < hash depth" << endl;
        f.close();
        exit(1);
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
    cout << " fen: " << fen << "\n";
    cout << " mbSize: " << mbSize << "\n";
    cout << " depth: " << perftRes.depth << "\n";
    cout << " nCpu: " << perftRes.nCpu << "\n";
    perftRes.totMoves = 0;
    u64 kHash = 1024 * 1024 * mbSizeHash / POW2[depthHash];
    u64 sizeAtDepthHash[255];
    for (int i = 1; i <= depthHash; i++) {
        sizeAtDepthHash[i] = kHash * POW2[i - 1] / sizeof(_ThashPerft);
        cout << sizeAtDepthHash[i] * sizeof(_ThashPerft) << "\n";
    }
    _ThashPerft *tmp = (_ThashPerft *) malloc(sizeAtDepthHash[depthHash] * sizeof(_ThashPerft));
    assert(tmp);
    for (int i = 1; i <= depthHash; i++) {
        f.read(reinterpret_cast<char *>(tmp), sizeAtDepthHash[i] * sizeof(_ThashPerft));
        for (unsigned y = 0; y < sizeAtDepthHash[i]; y++) {
            if (tmp[y].key) {
                u64 rr = tmp[y].key % perftRes.sizeAtDepth[i];
                perftRes.hash[i][rr].key = tmp[y].key;
                perftRes.hash[i][rr].nMoves = tmp[y].nMoves;
            }
        }
    }
    free(tmp);
    f.close();
    cout << "loaded" << endl;
    return true;
}

Perft::~Perft() {
    if (timer) {
        delete timer;
    }
    if (perftRes.hash) {
        for (int i = 1; i <= perftRes.depth; i++) {
            free(perftRes.hash[i]);
        }
        free(perftRes.hash);
    }
}

void Perft::alloc() {
    perftRes.hash = (_ThashPerft **) calloc(perftRes.depth + 1, sizeof(_ThashPerft *));
    assert(perftRes.hash);
    u64 k = 1024 * 1024 * mbSize / POW2[perftRes.depth];
    for (int i = 1; i <= perftRes.depth; i++) {
        perftRes.sizeAtDepth[i] = k * POW2[i - 1] / sizeof(_ThashPerft);
        perftRes.hash[i] = (_ThashPerft *) calloc(perftRes.sizeAtDepth[i], sizeof(_ThashPerft));
#ifdef DEBUG_MODE
        cout << "alloc hash[" << i << "] " << perftRes.sizeAtDepth[i] * sizeof(_ThashPerft) << endl;
#endif
        assert(perftRes.hash[i]);
    }
}

Perft::Perft(string fen1, int depth1, int nCpu2, int mbSize1, string dumpFile1) : ThreadPool(1) {
    memset(&perftRes, 0, sizeof(_TPerftRes));
    mbSize = mbSize1;
    perftRes.depth = depth1;
    fen = fen1;
    dumpFile = dumpFile1;
    perftRes.totMoves = 0;
    perftRes.nCpu = nCpu2;
}

void Perft::run() {

    if (!load()) {
        perftRes.hash = nullptr;
        if (mbSize) {
            alloc();
        }
    }
    if (perftRes.hash && !dumpFile.empty()) {
        timer = new Timer(secondsToDump);
        cout << "dump hash table in file every " << (secondsToDump / 60) << " minutes or type 'flush'" << endl;
        timer->registerObservers([this]() {
            dump();
        });
        timer->start();
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
    cout << "fen:\t\t\t" << fen << "\n";
    cout << "depth:\t\t\t" << perftRes.depth << "\n";
    cout << "#cpu:\t\t\t" << perftRes.nCpu << "\n";
    cout << "cache size:\t\t" << mbSize << "\n";
    cout << "dump file:\t\t" << dumpFile << "\n";
    cout << "\nstart...\n";

    start1 = std::chrono::high_resolution_clock::now();
    p->incListId();
    u64 friends = side ? p->getBitBoard<WHITE>() : p->getBitBoard<BLACK>();
    u64 enemies = side ? p->getBitBoard<BLACK>() : p->getBitBoard<WHITE>();
    p->generateCaptures(side, enemies, friends);
    p->generateMoves(side, friends | enemies);
    int listcount = p->getListSize();
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
    auto end1 = std::chrono::high_resolution_clock::now();
    int t = Time::diffTime(end1, start1) / 1000;
    int days = t / 60 / 60 / 24;
    int hours = (t / 60 / 60) % 24;
    int minutes = (t / 60) % 60;
    int seconds = t % 60;
    cout << endl << endl << "Perft moves: " << perftRes.totMoves << " in ";
    if (days) {
        cout << days << " days, ";
    }
    if (days || hours) {
        cout << hours << " hours, ";
    }
    if (days || hours || minutes) {
        cout << minutes << " minutes, ";
    }
    if (!days) {
        cout << seconds << " seconds";
    }
    if (t) {
        cout << " (" << (perftRes.totMoves / t) / 1000 - ((perftRes.totMoves / t) / 1000) % 1000 << "k nodes per seconds" << ")";
    }
    cout << endl;
    dump();
};
