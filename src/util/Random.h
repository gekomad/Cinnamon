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

#include <random>
#include <ctime>
#include <chrono>
#include <limits.h>

class Random {

public:
    static unsigned long long getRandom64() {
        unsigned sign = Random::getRandom(0, 1);
        unsigned long long a = Random::getRandom(0, INT_MAX) | sign << 31;
        sign = Random::getRandom(0, 1);
        unsigned b = Random::getRandom(0, INT_MAX) | sign << 31;
        a <<= 32;
        a |= b;
        return a;
    }

    static int getRandom(const int from, const int to) {
#ifdef _WIN32
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

