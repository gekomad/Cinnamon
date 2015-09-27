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

#include "../util/stacktrace.h"
#include "../util/Time.h"
#include "../util/FileUtil.h"

namespace _def {
    using namespace std;

    typedef unsigned char uchar;
    typedef long long unsigned u64;
    typedef u64 _Tchessboard[16];

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