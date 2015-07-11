/*
    Cinnamon is a UCI chess engine
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

#include <iostream>
#include <string.h>
#include "namespaces.h"
#include "util/Singleton.h"
#include <mutex>

using namespace _board;

class Hash : public Singleton<Hash> {
    friend class Singleton<Hash>;

public:

#pragma pack(push)
#pragma pack(1)
    typedef struct {
        u64 key;
        short score;
        char depth;
        uchar from:6;
        uchar to:6;
        uchar entryAge:1;
        uchar flags:2;
    } _Thash;
#pragma pack(pop)
    enum : char {
        hashfEXACT = 1, hashfALPHA = 0, hashfBETA = 2
    };
    //TODO private
    _Thash *hash_array_greater;
    _Thash *hash_array_always;
    int HASH_SIZE;
#ifdef DEBUG_MODE
    unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;
    //unsigned n_cut_hash;
    int n_cut_hashA, n_cut_hashE, n_cut_hashB, cutFailed, probeHash;
#endif

    virtual ~Hash();

    bool setHashSize(int mb);

    int getHashSize();

    void clearHash();

    void clearAge();

    void recordHash(bool running, Hash::_Thash *phashe_greater, _Thash *phashe_always, const char depth, const char flags, const u64 key, const int score, _Tmove *bestMove);

private:
    Hash();

    void dispose();

    mutex mutexRecordHash;


};

