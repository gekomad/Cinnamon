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

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <mutex>
#include <unistd.h>

template<class T>
class ThreadPool {

public:

    ThreadPool() {
        for (int i = 0; i < N_THREAD; i++) {
            searchPool.push_back(new T());
        }
    }

    int getFirstBit(int threadsBits1) {
        //TODO ottimizzare
        for (int i = 0; i < N_THREAD; ++i) {
            if ((threadsBits1 & POW2[i]) == 0) {
                return i;
            }
        }
        assert(0);
        return 0;
    }

    int getNextThread() {
        mutex mx;
        lock_guard<mutex> lock1(mx);
        mutex mtx;
        std::unique_lock<std::mutex> lck(mtx);

        ASSERT(Bits::bitCount(threadsBits) <= N_THREAD);
        if (Bits::bitCount(threadsBits) == N_THREAD) {
            cv.wait(lck);
        }

        int i = getFirstBit(threadsBits);
        threadsBits |= POW2[i];
        return i;
    }

    template<int threadID>
    void observerPVS() {
        mutex mx;
        lock_guard<mutex> lock1(mx);

        cout << "tolgo bit " << threadID << " maschera: " << threadsBits;
        threadsBits &= ~POW2[threadID];

        cout << " nuova maschera: " << threadsBits << " " << Bits::bitCount(threadsBits) << endl;
        cv.notify_all();
    }

    void init() {
        threadsBits = 0;
    }

protected:
    vector<T *> searchPool;

private:

    int threadsBits;
    int N_THREAD = 4;
    condition_variable cv;
};

#endif
