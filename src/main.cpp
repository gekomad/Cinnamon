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

#include "Uci.h"
#include "util/GetOpt.h"


/*

 8| 63 62 61 60 59 58 57 56
 7| 55 54 53 52 51 50 49 48
 6| 47 46 45 44 43 42 41 40
 5| 39 38 37 36 35 34 33 32
 4| 31 30 29 28 27 26 25 24
 3| 23 22 21 20 19 18 17 16
 2| 15 14 13 12 11 10 09 08
 1| 07 06 05 04 03 02 01 00
 ...a  b  c  d  e  f  g  h

tc="0/0:15+0.05" single thread
Rank Name            Elo    +    - games score oppo. draws 
   1 Cinnamon.20-1    25    2    3 49962   57%   -25   25%
   2 Cinnamon 1.2b   -25    3    2 49962   43%    25   25%

   # PLAYER           : RATING    POINTS  PLAYED    (%)
   1 Cinnamon.20-1    : 2324.9   28513.5   49962   57.1%
   2 Cinnamon 1.2b    : 2275.1   21448.5   49962   42.9%

 Rank Name            Elo    +    - games score oppo. draws
   1 Cinnamon.20-1    24    4    4 10526   57%   -24   25%
   2 Cinnamon 1.2b   -24    4    4 10526   43%    24   25%

    # PLAYER           : RATING    POINTS  PLAYED    (%)
   1 Cinnamon.20-1    : 2323.8    5974.5   10526   56.8%
   2 Cinnamon 1.2b    : 2276.2    4551.5   10526   43.2%

 */


using namespace _board;

void printHeader() {
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
#if UINTPTR_MAX == 0xffffffffffffffff
    cout << "64-bit ";
#else
    cout << "32-bit ";
#endif
#ifdef HAS_POPCNT
    cout << "popcnt ";
#endif
#ifdef HAS_BSF
    cout << "bsf ";
#endif
    cout << "compiled " << __DATE__ << " with ";
#if defined(__clang__)
    cout << "Clang/LLVM " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__ICC) || defined(__INTEL_COMPILER)
    cout << "Intel ICC "<<__VERSION__;
#elif defined(__GNUC__) || defined(__GNUG__)
    cout << "GNU GCC " << __VERSION__;
#elif defined(__HP_cc) || defined(__HP_aCC)
    cout << "Hewlett-Packard aC++" <<__HP_aCC;
#elif defined(__IBMC__) || defined(__IBMCPP__)
    cout << "IBM XL C++ <<"__IBMCPP__;
#elif defined(_MSC_VER)
    cout << "Microsoft Visual Studio. "<<_MSC_VER;
#elif defined(__PGI)
    cout << "Portland Group PGCC/PGCPP "<<__PGIC__;
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
    cout << "Oracle Solaris Studio "<<__SUNPRO_CC;
#else
    cout << "Unknown compiler";
#endif
    cout << "\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n";
#ifdef CLOP
    cout << "CLOP ENABLED\n";
#endif
#ifdef DEBUG_MODE
    cout << "DEBUG_MODE\n";
    cout << "Log level: " << LOG_LEVEL_STRING[DLOG_LEVEL] << "\n";
#endif
    cout << flush;
}

int main(int argc, char **argv) {
    printHeader();
    GetOpt::parse(argc, argv);
    return 0;
}
