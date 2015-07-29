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
class ThreadPool {

public:
    const static int MAX_THREAD = 8;

    ThreadPool() : threadsBits(0) {

        generateBitMap();
        for (int i = 0; i < MAX_THREAD; i++) {
            searchPool.push_back(new T(i));
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

        lock_guard<mutex> lock1(mxGet);
        if (bitMap[threadsBits].count == nThread) {
            cv.wait();
        }

        //get first bit == 0
        int i = bitMap[threadsBits].firstUnsetBit;
        threadsBits |= POW2[i];

        return *searchPool[i];
    }

    int getNthread() const {

        return nThread;
    }

    void setNthread(int t) {
        nThread = t;
        threadsBits = 0;
    }

    void joinAll() {
        for (Search *s:searchPool) {
            s->join();
        }
    }

    ~ThreadPool() {
        searchPool.clear();
    }

protected:

    vector<T *> searchPool;

    void releaseThread(int threadID) {
        lock_guard<mutex> lock1(mxRelease);
        threadsBits &= ~POW2[threadID];
        cv.notifyAll();
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
    mutex mxBit;
    _Tslot bitMap[256];

    void generateBitMap() {
        for (int idx = 0; idx < (int) POW2[MAX_THREAD]; idx++) {
            int threadsBits1 = idx;
            bitMap[idx].count = Bits::bitCount(idx);
            for (int i = 0; i < MAX_THREAD; ++i) {
                if ((threadsBits1 & 1) == 0) {
                    bitMap[idx].firstUnsetBit = i;
                    break;
                }
                threadsBits1 >>= 1;
            }
        };
    }
};


