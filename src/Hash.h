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

    typedef union _ThashData {
        u64 dataU;

        struct __dataS {
            short score;
            char depth;
            uchar from;
            uchar to;
            uchar entryAge;
            uchar flags;

            __dataS() { };

            __dataS(const short score, const char depth, const uchar from, const uchar to, const uchar entryAge, const uchar flags) :
                    score(score), depth(depth), from(from), to(to), entryAge(entryAge), flags(flags) { };
        } dataS;

        _ThashData() { };

        _ThashData(const short score, const char depth, const uchar from, const uchar to, const uchar entryAge, const uchar flags) :
                dataS(score, depth, from, to, entryAge, flags) { };
    } __Tdata;

    typedef struct {
        u64 key;
        _ThashData u;
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

    template<int type>
    u64 readHash(const u64 zobristKeyR) const {
        _Thash *hash = &(hashArray[type][zobristKeyR % HASH_SIZE]);
        u64 data = hash->u.dataU;
        u64 k = hash->key;
        if (zobristKeyR == (k ^ data)) {
            return data;
        }
        return 0;
    }

    void recordHash(const u64 zobristKey, _ThashData &tmp) {
        ASSERT(zobristKey);

        int kMod = zobristKey % HASH_SIZE;
        _Thash *rootHashG = &(hashArray[HASH_GREATER][kMod]);

        rootHashG->key = (zobristKey ^ tmp.dataU);
        rootHashG->u.dataU = tmp.dataU;

#ifdef DEBUG_MODE
        if (tmp.dataS.flags == hashfALPHA) {
            nRecordHashA++;
        } else if (tmp.dataS.flags == hashfBETA) {
            nRecordHashB++;
        } else {
            nRecordHashE++;
        }
#endif

        _Thash *rootHashA = &(hashArray[HASH_ALWAYS][kMod]);

        if (rootHashA->u.dataS.depth >= tmp.dataS.depth && rootHashA->u.dataS.entryAge) {
            INC(collisions);
            return;
        }
        tmp.dataS.entryAge = 1;
        rootHashA->key = (zobristKey ^ tmp.dataU);
        rootHashA->u.dataU = tmp.dataU;

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
    static mutex mutexConstructor;

};

