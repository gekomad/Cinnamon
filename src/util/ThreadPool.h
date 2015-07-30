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

#include "Thread.h"
#include <mutex>
#include <unistd.h>
#include "ObserverThread.h"
#include "ConditionVariable.h"

template<typename T, typename = typename std::enable_if<std::is_base_of<Thread, T>::value, T>::type>
class ThreadPool : public ObserverThread {

public:
    const static int MAX_THREAD = 8;

    ThreadPool() : threadsBits(0) {

        generateBitMap();
        for (int i = 0; i < MAX_THREAD; i++) {
            threadPool.push_back(new T(i));
        }
        if (thread::hardware_concurrency() && (unsigned) getNthread() > thread::hardware_concurrency()) {
            cout << "WARNING active threads (" << getNthread() << ") > physical cores (" << thread::hardware_concurrency() << ")" << endl;
        }
#ifdef DEBUG_MODE
        else {
            cout << "Active threads: " << getNthread() << "\n";
        }
#endif
    }

    T &getNextThread() {
        debug("........................getNextThread ");
        lock_guard<mutex> lock1(mxGet);
        if (bitMap[threadsBits].count == nThread) {
            cv.wait();
        }

        int i = bitMap[threadsBits].firstUnsetBit;
        threadPool[i]->join();
        ASSERT(!(threadsBits & POW2[i]));
        threadsBits |= POW2[i];
        return *threadPool[i];
    }

    int getNthread() const {

        return nThread;
    }

    void setNthread(int t) {
        joinAll();
//#ifdef DEBUG_MODE
//        for (Search *s:threadPool) {
//            ASSERT(!s->isJoinable());
//        }
//#endif
        nThread = t;

        ASSERT(threadsBits == 0);
//        threadsBits = 0;
    }

    void joinAll() {
        for (Search *s:threadPool) {
            s->join();
        }
    }

    ~ThreadPool() {
        joinAll();
        threadPool.clear();
    }

protected:

    vector<T *> threadPool;

    void observerEndThread(int threadID) {
        releaseThread(threadID);
    }

    void registerThreads() {
        for (T *s:threadPool) {
            s->registerObserverThread(this);
        }
    }

private:
    typedef struct {
        uchar firstUnsetBit;
        uchar count;
    } _Tslot;

    atomic_int threadsBits;
    int nThread = 2;
    ConditionVariable cv;
    mutex mxRelease;
    mutex mxGet;

    _Tslot bitMap[256];

    void releaseThread(int threadID) {
        debug("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1 releaseThread ");
        lock_guard<mutex> lock1(mxRelease);

        ASSERT(threadsBits & POW2[threadID]);
        threadsBits &= ~POW2[threadID];
//        threadPool[threadID]->join();
        cv.notifyAll();
    }

    void generateBitMap() {
        for (int idx = 0; idx < (int) POW2[MAX_THREAD]; idx++) {
            int g = idx;
            bitMap[idx].count = Bits::bitCount(idx);
            for (int i = 0; i < MAX_THREAD; ++i) {
                if ((g & 1) == 0) {
                    bitMap[idx].firstUnsetBit = i;
                    break;
                }
                g >>= 1;
            }
        };
    }


};


