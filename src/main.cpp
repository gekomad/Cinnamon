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

ratings
Rank Name                  Elo    +    - games score oppo. draws
   1 buzz-008_2181         385   13   13  2551   79%   146   14%
   2 beowulf_2194          368   13   13  2551   77%   146   14%
   3 gk-090-64-ja_2109     360   13   13  2552   77%   146   16%
   4 clarabit_2098         263   12   12  2542   65%   146   18%
   5 soldat_1960           197   12   12  2542   56%   146   16%
   6 smash_1925            172   12   12  2552   53%   146   12%
   7 faile-64_1976         153   11   11  2552   51%   146   20%
   8 Cinnamon 1.2a         152   11   11  2550   51%   146   30%
   9 Cinnamon 1.2b         146    5    5 33140   52%   -11   15%
  10 Cinnamon 1.1c         105   11   11  2552   44%   146   27%
  11 heracles_1973         105   12   12  2551   44%   146   16%
  12 zct_2043              -15   12   12  2542   30%   146   12%
  13 jabba-64_2041       -1196  200  179  2551    0%   146    0%
  14 gullydeckel-64_1982 -1196  200  179  2552    0%   146    0%

los
                     bu be gk cl so sm fa Ci Ci Ci he zc ja gu
buzz-008_2181           95 99100100100100100100100100100100100
beowulf_2194          4    78100100100100100100100100100100100
gk-090-64-ja_2109     0 21   100100100100100100100100100100100
clarabit_2098         0  0  0    99100100100100100100100100100
soldat_1960           0  0  0  0    99 99 99 99100100100100100
smash_1925            0  0  0  0  0    98 99 99 99 99100100100
faile-64_1976         0  0  0  0  0  1    54 86 99 99100100100
Cinnamon 1.2a         0  0  0  0  0  0 45    83 99 99100100100
Cinnamon 1.2b         0  0  0  0  0  0 13 16    99 99100100100
Cinnamon 1.1c         0  0  0  0  0  0  0  0  0    52100100100
heracles_1973         0  0  0  0  0  0  0  0  0 47   100100100
zct_2043              0  0  0  0  0  0  0  0  0  0  0    99 99
jabba-64_2041         0  0  0  0  0  0  0  0  0  0  0  0    50
gullydeckel-64_1982   0  0  0  0  0  0  0  0  0  0  0  0 49


tc="0/0:15+0.05" single thread
Rank Name                                   Elo    +    - games score oppo. draws
   1 Cinnamon 1.2c-smp.12-single-core-mod     3    5    5  5511   51%    -3   22%
   2 Cinnamon 1.2c-SNAPSHOT                  -3    5    5  5511   49%     3   22%

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
    cout << "Unknow compiler";
#endif
    cout << "\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\n";
#ifdef CLOP
    cout << "CLOP ENABLED\n";
#endif
#ifdef DEBUG_MODE
    cout << "DEBUG_MODE\n";
#endif
    cout << flush;
}

int main(int argc, char **argv) {
//    Message m("aaaaaaaaa", "",-1,-1,-1, "",-1,-1,-1,1252);
//    cout <<m.getSerializedString();
    cout << " " << LOG_LEVEL_STRING[DLOG_LEVEL] << " " << endl;
    printHeader();
    GetOpt::parse(argc, argv);
    return 0;
}
