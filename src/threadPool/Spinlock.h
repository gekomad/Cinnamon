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

#pragma intrinsic(_InterlockedExchange)

class Spinlock {
private:
    volatile long _lock = 0;
public:
    __forceinline void lock() {
        while (true) {
            if (!_InterlockedExchange(&_lock, 1))
                return;
            while (_lock);//TODO Sleep(1)
        }
    }

    inline void unlock() { _InterlockedExchange(&_lock, 0); }

};

#else

class Spinlock {
private:
    volatile int _lock = 0;
public:
    inline void lock() {
        while (true) {
            if (!__sync_lock_test_and_set(&_lock, 1))
                return;
            while (_lock);//TODO Sleep(1)
        }
    }

    inline void unlock() { __sync_lock_release(&_lock); }

};

#endif