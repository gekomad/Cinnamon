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
#include <shared_mutex>

using namespace _board;

class Hash : public Singleton<Hash> {
    friend class Singleton<Hash>;

public:

    static const int HASH_GREATER = 0;
    static const int HASH_ALWAYS = 1;
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

    int HASH_SIZE;
#ifdef DEBUG_MODE
    unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;

    int n_cut_hashA, n_cut_hashE, n_cut_hashB, cutFailed, probeHash;
#endif

    virtual ~Hash();

    bool setHashSize(int mb);

    int getHashSize();

    void clearHash();

    void clearAge();

    void recordHash(u64, bool running, Hash::_Thash *phashe_greater, _Thash *phashe_always, const char depth, const char flags, const u64 key, const int score, _Tmove *bestMove);

    bool readHash(_Thash *phashe[2], const int type, const u64 zobristKeyR, const int depth, _Thash &hash);

private:
    Hash();

    void dispose();

    _Thash *hashArray[2];
    static const int N_MUTEX_BUCKET = 128;
    shared_timed_mutex MUTEX_BUCKET[2][N_MUTEX_BUCKET];

};

