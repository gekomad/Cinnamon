/*
    https://github.com/gekomad/ThreadPool
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
#include "../util/Bits.h"
#include <condition_variable>

template<typename T, typename = typename std::enable_if<std::is_base_of<Thread, T>::value, T>::type>
class ThreadPool : public ObserverThread {

public:
    ThreadPool(int t) {
        setNthread(t);
    }

    ThreadPool() : threadsBits(0) {
        setNthread(thread::hardware_concurrency());
    }

    T &getNextThread() {
        lock_guard<mutex> lock1(mxRel);
        unique_lock<mutex> lck(mtx);
        cv.wait(lck, [this] { return Bits::bitCount(threadsBits) != nThread; });
        return getThread();
    }

    int getNthread() const {
        return nThread;
    }

#ifdef DEBUG_MODE

    int getBitCount() const {
        return Bits::bitCount(threadsBits);
    }

#endif

    bool setNthread(const int t) {
        if (t < 1 || t > 64 || t == nThread) {
            return false;
        }
        joinAll();
        removeAllThread();
        nThread = t;
        ASSERT(threadsBits == 0);
        for (int i = 0; i < nThread; i++) {
            T *x = new T();
            x->setId(i);
            threadPool.push_back(x);
        }
        registerThreads();
        cout << "ThreadPool size: " << getNthread() << "\n";
        return true;
    }

    void joinAll() {
        for (int i = 0; i < nThread; i++) {
            threadPool[i]->join();
        }
    }

    void sleepAll(bool b) {
        for (int i = 0; i < nThread; i++) {
            threadPool[i]->setSleep(b);
            if (!b) {
                threadPool[i]->notify();
            }
        }
    }

    void startAll() {
        for (int i = 0; i < nThread; i++) {
            threadPool[i]->start();
        }
    }

    ~ThreadPool() {
        removeAllThread();
    }

protected:
    vector<T *> threadPool;
private:

    mutex mtx;
    atomic <u64> threadsBits;
    int nThread;
    condition_variable cv;
    mutex mxGet;
    mutex mxRel;

    T &getThread() {
        lock_guard<mutex> lock1(mxGet);
        int i = Bits::BITScanForwardUnset(threadsBits);
        threadPool[i]->join();
        ASSERT(!(threadsBits & POW2[i]));
        threadsBits |= POW2[i];
        return *threadPool[i];
    }

    void releaseThread(const int threadID) {
        ASSERT_RANGE(threadID, 0, 63);
        lock_guard<mutex> lock1(mxGet);
        ASSERT(threadsBits & POW2[threadID]);
        threadsBits &= ~POW2[threadID];
        cv.notify_all();
        debug("ThreadPool::releaseThread #", threadID);
    }

    void observerEndThread(int threadID) {
        releaseThread(threadID);
    }

    void registerThreads() {
        for (T *s:threadPool) {
            s->registerObserverThread(this);
        }
    }

    void removeAllThread() {
        joinAll();
        for (T *s:threadPool) {
            delete s;
        }
        threadPool.clear();
        ASSERT(threadsBits == 0);
    }

};


