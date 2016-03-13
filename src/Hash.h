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
#include "util/logger.h"
#include "threadPool/Spinlock.h"

using namespace _board;
using namespace _logger;

class Hash : public Singleton<Hash> {
    friend class Singleton<Hash>;

public:

    static const int HASH_GREATER = 0;
    static const int HASH_ALWAYS = 1;

    typedef struct {
        u64 key;
        short score;
        char depth;
        uchar from:6;
        uchar to:6;
        uchar entryAge:1;
        uchar flags:2;
    } _Thash;

    enum : char {
        hashfALPHA = 0, hashfEXACT = 1, hashfBETA = 2
    };

#ifdef DEBUG_MODE
    unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;

    int n_cut_hashA, n_cut_hashB, cutFailed, probeHash;
#endif

    virtual ~Hash();

    void setHashSize(int mb);

    int getHashSize();

    void clearHash();

    void clearAge();

    template<bool smp, int type>
    bool readHash(_Thash *phashe[2], const u64 zobristKeyR, _Thash *hashMini) {
        bool b = false;
        _Thash *hash = phashe[type] = &(hashArray[type][zobristKeyR % HASH_SIZE]);

        if (smp && type == HASH_GREATER)spinlockHashGreater.lock();
        if (smp && type == HASH_ALWAYS)spinlockHashAlways.lock();
        if (hash->key == zobristKeyR) {
            b = true;
            memcpy(hashMini, hash, sizeof(_Thash));
        }
        if (smp && type == HASH_GREATER)spinlockHashGreater.unlock();
        if (smp && type == HASH_ALWAYS)spinlockHashAlways.unlock();

        return b;
    }

    template<bool smp>
    void recordHash(bool running, _Thash *rootHash[2], const char depth, const char flags, const u64 key, const int score, _Tmove *bestMove) {
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

        if (smp)spinlockHashGreater.lock();
        memcpy(rootHash[HASH_GREATER], &tmp, sizeof(_Thash));
        if (smp)spinlockHashGreater.unlock();


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

        if (smp)spinlockHashAlways.lock();
        if (rootHash[HASH_ALWAYS]->key && rootHash[HASH_ALWAYS]->depth >= depth && rootHash[HASH_ALWAYS]->entryAge) {
            INC(collisions);
            if (smp)spinlockHashAlways.unlock();
            return;
        }
        memcpy(rootHash[HASH_ALWAYS], &tmp, sizeof(_Thash));
        if (smp)spinlockHashAlways.unlock();

    }

private:
    Hash();
    int HASH_SIZE;
#ifdef JS_MODE
    static const int HASH_SIZE_DEFAULT = 1;
#else
    static const int HASH_SIZE_DEFAULT = 64;
#endif

    void dispose();

    _Thash *hashArray[2];
    Spinlock spinlockHashGreater;
    Spinlock spinlockHashAlways;
};

