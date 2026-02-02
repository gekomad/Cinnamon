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
        // 123456789ABCDEF|12345678|12345678|12345678|12345678|0123456789ABCDEF|
        // age            | flags  | from   |   to   | depth  |     score      |
        u64 data;

        _Thash(const u64 zobristKeyR, const short score, const char depth, const uchar from, const uchar to,
               const uchar flags) {
            key = zobristKeyR;
            data = (u64)(uint16_t)score;
            data |= (u64) depth << 16;
            data |= (u64) to << (16 + 8);
            data |= (u64) from << (16 + 8 + 8);
            data |= (u64) flags << (16 + 8 + 8 + 8);
        }
    } _Thash;

    enum : char {
        hashfALPHA = 0, hashfEXACT = 1, hashfBETA = 2
    };

#ifdef DEBUG_MODE
    static unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions, readCollisions, n_cut_hashA, n_cut_hashB, n_cut_hashE, readHashCount;
#endif

    static void setHashSize(const int mb);

    static void clearHash();


    static  __attribute__((always_inline)) void SET_AGE(u64& u, const int v) { u |= static_cast<u64>(v) << (16 + 8 + 8 + 8 + 8); }
    static  __attribute__((always_inline)) uchar GET_DEPTH(const u64 v) {return v>>16 ;}
    static  __attribute__((always_inline)) uchar GET_FLAGS(const u64 v) {return v>>(16 + 8 + 8 + 8) ;}
    static  __attribute__((always_inline)) uchar GET_FROM(const u64 v) {return v>>(16 + 8 + 8) ;}
    static  __attribute__((always_inline)) uchar GET_TO(const u64 v) {return v>>(16 + 8) ;}
    static  __attribute__((always_inline)) short GET_SCORE(const u64 v) {return v ;}
    static  __attribute__((always_inline)) unsigned short GET_AGE(const u64 v) {return v >> (16 + 8 + 8 + 8 + 8);}
    static  __attribute__((always_inline)) u64 GET_KEY(const _Thash *hash) {return hash->key;}
    static  __attribute__((always_inline)) void INC_AGE(u64& u) { SET_AGE(u, GET_AGE(u)+1);}

    static inline int readHash(
            int &alpha,
            int &beta,
            const int depth,
            const u64 zobristKeyR,
            u64 &hashStruct,
            const bool currentPly) {
 
        INC(readHashCount);
        Hash::_Thash *hash = &(hashArray[zobristKeyR & (HASH_SIZE - 1)]);
        DEBUG(u64 d = 0)
        hashStruct = 0;
        bool found = false;
        for (int i = 0; i < BUCKETS; i++, hash++) {
            if (found)break;
            const u64 data = hash->data;
            DEBUG(d |= data)
            if (zobristKeyR == GET_KEY(hash)) {
                found = true;
                INC_AGE(hash->data);
                hashStruct = data;
                if (currentPly && GET_DEPTH(hashStruct) >= depth) {
                    const int ttScore = GET_SCORE(hashStruct);
                    switch (GET_FLAGS(hashStruct)) {
                        case Hash::hashfEXACT: {
                            INC(n_cut_hashE);
                            return ttScore;
                        }
                        case Hash::hashfBETA:
                            if (ttScore >= beta) {
                                INC(n_cut_hashB);
                                return beta;
                            }
                            if (ttScore > alpha)
                                alpha = ttScore;
                            break;
                        case Hash::hashfALPHA:
                            if (ttScore <= alpha) {
                                INC(n_cut_hashA);
                                return alpha;
                            }
                            if (ttScore < beta)
                                beta = ttScore;
                            break;
                        default:
                            fatal("Error checkHash")
                            exit(1);
                    }
                    if (alpha >= beta)
                        return ttScore;
                }
            }
        }
        
        DEBUG(if (d && !found)readCollisions++)
        return INT_MAX;
    }

    static void recordHash(const _Thash &toStore, const int age) {
#ifdef DEBUG_MODE
        ASSERT(toStore.key);
        if (GET_FLAGS(toStore.data) == hashfALPHA) nRecordHashA++;
        else if (GET_FLAGS(toStore.data) == hashfBETA) nRecordHashB++;
        else nRecordHashE++;
        ASSERT(GET_DEPTH(toStore.data) < MAX_PLY);
#endif
        const unsigned kMod = toStore.key & (HASH_SIZE - 1);
        _Thash *empty = nullptr;
        { // update
            _Thash *hash = &(hashArray[kMod]);
            bool found = false;
            for (int i = 0; i < BUCKETS; i++, hash++) {
                const u64 data = hash->data;
                if (toStore.key == GET_KEY(hash)) {
                    found = true;
                    if (GET_DEPTH(data) <= GET_DEPTH(toStore.data)) {
                        // hash->key = (toStore.key ^ toStore.data);
                        hash->key = (toStore.key);
                        hash->data = toStore.data;
                        SET_AGE(hash->data, age);
                        return;
                    }
                } else if (!hash->key) {
                    empty = hash;
                    if (found)
                        break;
                }
            }
        }
        if (empty) { //empty slot
            // empty->key = (toStore.key ^ toStore.data);
            empty->key = (toStore.key);
            empty->data = toStore.data;
            SET_AGE(empty->data, age);
            return;
        }
       
        { // age
            _Thash *hash = &hashArray[kMod];
            _Thash *old  = hash;
            int best = INT_MAX;
            for (int i = 0; i < BUCKETS; i++, hash++) {
                assert (hash->key);
                const u64 data = hash->data;
                const int score = GET_AGE(data);// ((age - GET_AGE(data)) & 255) * 256 + (255 - GET_DEPTH(data));
                if (score < best) {
                    best = score;
                    old = hash;
                }
            }
            old->key  = toStore.key;
            old->data = toStore.data;
            SET_AGE(old->data, age);
        }
    }

private:
    Hash();
    ~Hash();
    static void dispose();
    static constexpr int BUCKETS = 3;
    static unsigned HASH_SIZE;
#ifdef JS_MODE
    static constexpr int HASH_SIZE_DEFAULT = 1;
#else
    static constexpr int HASH_SIZE_DEFAULT = 64; // TODO 128
#endif


    static _Thash *hashArray;
};

