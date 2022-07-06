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

#include "Hash.h"

unsigned Hash::HASH_SIZE;
Hash::_Thash *Hash::hashArray;
#ifndef NDEBUG
unsigned Hash::nRecordHashA, Hash::nRecordHashB, Hash::nRecordHashE, Hash::collisions, Hash::readCollisions,
        Hash::n_cut_hashA, Hash::n_cut_hashB, Hash::n_cut_hashE, Hash::readHashCount;
#endif

Hash::Hash() {
    HASH_SIZE = 0;
    hashArray = nullptr;
    DEBUG(n_cut_hashA = n_cut_hashB = nRecordHashA = nRecordHashB = nRecordHashE = readCollisions = collisions = 0)
    setHashSize(HASH_SIZE_DEFAULT);
}

void Hash::clearHash() {
    if (!HASH_SIZE) return;
    memset(static_cast<void *>(hashArray), 0, sizeof(_Thash) * (HASH_SIZE + BUCKETS));
}

void Hash::setHashSize(const int mb) {
    dispose();
    if (mb > 0) {
        u64 tmp = (u64) mb * 1024 * 1024 / (sizeof(_Thash));
        hashArray = (_Thash *) calloc(tmp, sizeof(_Thash));
        if (!hashArray) {
            fatal("info string error - no memory")
            exit(1);
        }
        HASH_SIZE = tmp - BUCKETS;
    }
}

void Hash::dispose() {
    if (hashArray != nullptr) free(hashArray);
    hashArray = nullptr;
    HASH_SIZE = 0;
}


