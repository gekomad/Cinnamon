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
#ifdef _WIN32

#include <windows.h>
#include <intrin.h>

#define LOCK_TEST_AND_SET(_lock) _InterlockedExchange(&_lock, 1)
#define LOCK_RELEASE(_lock) _InterlockedExchange(&_lock, 0)

#pragma intrinsic(_InterlockedExchange)

class Spinlock {
private:
    volatile long _lock = 0;
public:
    __forceinline void lock() {
        while (true) {
            if (!LOCK_TEST_AND_SET(_lock))
                return;
            while (_lock);
        }
    }
    __forceinline void unlock() { LOCK_RELEASE(_lock); }
};

#else

#define LOCK_TEST_AND_SET(_lock) __sync_lock_test_and_set(&_lock, 1)
#define LOCK_RELEASE(_lock) __sync_lock_release(&_lock)

class Spinlock {
private:
    volatile long _lock = 0;
public:
    inline void lock() {
        while (true) {
            if (!LOCK_TEST_AND_SET(_lock))
                return;
            while (_lock);
        }
    }
    inline void unlock() { LOCK_RELEASE(_lock); }
};

#endif