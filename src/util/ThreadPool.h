/*
    Cinnamon is a UCI chess engine
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

#include <mutex>
#include <unistd.h>

template<typename T>
class ThreadPool {

public:
    const static int MAX_THREAD = 8;

    ThreadPool() {
        ASSERT(pow(2, MAX_THREAD) == sizeof(bitMap) / sizeof(int));
        generateBitMap();
        allocThread();
        if (getNthread() > thread::hardware_concurrency()) {
            //TODO verificare su windows
            cout << "WARNING active threads (" << getNthread() << ") > physical cores (" << thread::hardware_concurrency() << ")" << endl;
        }
    }

    int getFirstBit(int threadsBits1) {
        return bitMap[threadsBits1];
    }

    int getNextThread() {//TODO eliminare e creare un metodo start

        lock_guard<mutex> lock1(mx);

        std::unique_lock<std::mutex> lck(mtx1);

        ASSERT(Bits::bitCount(threadsBits) <= nThread);
        if (Bits::bitCount(threadsBits) == nThread) {
            cv.wait(lck);
        }

        int i = getFirstBit(threadsBits);
        threadsBits |= POW2[i];

        return i;
    }

    void releaseThread(int threadID) {//TODO eliminare e automatizzare
        lock_guard<mutex> lock1(mx1);
        threadsBits &= ~POW2[threadID];
        cv.notify_all();
    }

    void init() {
        threadsBits = 0;
    }

    int getNthread() {
        return nThread;
    }

    void setNthread(int t) {
        nThread = t;
        allocThread();
    }


protected:
    vector<T *> searchPool;

private:
    mutex mx;
    mutex mx1;
    mutex mtx1;
    int threadsBits;
    int nThread = 4;
    condition_variable cv;
    int bitMap[256];

    void generateBitMap() {
        auto lambda = [this](int threadsBits1) {
            for (int i = 0; i < nThread; ++i) {
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

    void allocThread() {
        for (int i = 0; i < searchPool.size(); i++) {
            delete searchPool[i];
        }
        searchPool.clear();
        for (int i = 0; i < nThread; i++) {
            searchPool.push_back(new T(i));
        }
    }

};

