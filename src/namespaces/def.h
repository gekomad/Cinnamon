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

#include "../util/Time.h"
#include "../util/FileUtil.h"
#include "debug.h"
#include <array>

using namespace _debug;

namespace _def {
    using namespace std;

    static constexpr int BLACK = 0;
    static constexpr int WHITE = 1;

    typedef unsigned char uchar;
    typedef long long unsigned u64;
    typedef u64 _Tchessboard[16];

#define RESET_LSB(bits) (bits&=bits-1)

#if defined(CLOP) || defined(DEBUG_MODE)
#define STATIC_CONST
#else
#define STATIC_CONST static constexpr
#endif

#define _assert(a) if(!(a)){  print_stacktrace();cout<<dec<<endl<<Time::getLocalTime()<<" ********************************** assert error in "<<FileUtil::getFileName(__FILE__)<< ":"<<__LINE__<<" "<<" **********************************"<<endl;cerr<<flush;std::exit(1);};

#ifdef BENCH_MODE
#define BENCH(a) a
#else
#define BENCH(a)
#endif

#ifdef DEBUG_MODE
#define ASSERT(a) _assert(a)
#define ASSERT_RANGE(value, from, to) {if ((value)<(from) || (value)>(to)){cout<<"ASSERT_RANGE: "<<value<<endl;_assert(0)};}
#define INC(a) (a++)
#define SET(a, v) (a=(v))
#define ADD(a, b) (a+=(b))
#else

#define ASSERT(a)
#define ASSERT_RANGE(value, from, to)
#define INC(a)
#define SET(a, v)
#define ADD(a, b)

#endif


#ifdef HAS_POPCNT
#ifdef HAS_64BIT

    static inline int bitCount(const u64 bits) {
        return __builtin_popcountll(bits);
    }
#else
    static inline int bitCount(const u64 bits) {
        return __builtin_popcountl(bits)+__builtin_popcountl(bits>>32);
    }
#endif
#else

    static inline int bitCount(u64 bits) {  
        int count = 0;
        for (; bits; RESET_LSB(bits))
            count++;
        return count;
    }

#endif


#ifdef HAS_BSF
#ifdef HAS_64BIT

    static inline int BITScanForward(const u64 bits) {
        size_t idx;
        __asm__("bsfq %1, %0": "=r"(idx): "rm"(bits));
        return idx;
    }

    static inline int BITScanReverse(const u64 bits) {
        size_t idx;
        __asm__("bsrq %1, %0": "=r"(idx): "rm"(bits));
        return idx;
    }

#else
    static inline int BITScanForward(const u64 bits) {
        return ((unsigned) bits) ? __builtin_ffs(bits) - 1 : __builtin_ffs(bits >> 32) + 31;
    }

    static inline int BITScanReverse(const u64 bits) {
        return ((unsigned)(bits >> 32)) ? 63 - __builtin_clz(bits >> 32) : 31 - __builtin_clz(bits);
    }
#endif
#else

    static inline int BITScanForward(u64 bb) {
        //  @author Matt Taylor (2003)
        static constexpr int lsb_64_table[64] = {63, 30, 3, 32, 59, 14, 11, 33, 60, 24, 50, 9, 55, 19, 21, 34, 61, 29,
                                                 2, 53, 51, 23, 41, 18, 56, 28, 1, 43, 46, 27, 0, 35, 62, 31, 58, 4, 5,
                                                 49, 54, 6, 15, 52, 12, 40, 7, 42, 45, 16, 25, 57, 48, 13, 10, 39, 8,
                                                 44, 20, 47, 38, 22, 17, 37, 36, 26};
        bb ^= bb - 1;
        unsigned folded = (int) bb ^(bb >> 32);
        return lsb_64_table[folded * 0x78291ACF >> 26];
    }

    static inline int BITScanReverse(u64 bb) {

        static constexpr int index64[64] = {0, 47, 1, 56, 48, 27, 2, 60, 57, 49, 41, 37, 28, 16, 3, 61, 54, 58, 35, 52,
                                            50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4, 62, 46, 55, 26, 59, 40, 36, 15,
                                            53, 34, 51, 20, 43, 31, 22, 10, 45, 25, 39, 14, 33, 19, 30, 9, 24, 13, 18,
                                            8, 12, 7, 6, 5, 63};
        static constexpr u64 debruijn64 = 0x03f79d71b4cb0a89ULL;
        bb |= bb >> 1;
        bb |= bb >> 2;
        bb |= bb >> 4;
        bb |= bb >> 8;
        bb |= bb >> 16;
        bb |= bb >> 32;
        return index64[(bb * debruijn64) >> 58];
    }

#endif

    template<int side, int shift>
    static inline u64 shiftForward(const u64 bits) {
        ASSERT(shift == 7 || shift == 8 || shift == 9);
        static constexpr array<u64, 2> NO_FILE_LEFT = {0xFEFEFEFEFEFEFEFEULL, 0x7F7F7F7F7F7F7F7FULL};
        static constexpr array<u64, 2> NO_FILE_RIGHT = {0x7F7F7F7F7F7F7F7FULL, 0xFEFEFEFEFEFEFEFEULL};

        auto a = side == WHITE ? bits << shift : bits >> shift;
        if (shift == 7)
            return a & NO_FILE_LEFT[side];
        if (shift == 9)
            return a & NO_FILE_RIGHT[side];
        return a;

    }

    static inline int BITScanForwardUnset(const u64 bb) {
        return BITScanForward(~bb);
    }
}
