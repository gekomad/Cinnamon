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
#include <mutex>
#include "PerftThread.h"
#include "../blockingThreadPool/ThreadPool.h"
#include "_TPerftRes.h"
#include <signal.h>

/*
cat perft.html   | sed -e "s/xxxxxxa/\<img src=/g" | sed -e "s/xxxxxxb/>/g" >perft2.html

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

class Perft : public Thread, public ThreadPool<PerftThread>, public Singleton<Perft> {
    friend class Singleton<Perft>;

public:

    void setParam(string fen, int depth, int nCpu, int mbSize, string dumpFile, bool forceExit);

    ~Perft();

    void dump();

    virtual void run();

    virtual void endRun();

    static int count;

    void status();



private:
    Perft() : ThreadPool(1) { };

    _TPerftRes perftRes;
    high_resolution_clock::time_point start1;

    string fen;
    string dumpFile;
    u64 mbSize;

    void alloc();

    bool load();

    const static int minutesToDump = Time::HOUR_IN_MINUTES * 3;

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

    bool forceExit = false;

};

