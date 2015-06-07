/*
    Cinnamon is a UCI chess engine
    Copyright (C) 2011-2014 Giuseppe Cannella

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

#ifndef PERFT_H_
#define PERFT_H_

#include "Search.h"
#include "Thread.h"
#include <iomanip>
#include <atomic>
#include <fstream>
#include <unistd.h>
#include "Timer.h"
#include <mutex>


/*

rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 Depth  Perft
 1      20                  verified
 2      400                 verified
 3      8902                verified
 4      197281              verified
 5      4865609             verified
 6      119060324           verified
 7      3195901860          verified
 8      84998978956         verified
 9      2439530234167       verified
 10     69352859712417      verified
 11     2097651003696806    verified

r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -
Depth   Perft
 1      48                  verified
 2      2039                verified
 3      97862               verified
 4      4085603             verified
 5      193690690           verified
 6      8031647685          verified
 7      374190009323        verified
 8      15493944087984      verified
 9		708027759953502	    verified

Be2-d1	9684737364184
Be2*a6	11610329611080
Ke1-d1	12124039380826
b2-b3	12953376489356
Ra1-c1	13537161365309
Bd2-f4	14604950944019
Nc3-b5	16558954887676
Qf3-f5	21378118001692
Ke1-f1	10712055450053
g2-g3	11066640490880
Ra1-d1	12517006910284
Be2-f1	14859603456059
Bd2-e3	17283591628447
Ne5*d7	17841614762771
Nc3-a4	18295514848885
Qf3-f4	16226823301871
e1-c1	12287002890394
Rh1-f1	12893511653497
Ne5-c6	15770851252373
a2-a4	16877033270225
Bd2-c1	13381123307084
Ne5*f7	15556584634166
Qf3-d3	14529695547948
Nc3-b1	13989775862630
g2-g4	10348190768599
Ne5-c4	11935384349792
Rh1-g1	14734587002952
e1-g1	15367034402130
Be2-b5	14439891624558
Ne5-g4	10926517539663
Ne5*g6	14660802752365
Bd2-h6	14213390633204
Qf3-e3	17154817618150
Nc3-d1	13940234223992
Qf3-h5	18109546088726
Be2-c4	15457831370434
Ne5-d3	10949898218012
Qf3-g4	17590420113757
d5-d6	13147124269772
Qf3-g3	18682076264644
Bd2-g5	16474645122655
Be2-d3	14796448995983
d5*e6	19648740553553
Qf3*h3	20213264583037
Ra1-b1	13746642706679
g2*h3	13585927848828
a2-a3	18935000323218
Qf3*f6	12429245267090

rnbqkbnr/pp1ppppp/8/2pP4/8/8/PPP1PPPP/RNBQKBNR w KQkq c6 0 2
Depth   Perft
 1      30              verified
 2      631             verified
 3      18825           verified
 4      437149          verified
 5      13787913        verified
*/

class Perft {

public:

    const static int secondsToDump = 60 * 180;
    Perft(string fen, int depth, int nCpu, u64 mbSize, string dumpFile);
    ~Perft();

private:

#pragma pack(push)
#pragma pack(1)
    typedef struct {
        u64 key;
        u64 nMoves;
    } _ThashPerft;
#pragma pack(pop)
    mutex updateHash;
    mutex mutexPrint;
    Timer* timer = nullptr;
    string fen;
    string dumpFile;
    int depth, nCpu;
    u64 mbSize;
    constexpr static u64 RANDSIDE[2] = { 0x1cf0862fa4118029ULL, 0xd2a5cab966b3d6cULL };
    _ThashPerft** hash = nullptr;
    u64 sizeAtDepth[255];
    atomic_ullong totMoves;
    void alloc();
    void dump();
    bool load();
    void setResult(u64 result) {
        totMoves += result;
    }

    class PerftThread:public Thread, public GenMoves {
    public:

        PerftThread(int, string fen, int from, int to, Perft* Perft);
        PerftThread();
        virtual ~ PerftThread();

    private:
        virtual void run();
        //void setDump();
        template <int side, bool useHash> u64 search(const int depth);
        int from, to, cpuID;
        Perft* perft;
    };
    vector <PerftThread*>threadList;
};
#endif
