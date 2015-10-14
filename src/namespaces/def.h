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

using namespace _debug;
namespace _def {
    using namespace std;

    typedef unsigned char uchar;
    typedef long long unsigned u64;
    typedef u64 _Tchessboard[16];

    static const u64 POW2[64] = {0x1ULL, 0x2ULL, 0x4ULL, 0x8ULL, 0x10ULL, 0x20ULL, 0x40ULL, 0x80ULL, 0x100ULL, 0x200ULL, 0x400ULL, 0x800ULL, 0x1000ULL, 0x2000ULL, 0x4000ULL, 0x8000ULL, 0x10000ULL, 0x20000ULL, 0x40000ULL, 0x80000ULL, 0x100000ULL, 0x200000ULL, 0x400000ULL, 0x800000ULL, 0x1000000ULL, 0x2000000ULL, 0x4000000ULL, 0x8000000ULL, 0x10000000ULL, 0x20000000ULL, 0x40000000ULL, 0x80000000ULL, 0x100000000ULL, 0x200000000ULL, 0x400000000ULL, 0x800000000ULL, 0x1000000000ULL, 0x2000000000ULL, 0x4000000000ULL, 0x8000000000ULL, 0x10000000000ULL, 0x20000000000ULL, 0x40000000000ULL, 0x80000000000ULL, 0x100000000000ULL, 0x200000000000ULL, 0x400000000000ULL, 0x800000000000ULL, 0x1000000000000ULL, 0x2000000000000ULL, 0x4000000000000ULL, 0x8000000000000ULL, 0x10000000000000ULL, 0x20000000000000ULL, 0x40000000000000ULL, 0x80000000000000ULL, 0x100000000000000ULL, 0x200000000000000ULL, 0x400000000000000ULL, 0x800000000000000ULL, 0x1000000000000000ULL, 0x2000000000000000ULL, 0x4000000000000000ULL, 0x8000000000000000ULL};

    static const u64 NOTPOW2[64] = {0xfffffffffffffffeULL, 0xfffffffffffffffdULL, 0xfffffffffffffffbULL, 0xfffffffffffffff7ULL, 0xffffffffffffffefULL, 0xffffffffffffffdfULL, 0xffffffffffffffbfULL, 0xffffffffffffff7fULL, 0xfffffffffffffeffULL, 0xfffffffffffffdffULL, 0xfffffffffffffbffULL, 0xfffffffffffff7ffULL, 0xffffffffffffefffULL, 0xffffffffffffdfffULL, 0xffffffffffffbfffULL, 0xffffffffffff7fffULL, 0xfffffffffffeffffULL, 0xfffffffffffdffffULL, 0xfffffffffffbffffULL, 0xfffffffffff7ffffULL, 0xffffffffffefffffULL, 0xffffffffffdfffffULL, 0xffffffffffbfffffULL, 0xffffffffff7fffffULL, 0xfffffffffeffffffULL, 0xfffffffffdffffffULL, 0xfffffffffbffffffULL, 0xfffffffff7ffffffULL, 0xffffffffefffffffULL, 0xffffffffdfffffffULL, 0xffffffffbfffffffULL, 0xffffffff7fffffffULL, 0xfffffffeffffffffULL, 0xfffffffdffffffffULL, 0xfffffffbffffffffULL, 0xfffffff7ffffffffULL, 0xffffffefffffffffULL, 0xffffffdfffffffffULL, 0xffffffbfffffffffULL, 0xffffff7fffffffffULL, 0xfffffeffffffffffULL, 0xfffffdffffffffffULL, 0xfffffbffffffffffULL, 0xfffff7ffffffffffULL, 0xffffefffffffffffULL, 0xffffdfffffffffffULL, 0xffffbfffffffffffULL,
                                    0xffff7fffffffffffULL, 0xfffeffffffffffffULL, 0xfffdffffffffffffULL, 0xfffbffffffffffffULL, 0xfff7ffffffffffffULL, 0xffefffffffffffffULL, 0xffdfffffffffffffULL, 0xffbfffffffffffffULL, 0xff7fffffffffffffULL, 0xfeffffffffffffffULL, 0xfdffffffffffffffULL, 0xfbffffffffffffffULL, 0xf7ffffffffffffffULL, 0xefffffffffffffffULL, 0xdfffffffffffffffULL, 0xbfffffffffffffffULL, 0x7fffffffffffffffULL};


#if defined(CLOP) || defined(DEBUG_MODE)
#define STATIC_CONST
#else
#define STATIC_CONST static const
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define FORCEINLINE __inline
#elif defined(__MINGW32__)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __always_inline
#endif

#define assert(a) if(!(a)){  print_stacktrace();cout<<dec<<endl<<Time::getLocalTime()<<" ********************************** assert error in "<<FileUtil::getFileName(__FILE__)<< " line "<<__LINE__<<" "<<" **********************************"<<endl;cerr<<flush;std::_Exit(1);};

#ifdef DEBUG_MODE
#define ASSERT(a) assert(a)
#define ASSERT_RANGE(value, from, to) {if ((value)<(from) || (value)>(to)){cout<<"VALUE: "<<value<<endl;assert(0)};}
#define INC(a) (a++)
#define ADD(a, b) (a+=(b))
#else

#define ASSERT(a)
#define ASSERT_RANGE(value, from, to)
#define INC(a)
#define ADD(a, b)

#endif
}
