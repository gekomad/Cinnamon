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
#include <mutex>

using namespace _board;
using namespace _logger;

class Hash {

public:
    Hash();

    static const int HASH_GREATER = 0;
    static const int HASH_ALWAYS = 1;

    typedef union {
        u64 dataU;
        struct {
            short score;
            char depth;
            uchar from:6;
            uchar to:6;
            uchar entryAge:1;
            uchar flags:2;
        } dataS;
    } _Tdata;

    typedef struct {
        u64 key;
        _Tdata u;
    } _Thash;

    enum : char {
        hashfALPHA = 0, hashfEXACT = 1, hashfBETA = 2
    };

#ifdef DEBUG_MODE
    unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;

    int n_cut_hashA, n_cut_hashB, cutFailed, probeHash;
#endif

    ~Hash();

    void setHashSize(int mb);

    int getHashSize() const;

    void clearHash();

    void clearAge();

    template<bool smp, int type>
    bool readHash(const u64 zobristKeyR, u64 *hashMini) const {
        //bool b = false;
        _Thash *hash = &(hashArray[type][zobristKeyR % HASH_SIZE]);
        *hashMini = hash->u.dataU;
        if (smp) {
//            if (type == HASH_GREATER)spinlockHashGreater.lock();
//            if (type == HASH_ALWAYS)spinlockHashAlways.lock();
            u64 k = hash->key;

            if (zobristKeyR == (k ^ *hashMini)) {
                return true;
//                memcpy(hashMini, hash, sizeof(_Thash));
            }
//            if (type == HASH_GREATER)spinlockHashGreater.unlock();
//            if (type == HASH_ALWAYS)spinlockHashAlways.unlock();

        } else {
            if (zobristKeyR == (hash->key ^ *hashMini)) {

                *hashMini = hash->u.dataU;
                return true;
//                memcpy(hashMini, hash, sizeof(_Thash));
            }

        }

        return false;
    }

    template<bool smp>
    void recordHash(const bool running, const char depth, const char flags, const u64 key, const int score, const _Tmove *bestMove) {
        ASSERT(key);

        if (!running) {
            return;
        }
        ASSERT(abs(score) <= 32200);
        _Thash tmp;

//        tmp.key = key;
        tmp.u.dataS.score = score;
        tmp.u.dataS.flags = flags;
        tmp.u.dataS.depth = depth;
        if (bestMove && bestMove->from != bestMove->to) {
            tmp.u.dataS.from = bestMove->from;
            tmp.u.dataS.to = bestMove->to;
        } else {
            tmp.u.dataS.from = tmp.u.dataS.to = 0;
        }

        _Thash *rootHashG = &(hashArray[HASH_GREATER][key % HASH_SIZE]);
//        if (smp)spinlockHashGreater.lock();
        rootHashG->key = (key ^ tmp.u.dataU);
        rootHashG->u.dataU = tmp.u.dataU;
//        memcpy(rootHash[HASH_GREATER], &tmp, sizeof(_Thash));
//        if (smp)spinlockHashGreater.unlock();


#ifdef DEBUG_MODE
        if (flags == hashfALPHA) {
            nRecordHashA++;
        } else if (flags == hashfBETA) {
            nRecordHashB++;
        } else {
            nRecordHashE++;
        }
#endif
        tmp.u.dataS.entryAge = 1;

//        if (smp)spinlockHashAlways.lock();
        _Thash *rootHashA = &(hashArray[HASH_ALWAYS][key % HASH_SIZE]);
        if (rootHashA->key && rootHashA->u.dataS.depth >= depth && rootHashA->u.dataS.entryAge) {//TODO eliminare prima condizone
            INC(collisions);
//            if (smp)spinlockHashAlways.unlock();
            return;
        }
        rootHashA->key = (key ^ tmp.u.dataU);
        rootHashA->u.dataU = tmp.u.dataU;
//        memcpy(rootHash[HASH_ALWAYS], &tmp, sizeof(_Thash));
//        if (smp)spinlockHashAlways.unlock();

    }

private:
    static bool generated;
    static int HASH_SIZE;
#ifdef JS_MODE
    static const int HASH_SIZE_DEFAULT = 1;
#else
    static const int HASH_SIZE_DEFAULT = 64;
#endif

    void dispose();

    static _Thash *hashArray[2];
//    static Spinlock spinlockHashGreater;
//    static Spinlock spinlockHashAlways;
    static mutex mutexConstructor;

};

