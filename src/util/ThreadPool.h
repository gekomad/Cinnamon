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

template<class T>
class ThreadPool {

public:

    ThreadPool() {
        generateBitMap();
        for (int i = 0; i < nThread; i++) {
            searchPool.push_back(new T(i));
        }
    }

    int getFirstBit(int threadsBits1) {
        return bitMap[threadsBits1];
    }

    int getNextThread() {
        cout << "getthread prima" << endl;
        mutex mx;
        lock_guard<mutex> lock1(mx);
        cout << "getthread dentro" << endl;
        mutex mtx1;
        std::unique_lock<std::mutex> lck(mtx1);

        ASSERT(Bits::bitCount(threadsBits) <= nThread);
        if (Bits::bitCount(threadsBits) == nThread) {
            cv.wait(lck);
        }

        int i = getFirstBit(threadsBits);
        threadsBits |= POW2[i];
        cout << "getthread dopo" << endl;
        return i;
    }

    void releaseThread(int threadID) {
        mutex mx1;
        lock_guard<mutex> lock1(mx1);
        threadsBits &= ~POW2[threadID];
        cv.notify_all();
    }

    void init() {
        threadsBits = 0;
    }

    int getSize() {
        return nThread;
    }

    void setSize(int t) {
        nThread = t;
    }

protected:
    vector<T *> searchPool;

private:

    int threadsBits;
    int nThread = 4;
    condition_variable cv;
    int bitMap[16];


    void generateBitMap() {
        auto lambda = [this](int threadsBits1) {
            ASSERT(nThread == 4);
            for (int i = 0; i < nThread; ++i) {
                if ((threadsBits1 & 1) == 0) {
                    return i;
                }
                threadsBits1 >>= 1;
            }
            return -1;
        };
        for (int i = 0; i < pow(2, nThread); i++) {
            bitMap[i] = lambda(i);
        }
    }

};

