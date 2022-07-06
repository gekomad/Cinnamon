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
#include <cstring>
#include "namespaces/bits.h"
#include "namespaces/constants.h"
#include "util/Singleton.h"
#include "util/logger.h"
#include <limits.h>

using namespace constants;
using namespace _logger;

class Hash : public Singleton<Hash> {
    friend class Singleton<Hash>;

public:

    typedef struct _Thash {
        u64 key;
        // 123456789ABCDEF|12345678|12345678|12345678|12345678|123456789ABCDEF|
        // age            | flags  | from   |   to   | depth  |    score      |
        u64 data;

        _Thash(const u64 zobristKeyR, const short score, const char depth, const uchar from, const uchar to,
               const uchar flags) {
            key = zobristKeyR;
            data = score;
            data &= 0xffffULL;
            data |= (u64) depth << 16;
            data |= (u64) to << (16 + 8);
            data |= (u64) from << (16 + 8 + 8);
            data |= (u64) flags << (16 + 8 + 8 + 8);
        }
    } _Thash;

    enum : char {
        hashfALPHA = 0, hashfEXACT = 1, hashfBETA = 2
    };

#ifndef NDEBUG
    static unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions, readCollisions, n_cut_hashA, n_cut_hashB, n_cut_hashE, readHashCount;
#endif

    static void setHashSize(const int mb);

    static void clearHash();

#define SET_AGE(u, v) (u=(u&0xffffffffffffULL)|(((u64)v)<<(16 + 8 + 8 + 8 + 8)))
#define GET_DEPTH(v) ((uchar)(v>>16))
#define GET_FLAGS(v) ((uchar)(v>>(16 + 8 + 8 + 8)))
#define GET_FROM(v) ((uchar)(v>>(16 + 8 + 8)))
#define GET_TO(v) ((uchar)(v>>(16 + 8)))
#define GET_SCORE(v) ((short ) v)
#define GET_AGE(v) ((unsigned short)(v>> (16 + 8 + 8 + 8 + 8)))
#define GET_KEY(hash) (hash->key ^ (hash->data & 0xffffffffffffULL))

    static inline int readHash(
            const int alpha,
            const int beta,
            const int depth,
            const u64 zobristKeyR,
            u64 &hashStruct,
            const bool currentPly) {
        INC(readHashCount);
        const Hash::_Thash *hash = &(hashArray[zobristKeyR % HASH_SIZE]);
        DEBUG(u64 d = 0)
        hashStruct = 0;
        bool found = false;
        for (int i = 0; i < BUCKETS; i++, hash++) {
            if (found)break;
            u64 data = hash->data;
            DEBUG(d |= data)
            if (zobristKeyR == GET_KEY(hash)) {
                found = true;
                hashStruct = data;
                if (GET_DEPTH(hashStruct) >= depth) {
                    if (currentPly) {
                        switch (GET_FLAGS(hashStruct)) {
                            case Hash::hashfEXACT:
                            case Hash::hashfBETA:
                                if (GET_SCORE(hashStruct) >= beta) {
                                    INC(n_cut_hashB);
                                    return beta;
                                }
                                break;
                            case Hash::hashfALPHA:
                                if (GET_SCORE(hashStruct) <= alpha) {
                                    INC(n_cut_hashA);
                                    return alpha;
                                }
                                break;
                            default:
                                fatal("Error checkHash")
                                exit(1);
                        }
                    }
                }
            }
        }
        DEBUG(if (d && !found)readCollisions++)
        return INT_MAX;
    }

    static void recordHash(const _Thash &toStore, const int ply) {
#ifndef NDEBUG
        assert(toStore.key);
        if (GET_FLAGS(toStore.data) == hashfALPHA) nRecordHashA++;
        else if (GET_FLAGS(toStore.data) == hashfBETA) nRecordHashB++;
        else nRecordHashE++;
#endif
        assert(GET_DEPTH(toStore.data) < MAX_PLY);
        const unsigned kMod = toStore.key % HASH_SIZE;

        _Thash *empty = nullptr;

        { // update
            _Thash *hash = &(hashArray[kMod]);
            bool found = false;
            for (int i = 0; i < BUCKETS; i++, hash++) {
                const u64 data = hash->data;
                if (toStore.key == GET_KEY(hash)) {
                    found = true;
                    if (GET_DEPTH(data) <= GET_DEPTH(toStore.data)) {
                        hash->key = (toStore.key ^ toStore.data);
                        hash->data = toStore.data;
                        SET_AGE(hash->data, ply);
                        return;
                    }
                } else if (!hash->key) {
                    empty = hash;
                    if (found)break;
                }
            }
        }
        if (empty) { //empty slot
            empty->key = (toStore.key ^ toStore.data);
            empty->data = toStore.data;
            SET_AGE(empty->data, ply);
            return;
        }

        { // age
            _Thash *hash = &(hashArray[kMod]);
            _Thash *old = &(hashArray[kMod]);
            int i;
            int oldTT = -INT_MAX;
            for (i = 0; i < BUCKETS; i++, hash++) {
                const u64 data = hash->data;
                const auto age = ((ply - GET_AGE(data)) & 255) * 256 + 255 - GET_DEPTH(data);
                // const int age = ((pow(GET_DEPTH(data) - GET_DEPTH(old->data), 2)) + (ply - GET_AGE(data)));
                if (age > oldTT) {
                    old = hash;
                    oldTT = age;
                }
            }
            if (i == BUCKETS) hash = old;

            DEBUG(if (hash->key && hash->key != (toStore.key ^ toStore.data)) INC(collisions))
            hash->key = (toStore.key ^ toStore.data);
            hash->data = toStore.data;
            SET_AGE(hash->data, ply);
        }
    }

private:
    Hash();

    static constexpr int BUCKETS = 3;
    static unsigned HASH_SIZE;
#ifdef JS_MODE
    static constexpr int HASH_SIZE_DEFAULT = 1;
#else
    static constexpr int HASH_SIZE_DEFAULT = 64;
#endif

    static void dispose();

    static _Thash *hashArray;
};

