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
//#include "db/bitbase/kpk.h"
#include "Uci.h"
#include "util/GetOpt.h"

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

#ifdef HAS_64BIT
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
static inline int BITScanForwardTODO(u64 bb) {
    //  @author Matt Taylor (2003)
    static constexpr int lsb_64_table[64] = {63, 30, 3, 32, 59, 14, 11, 33, 60, 24, 50, 9, 55, 19, 21, 34, 61, 29,
                                             2, 53, 51, 23, 41, 18, 56, 28, 1, 43, 46, 27, 0, 35, 62, 31, 58, 4, 5,
                                             49, 54, 6, 15, 52, 12, 40, 7, 42, 45, 16, 25, 57, 48, 13, 10, 39, 8,
                                             44, 20, 47, 38, 22, 17, 37, 36, 26};
    bb ^= bb - 1;
    unsigned folded = (int) bb ^(bb >> 32);
    return lsb_64_table[folded * 0x78291ACF >> 26];
}
void benchTODO() {
    using namespace _random;
    Time time;
    u64 r;
    const unsigned K = 30000000;
    const unsigned tot = K * 64;

    ////
    r = 0;
    time.resetAndStart();
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += 1;
        }
    time.stop();
    cout << "plus:\t\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\t\tres: " << r << endl;

    ////
    r = 0;
    time.resetAndStart();
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += BITScanForward(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "BITScanForward:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\tres: " << r << endl;
    ////
    r = 0;
    time.resetAndStart();
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += BITScanForwardTODO(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "BITScanForward2:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\tres: " << r << endl;
    ////
    r = 0;
    time.resetAndStart();
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += BITScanReverse(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "BITScanReverse:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\tres: " << r << endl;

    ////
    r = 0;
    time.resetAndStart();
    for (int t = 0; t < K; t++)
        for (int j = 0; j < 64; j++) {
            r += shiftForward<WHITE, 7>(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "shiftForward7:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\t\tres: " << r << endl;
    ////
    r = 0;
    time.resetAndStart();
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += shiftForward<WHITE, 8>(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "shiftForward8:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\t\tres: " << r << endl;
    ////
    time.resetAndStart();
    r = 0;
    for (unsigned t = 0; t < K; t++)
        for (unsigned j = 0; j < 64; j++) {
            r += bitCount(RANDOM_KEY[0][j]);
        }
    time.stop();
    cout << "bitCount:\ttime: " << time.getNano() / tot << " nano: " << time.getNano() << "\tres: " << r << endl;

}

int main(int argc, char **argv) {
    ASSERT(sizeof(Hash::_Thash) == 16);
    ASSERT(sizeof(_Tmove) == 16);
//    auto k=KPK().getInstance();
//    k.init();
//    return 0;
//    benchTODO();
//    return 0;

  printHeader();

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


