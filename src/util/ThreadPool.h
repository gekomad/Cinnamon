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
        ASSERT(POW2[MAX_THREAD] == sizeof(bitMap) / sizeof(int));
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

    int getFirstBit(int threadsBits1) const {
        return bitMap[threadsBits1];
    }

    T &getNextThread() {
        lock_guard<mutex> lock1(mx);
        ASSERT(Bits::bitCount(threadsBits) <= nThread);
        if (Bits::bitCount(threadsBits) == nThread) {  //TODO al posto di bitcount mettere  POW2[nThread]
            cv.wait();
        }

        int i = getFirstBit(threadsBits);
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
    //TODO private
    vector<T *> searchPool;

    void releaseThread(int threadID) {
        lock_guard<mutex> lock1(mx1);
        threadsBits &= ~POW2[threadID];
        cv.notifyAll();
    }

private:
    mutex mx;
    mutex mx1;

    int threadsBits;
    int nThread = 2;
    ConditionVariable cv;
    int bitMap[256];

    void generateBitMap() {
        auto lambda = [this](int threadsBits1) {
            for (int i = 0; i < MAX_THREAD; ++i) {
                if ((threadsBits1 & 1) == 0) {
                    return i;
                }
                threadsBits1 >>= 1;
            }
            return -1;
        };
        for (int i = 0; i < (int) POW2[MAX_THREAD]; i++) {
            bitMap[i] = lambda(i);
        }
    }

};

