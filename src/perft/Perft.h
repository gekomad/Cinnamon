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

#include "../Search.h"
#include <iomanip>
#include <atomic>
#include <fstream>
#include <unistd.h>
#include "../util/Timer.h"
#include "PerftThread.h"
#include "../threadPool/ThreadPool.h"
#include "_TPerftRes.h"
#include <signal.h>

/*

rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1
 Depth  Perft
 1      20                  
 2      400                 
 3      8902                
 4      197281              
 5      4865609             
 6      119060324           
 7      3195901860          
 8      84998978956         
 9      2439530234167       
 10     69352859712417      
 11     2097651003696806    

r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
Depth   Perft
 1      48                  
 2      2039                
 3      97862               
 4      4085603             
 5      193690690           
 6      8031647685
 7      374190009323        
 8      15493944087984
 9		708027759953502

*/

class Perft: public Thread<Perft>, public ThreadPool<PerftThread>, public Singleton<Perft> {
    friend class Singleton<Perft>;

public:

    void setParam(const string &fen1, int depth1, const int nCpu2, const int mbSize1, const string &dumpFile1);

    ~Perft();

    void dump();

    void run();

    void endRun();

    static int count;

    u64 getResult() {
        return perftRes.totMoves;
    }

private:
    Perft() : ThreadPool(1) { };

    _TPerftRes perftRes;
    high_resolution_clock::time_point start1;

    string fen;
    string dumpFile;
    u64 mbSize;

    void alloc();

    bool load();

    constexpr static int minutesToDump = Time::HOUR_IN_MINUTES * 10;

    static void ctrlChandler(int s) {
        if (dumping) {
            cout << "dumping hash... " << endl;
            return;
        }

        Perft::getInstance().dump();
        if (s < 0)cout << s;
        exit(1);

    }

    static bool dumping;

};

