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

#include <chrono>
#include <limits.h>
#include <random>

class Random {
  public:
    static uint64_t state;

    static __attribute__((always_inline)) bool getRandomBool() {
        static std::mt19937_64 rng{std::random_device{}()};
        static std::bernoulli_distribution dist(0.5);
        return dist(rng);
    }

    static __attribute__((always_inline)) bool getFastRandomBool() {
        state = state * 1664525u + 1013904223u; // LCG
        return (state >> 31) & 1;
    }

    static unsigned long long getRandom64() {
        unsigned sign = getRandom(0, 1);
        unsigned long long a = getRandom(0, INT_MAX) | sign << 31;
        sign = getRandom(0, 1);
        const unsigned b = getRandom(0, INT_MAX) | sign << 31;
        a <<= 32;
        return a | b;
    }

    static int getRandom(const int from, const int to) {
#if _WIN32 || _WIN64
        std::chrono::nanoseconds ns = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch());
        std::mt19937 mt(static_cast<unsigned int>(ns.count()));
#else
        std::random_device rd;
        std::mt19937 mt(rd());
#endif
        std::uniform_int_distribution<> dist(from, to);
        return dist(mt);
    }
};
