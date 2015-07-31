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
#include <atomic>
#include <mutex>
#include <unistd.h>
#include "ObserverThread.h"
#include <condition_variable>

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
        lock_guard<mutex> lock1(mxGet);
        unique_lock<mutex> lck(mtx);
        debug("ThreadPool::getNextThread count", getBitCount());
        if (bitMap[threadsBits].count == nThread) {
            debug("ThreadPool::getNextThread go wait count:", getBitCount());
            cv.wait(lck);
            debug("ThreadPool::getNextThread exit wait count:", getBitCount());
        }

        int i = bitMap[threadsBits].firstUnsetBit;
        threadPool[i]->join();
        ASSERT(!(threadsBits & POW2[i]));
        threadsBits |= POW2[i];
        debug("ThreadPool::getNextThread inc bit count:", getBitCount());
        return *threadPool[i];
    }

    int getNthread() const {
        return nThread;
    }

#ifdef DEBUG_MODE

    int getBitCount() const {
        return bitMap[threadsBits].count;
    }

#endif

    void setNthread(const int t) {
        joinAll();
        nThread = t;
        ASSERT(threadsBits == 0);
    }

    void joinAll() {
        for (T *s:threadPool) {
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
    mutex mtx;
    atomic_int threadsBits;
    //TODO togliere atomic
    int nThread = 4;
    condition_variable cv;
    mutex mxRelease;
    mutex mxGet;

    _Tslot bitMap[256];

    void releaseThread(const int threadID) {
        ASSERT(threadsBits);
        lock_guard<mutex> lock1(mxRelease);

        ASSERT(threadsBits & POW2[threadID]);
        int count = bitMap[threadsBits].count;
        threadsBits &= ~POW2[threadID];
        debug("ThreadPool::releaseThread threadID:", threadID, "count:", getBitCount());
        if (count == nThread) {
            debug("ThreadPool::releaseThread NOTIFY threadID:", threadID, "count:", getBitCount());
            cv.notify_one();
        }
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


