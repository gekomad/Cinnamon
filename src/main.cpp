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
#include "util/BitmapGenerator.h"

vector<u64> getPermutationDiag(int pos);

#if defined(DEBUG_MODE) || defined(FULL_TEST)

#include <gtest/gtest.h>

#endif

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

tc="40/4:0+0" single thread
Rank Name                        Elo    +    - games score oppo. draws
   1 Cinnamon 1.2c-smp.16-tris    20    5    5  5864   56%   -20   34%
   2 Cinnamon 1.2b               -20    5    5  5864   44%    20   34%

 su windows
Rank Name                  Elo    +    - games score oppo. draws
   1 Cinnamon 1.2c-smp.x    23   13   13   492   57%   -23   34%
   2 Cinnamon 1.2b         -23   13   13   492   43%    23   34%

 */

using namespace _board;

void printHeader() {
    cout << NAME;
    cout << " UCI by Giuseppe Cannella\n";
#if UINTPTR_MAX == 0xffffffffffffffffULL
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


u64 get_col(u64 board, int col, int y) {
    const u64 column_mask = 0x8080808080808080ull;
    const u64 magic = 0x2040810204081ull;

    u64 column = (board << col) & column_mask;
    column *= magic;
    u64 t = 0;
    int idx = (column >> 56) & 0xff;

    return Bits::MASK_BIT_SET_NOBOUND[idx][y];

}


int main(int argc, char **argv) {
    BitMapGenerator bitMapGenerator;return 0;


    return 0;
  /*  u64 t = 0;
    for (int ii = 0; ii < 999999; ii++)
        for (int i = 0; i < 64; i++) {
            for (int j = 0; j < 64; j++) {
//                t += get_col(_random::RANDOM_KEY[i][j], i % 8,j%64);
                t += performDiagShift(_random::RANDOM_KEY[i][j], j % 64);
            }
        }
    cout << t;
    return 0;
    printHeader();
#if defined(DEBUG_MODE) || defined(FULL_TEST)
    testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS())return 1;
#if defined(FULL_TEST)
    return 0;
#endif
    cout << "\n\n";
#endif

    GetOpt::parse(argc, argv);
    return 0;*/
}

