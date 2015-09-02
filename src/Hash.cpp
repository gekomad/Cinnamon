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

#include <mutex>
#include "Hash.h"

Hash::Hash() {
    HASH_SIZE = 0;
    hashArray[HASH_GREATER] = hashArray[HASH_ALWAYS] = nullptr;
#ifdef DEBUG_MODE
    n_cut_hashA = n_cut_hashB = cutFailed = probeHash = 0;
    nRecordHashA = nRecordHashB = nRecordHashE = collisions = 0;
#endif
    setHashSize(64);
}

void Hash::setSMP(int smp) {
    if (smp > 1) {
        MUTEX_BUCKET = (shared_timed_mutex **) malloc(sizeof(shared_timed_mutex) * 2);
        MUTEX_BUCKET[0] = (shared_timed_mutex *) malloc(sizeof(shared_timed_mutex) * N_MUTEX_BUCKET);
        MUTEX_BUCKET[1] = (shared_timed_mutex *) malloc(sizeof(shared_timed_mutex) * N_MUTEX_BUCKET);
    }
    else {
        if (MUTEX_BUCKET) {
            free(MUTEX_BUCKET[0]);
            free(MUTEX_BUCKET[1]);
            free(MUTEX_BUCKET);
            MUTEX_BUCKET = nullptr;
        }
    }
}

void Hash::clearAge() {
    for (int i = 0; i < HASH_SIZE; i++) {
        hashArray[HASH_GREATER][i].entryAge = 0;
    }
}

void Hash::clearHash() {
    if (!HASH_SIZE) {
        return;
    }
    memset(hashArray[HASH_ALWAYS], 0, sizeof(_Thash) * HASH_SIZE);
    memset(hashArray[HASH_GREATER], 0, sizeof(_Thash) * HASH_SIZE);
}

int Hash::getHashSize() {
    return HASH_SIZE / (1024 * 1000 / (sizeof(_Thash) * 2));
}

bool Hash::setHashSize(int mb) {
    if (mb < 1 || mb > 32768) {
        return false;
    }
    dispose();
    if (mb) {
        HASH_SIZE = mb * 1024 * 1000 / (sizeof(_Thash) * 2);
        hashArray[HASH_GREATER] = (_Thash *) calloc(HASH_SIZE, sizeof(_Thash));
        assert(hashArray[HASH_GREATER]);
        hashArray[HASH_ALWAYS] = (_Thash *) calloc(HASH_SIZE, sizeof(_Thash));
        assert(hashArray[HASH_ALWAYS]);
    }
    return true;
}

void Hash::dispose() {
    if (hashArray[HASH_GREATER]) {
        free(hashArray[HASH_GREATER]);
    }
    if (hashArray[HASH_ALWAYS]) {
        free(hashArray[HASH_ALWAYS]);
    }
    hashArray[HASH_GREATER] = hashArray[HASH_ALWAYS] = nullptr;
    HASH_SIZE = 0;
}

Hash::~Hash() {
    dispose();
}


