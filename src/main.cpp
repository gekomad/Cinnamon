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
//TODO aggiornare home page nuovo tarrash e link a dgt
#include "Uci.h"
#include "util/GetOpt.h"

#if defined(DEBUG_MODE) || defined(FULL_TEST)

#include <gtest/gtest.h>

void check();

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

 */

using namespace _board;

void printHeader() {
    cout <<
         "                 /\"\\       \n"
         "                /o o\\      \n"
         "           _\\/  \\   / \\/_  \n"
         "            \\\\._/  /_.//   \n"
         "            `--,  ,----'   \n"
         "              /   /        \n"
         "    ,        /    \\        \n"
         "   /|       (      )       \n"
         "  / |     ,__\\    /__,     \n"
         "  \\ \\   _//---,  ,--\\\\_    \n"
         "   \\ \\   /\\  /  /   /\\     \n"
         "    \\ \\.___,/  /           \n"
         "     \\.______,/           ";

    cout << NAME << " UCI chess engine by Giuseppe Cannella\n";
#if __WORDSIZE == 64
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
    cout << "Log level: " << LOG_LEVEL_STRING[DLOG_LEVEL] << endl;
#endif
    cout << flush;
}

void check() {
    ASSERT(sizeof(Hash::_Thash) == 16);
    ASSERT(sizeof(_Tmove) == 16);

}

int get8Rank(int i) {
    while (1) {
        if ((i + 8) > 63)return i;
        i += 8;
    }
}

int get1Rank(int i) {
    while (1) {
        if ((i - 8) < 0)return i;
        i -= 8;
    }
}

int getLungCol1(int i) {
    int y = 0;

    while (1) {
        if (i <= 0)break;
        i -= 8;
        y++;
    }
    return y;

}

int getLungCol(int i) {
    int y = 0;

    while (1) {
        if (i > 63)break;
        i += 8;
        y++;
    }
    return y;

}

int main(int argc, char **argv) {
//TODO
//    //white
//    for (int y = 0; y < 64; y++) {
//        auto l = getLungCol(y) - 1;
//        auto r8 = get8Rank(y);
//        auto r1 = get1Rank(y);
//        auto col = LINK_SQUARE[y][r8] | POW2[y] | POW2[r8];
//        u64 square = col;
//        auto x = min(l, 7 - r1);
//        for (int t = 1; t <= x; t++) {
//            square |= (col << t);
//        }
//        x = min(l, r1);
//        for (int t = 1; t <= x; t++) {
//            square |= (col >> t);
//        }
//        //cout << "0x" << hex << square << "ULL,\n";
//    }
//
//    //black
//    for (int y = 0; y < 64; y++) {
//        auto l = getLungCol1(y) - 1;
//        auto r8 = get8Rank(y);
//        auto r1 = get1Rank(y);
//        auto col = LINK_SQUARE[y][r1] | POW2[y] | POW2[r1];
//        u64 square = col;
//        auto x = min(l, 7 - r1);
//        for (int t = 1; t <= x; t++) {
//            square |= (col << t);
//        }
//        x = min(l, r1);
//        for (int t = 1; t <= x; t++) {
//            square |= (col >> t);
//        }
//        cout << "0x" << hex << square << "ULL,\n";
//    }
//    cout << 1;
//
    // TODO printHeader();
    check();
#if defined(FULL_TEST)
    testing::InitGoogleTest(&argc, argv);
    if (RUN_ALL_TESTS())return 1;
#if defined(FULL_TEST)
    return 0;
#endif
    cout << "\n\n";
#endif

    GetOpt::parse(argc, argv);

    return 0;
}


