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

    template<bool smp>
    bool readHash(_Thash *phashe[2], const int type, const u64 zobristKeyR, _Thash &hashMini) {
        bool b = false;
        _Thash *hash = phashe[type] = &(hashArray[type][zobristKeyR % HASH_SIZE]);
        if(smp) MUTEX_BUCKET[type][zobristKeyR % N_MUTEX_BUCKET].lock_shared();
        if (hash->key == zobristKeyR) {
            b = true;
            memcpy(&hashMini, hash, sizeof(_Thash));
        }
        if(smp)MUTEX_BUCKET[type][zobristKeyR % N_MUTEX_BUCKET].unlock_shared();
        return b;
    }

    template<bool smp>
    void recordHash(u64 zobristKeyR, bool running, _Thash *rootHash[2], const char depth, const char flags, const u64 key, const int score, _Tmove *bestMove) {
        ASSERT(key);
        ASSERT(rootHash[HASH_GREATER]);
        ASSERT(rootHash[HASH_ALWAYS]);
        if (!running) {
            return;
        }
        ASSERT(abs(score) <= 32200);
        _Thash tmp;

        tmp.key = key;
        tmp.score = score;
        tmp.flags = flags;
        tmp.depth = depth;
        if (bestMove && bestMove->from != bestMove->to) {
            tmp.from = bestMove->from;
            tmp.to = bestMove->to;
        } else {
            tmp.from = tmp.to = 0;
        }
        int keyMutex = zobristKeyR % N_MUTEX_BUCKET;
        if(smp) MUTEX_BUCKET[HASH_GREATER][keyMutex].lock();
        memcpy(rootHash[HASH_GREATER], &tmp, sizeof(_Thash));
        if(smp)MUTEX_BUCKET[HASH_GREATER][keyMutex].unlock();

        //////////////

#ifdef DEBUG_MODE
    if (flags == hashfALPHA) {
        nRecordHashA++;
    } else if (flags == hashfBETA) {
        nRecordHashB++;
    } else {
        nRecordHashE++;
    }
#endif
        tmp.entryAge = 1;
#ifdef DEBUG_MODE
    //TODO cancellare blocco
    ASSERT(tmp.key == key);
    ASSERT(tmp.score == score);
    ASSERT(tmp.flags == flags);
    ASSERT(tmp.depth == depth);
    if (bestMove && bestMove->from != bestMove->to) {
        ASSERT(tmp.from == bestMove->from);
        ASSERT(tmp.to == bestMove->to);
    } else {
        ASSERT(tmp.from == 0);
        ASSERT(tmp.to == 0);
    }
#endif
        if(smp)MUTEX_BUCKET[HASH_ALWAYS][keyMutex].lock();
        if (rootHash[HASH_ALWAYS]->key && rootHash[HASH_ALWAYS]->depth >= depth && rootHash[HASH_ALWAYS]->entryAge) {//TODO dovrebbe essere greater
            INC(collisions);
            if(smp) MUTEX_BUCKET[HASH_ALWAYS][keyMutex].unlock();
            return;
        }
        memcpy(rootHash[HASH_ALWAYS], &tmp, sizeof(_Thash));
        if(smp) MUTEX_BUCKET[HASH_ALWAYS][keyMutex].unlock();
    }

private:
    Hash();

    void dispose();

    _Thash *hashArray[2];
    static const int N_MUTEX_BUCKET = 4096;
    shared_timed_mutex MUTEX_BUCKET[2][N_MUTEX_BUCKET];

};

