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


// A shared Spinlock implementation

#pragma once

#include <atomic>
using namespace std;

#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_InterlockedExchange)
#define LOCK_TEST_AND_SET(_lock) _InterlockedExchange(&_lock, 1)
#define LOCK_RELEASE(_lock) _InterlockedExchange(&_lock, 0)
#else
#define LOCK_TEST_AND_SET(_lock) __sync_lock_test_and_set(&_lock, 1)
#define LOCK_RELEASE(_lock) __sync_lock_release(&_lock)
#endif

class Spinlock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    volatile long _write = 0;
    volatile atomic_int _read = { 0 };

    void _lock() {
        while (true) {
            if (!LOCK_TEST_AND_SET(_write))
                return;
            while (_write);
        }
    }

public:
    Spinlock() { }

    inline void lock() {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    inline void unlock() {
        flag.clear(std::memory_order_release);
    }

    inline void lockWrite() {
        bool w = false;
        while (true) {
            if (!w && !LOCK_TEST_AND_SET(_write)) {
                w = true;
            }
            if (w && !_read) {
                return;
            }
            while ((!w && _write) || _read);
        }
    }

    inline void unlockWrite() {
        LOCK_RELEASE(_write);
    }

    inline void lockRead() {
        lockWrite();
        _read++;
        unlockWrite();
    }

    inline void unlockRead() {
        _read--;
    }
};
