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

uchar diagonalIdx(const int position, u64 allpieces) {
    const u64 File = 0x8080808080808080ull;//FILE_[position];
    u64 diagonalMaskEx_sq = _board::LEFT_DIAG[position] | POW2[position];
    allpieces = ((diagonalMaskEx_sq & allpieces) * File) >> 56;
    return allpieces;
}


uchar antiDiagonalIdx(const int position, u64 allpieces) {
    const u64 File = 0x8080808080808080ull;//FILE_[position];
    u64 diagonalMaskEx_sq = _board::RIGHT_DIAG[position] | POW2[position];
    allpieces = ((diagonalMaskEx_sq & allpieces) * File) >> 56;
    return allpieces;
}

u64 performDiagShift(const int position, const u64 allpieces) {
    /*
        LEFT
             /
            /
           /
*/
    u64 q = allpieces & MASK_BIT_UNSET_LEFT_UP[position];
    u64 k = q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_LEFT_LOWER[position];
    q = allpieces & MASK_BIT_UNSET_LEFT_DOWN[position];
    k |= q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_LEFT_UPPER[position];
    return k;
    ///RIGHT
//        q = allpieces & MASK_BIT_UNSET_RIGHT_UP[position];
//        k |= q ? bits.MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_RIGHT_LOWER[position];
//        q = allpieces & MASK_BIT_UNSET_RIGHT_DOWN[position];
//        k |= q ? bits.MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_RIGHT_UPPER[position];
}

u64 performAntiDiagShift(const int position, const u64 allpieces) {
    /*
        RIGHT
        \
         \
          \
*/
//    u64 q = allpieces & MASK_BIT_UNSET_LEFT_UP[position];
//    u64 k = q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_LEFT_LOWER[position];
//    q = allpieces & MASK_BIT_UNSET_LEFT_DOWN[position];
//    k |= q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_LEFT_UPPER[position];
//    return k;

    u64 q = allpieces & MASK_BIT_UNSET_RIGHT_UP[position];
    u64 k = q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_RIGHT_LOWER[position];
    q = allpieces & MASK_BIT_UNSET_RIGHT_DOWN[position];
    k |= q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_RIGHT_UPPER[position];
    return k;
}

vector<u64> getPermutationDiag(int pos) {
    u64 diagonalMaskEx_sq = _board::LEFT_DIAG[pos] | POW2[pos];
    while (diagonalMaskEx_sq) {
        int bit = Bits::BITScanForward(diagonalMaskEx_sq);
        RESET_LSB(diagonalMaskEx_sq);
    }
    vector<u64> allpiecesv={1,2,2,3};
    return allpiecesv;
}


int main(int argc, char **argv) {
    BitMapGenerator bitMapGenerator;return 0;

    for (int pos = 0; pos < 64; pos++) {
        vector<u64> allpiecesv = getPermutationDiag(pos);
        for (u64 allpieces:allpiecesv) {
            uchar idx = diagonalIdx(pos, allpieces);
            u64 mapDiag = performDiagShift(pos, allpieces);
            cout << "ROTATE_BITMAP_DIAGONAL[pos:" << pos << "][idx:" << (int) idx << "]=" << "0x" << mapDiag << "ULL (allpieces: " << hex << "0x" << allpieces << "ULL)\n";
            Bits::ROTATE_BITMAP_DIAGONAL[pos][idx] = mapDiag;
        }

    }

//    for (int pos = 0; pos < 64; pos++) {
//        u64 allpieces = getPermutationAntiDiag(pos);
//        if(!allpieces)break;
//        uchar idx = antiDiagonalIdx(pos, allpieces);
//        u64 mapAntiDiag = performAntiDiagShift(pos, allpieces);
//        cout << "ROTATE_BITMAP_ANTIDIAGONAL[pos:" << pos << "][idx:" << (int) idx << "]=" << "0x" << mapAntiDiag << "ULL (allpieces: " << hex << "0x" << allpieces << "ULL)\n";
//        Bits::ROTATE_BITMAP_ANTIDIAGONAL[pos][idx] = mapAntiDiag;
//    }
    return 0;
    u64 t = 0;
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
    return 0;
}

