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

#include "../../namespaces/def.h"
#include <atomic>

using namespace _def;
using namespace std;
#pragma pack(push)
#pragma pack(1)
typedef struct {
    u64 key;
    u64 nMoves;
} _ThashPerft;
#pragma pack(pop)

typedef struct {
    atomic_ullong totMoves;
    _ThashPerft **hash;
    u64 sizeAtDepth[255];
    int depth;
    int nCpu;
} _TPerftRes;

