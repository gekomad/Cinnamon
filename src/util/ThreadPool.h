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
#include "../namespaces.h"
#include <condition_variable>

template<typename T, typename = typename std::enable_if<std::is_base_of<Thread, T>::value, T>::type>
class ThreadPool : public ObserverThread {

public:

    ThreadPool() : threadsBits(0) {

        generateBitMap();
        for (int i = 0; i < 8; i++) {
            T *x = new T();
            x->setId(i);
            threadPool.push_back(x);
        }

        registerThreads();
        nThread = thread::hardware_concurrency();
        if (nThread == 0) {
            nThread = 1;
        } else if (nThread > 8) {
            nThread = 8;
        }

        cout << "ThreadPool size: " << getNthread() << "\n";

    }

    T &getNextThread() {
        lock_guard<mutex> lock1(mxRel);
        debug("ThreadPool::getNextThread");
        unique_lock<mutex> lck(mtx);
        cv.wait(lck, [this] { return bitMap[threadsBits].count != nThread; });
        return getThread();
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
        nThread = std::min(8, t);
        ASSERT(threadsBits == 0);
    }

    void joinAll() {
        for (T *s:threadPool) {
            s->join();
        }
    }

    ~ThreadPool() {
        joinAll();
        for (T *s:threadPool) {
            delete s;
            s = nullptr;
        }
        threadPool.clear();
    }

protected:
    vector<T *> threadPool;
private:
    typedef struct {
        uchar firstUnsetBit;
        uchar count;
    } _Tslot;

    mutex mtx;
    atomic_int threadsBits;
    int nThread;
    condition_variable cv;
    mutex mxGet;
    mutex mxRel;
    _Tslot bitMap[256];

    T &getThread() {
        lock_guard<mutex> lock1(mxGet);
        int i = bitMap[threadsBits].firstUnsetBit;
        threadPool[i]->join();
        ASSERT(!(threadsBits & POW2[i]));
        threadsBits |= POW2[i];
        debug("ThreadPool::getNextThread inc bit");
        return *threadPool[i];
    }

    void releaseThread(const int threadID) {
        ASSERT_RANGE(threadID, 0, 7);
        lock_guard<mutex> lock1(mxGet);
        ASSERT(threadsBits & POW2[threadID]);
        threadsBits &= ~POW2[threadID];
        cv.notify_all();
        debug("ThreadPool::releaseThread NOTIFY threadID:", threadID);
    }

    void observerEndThread(int threadID) {
        releaseThread(threadID);
    }

    void registerThreads() {
        for (T *s:threadPool) {
            s->registerObserverThread(this);
        }
    }

    void generateBitMap() {
        for (int idx = 0; idx < 256; idx++) {
            int g = idx;
            bitMap[idx].firstUnsetBit = 0;
            for (int i = 0; i < 8; ++i) {
                if (g & 1) {
                    bitMap[idx].count++;
                }
                g >>= 1;
            }
            g = idx;
            for (int i = 0; i < 8; ++i) {
                if ((g & 1) == 0) {
                    bitMap[idx].firstUnsetBit = i;
                    break;
                }
                g >>= 1;
            }
        };
    }
};


