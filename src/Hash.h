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

#include <iostream>
#include <string.h>
#include "namespaces/def.h"
#include "namespaces/board.h"
#include "util/Singleton.h"
#include <mutex>

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
        hashfALPHA = 0, hashfEXACT = 1, hashfBETA = 2
    };

    int HASH_SIZE;
#ifdef DEBUG_MODE
    unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;

    int n_cut_hashA, n_cut_hashB, cutFailed, probeHash;
#endif

    virtual ~Hash();

    void setHashSize(int mb);

    int getHashSize();

    void clearHash();

    void clearAge();

    template<bool smp>
    bool readHash(_Thash *phashe[2], const int type, const u64 zobristKeyR, _Thash *hashMini) {
        bool b = false;
        _Thash *hash = phashe[type] = &(hashArray[type][zobristKeyR % HASH_SIZE]);
        {
            lock_guard<mutex> lock(MUTEX_HASH);
            if (hash->key == zobristKeyR) {
                b = true;
                memcpy(hashMini, hash, sizeof(_Thash));
            }
        }
        return b;
    }

    void recordHash(bool running, _Thash *rootHash[2], const char depth, const char flags, const u64 key, const int score, _Tmove *bestMove);


private:
    Hash();

    void dispose();

    _Thash *hashArray[2];
    mutex MUTEX_HASH;
};

