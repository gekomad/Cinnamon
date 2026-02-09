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

#include "../namespaces/bits.h"
#include "../util/logger.h"
#include "ObserverThread.h"
#include "Thread.h"
#include <atomic>
#include <condition_variable>

using namespace _def;

template <typename T, typename = typename std::enable_if<std::is_base_of<Thread<T>, T>::value, T>::type> class ThreadPool : public ObserverThread {
  public:
    explicit ThreadPool(const int t) : threadsBits(0) {
        setNthread(t);
    }
    using iterator = typename std::vector<T *>::iterator;
    using const_iterator = typename std::vector<T *>::const_iterator;

    iterator begin() noexcept {
        return threadPool.begin();
    }
    iterator end() noexcept {
        return threadPool.end();
    }

    const_iterator begin() const noexcept {
        return threadPool.begin();
    }
    const_iterator end() const noexcept {
        return threadPool.end();
    }

    const_iterator cbegin() const noexcept {
        return threadPool.cbegin();
    }
    const_iterator cend() const noexcept {
        return threadPool.cend();
    }
    ThreadPool() : ThreadPool(thread::hardware_concurrency()) {
    }

    T &getNextThread() {
        unique_lock<mutex> lck(mtx);
        cv.wait(lck, [this] { return bitCount(threadsBits) != nThread; });
        return getThread();
    }

    int getNthread() const {
        return nThread;
    }

#ifndef NDEBUG

    int getBitCount() const {
        return bitCount(threadsBits);
    }

#endif

    bool setNthread(const int t) {
        if (t < 1 || t > 64) {
            warn("invalid value");
            return false;
        }
        joinAll();
        removeAllThread();
        nThread = t;
        assert(threadsBits == 0);
        for (int i = 0; i < nThread; i++) {
            T *x = new T();
            x->setId(i);
            threadPool.push_back(x);
        }
        registerThreads();
        trace("ThreadPool size: ", getNthread()) return true;
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

    ~ThreadPool() override {
        removeAllThread();
    }
    T &getThread(int i) const {
        assert(i < nThread);
        return *threadPool[i];
    }

  private:
    vector<T *> threadPool;
    mutex mtx;
    atomic<u64> threadsBits;
    int nThread = 0;
    condition_variable cv;

    T &getThread() {
        int i = BITScanForwardUnset(threadsBits);
        threadPool[i]->join();
        assert(!(threadsBits & POW2(i)));
        threadsBits |= POW2(i);
        return *threadPool[i];
    }

    void releaseThread(const int threadID) {
        ASSERT_RANGE(threadID, 0, 63);
        assert(threadsBits & POW2(threadID));
        threadsBits &= ~POW2(threadID);
        cv.notify_all();
        debug("ThreadPool::releaseThread #", threadID);
    }

    void observerEndThread(const int threadID) override {
        releaseThread(threadID);
    }

    void registerThreads() {
        for (T *s : threadPool) {
            s->template registerObserverThread<ThreadPool<T>>(this);
        }
    }

    void removeAllThread() {
        joinAll();
        for (const T *s : threadPool) {
            delete s;
        }
        threadPool.clear();
        assert(threadsBits == 0);
    }
};
