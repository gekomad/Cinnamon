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

#include "Bits.h"

//array<array<uchar, 64>, 64> Bits::DISTANCE;

Bits::Bits() {
    //LINK_ROOKS
    LINK_ROOKS = (u64 **) malloc(64 * sizeof(u64 *));
    for (int i = 0; i < 64; i++) {
        LINK_ROOKS[i] = (u64 *) malloc(64 * sizeof(u64));
    }
    int from, to;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            u64 t = 0;
            if (RANK[i] & RANK[j]) {    //rank
                from = min(i, j);
                to = max(i, j);
                for (int k = from + 1; k <= to - 1; k++) {
                    t |= POW2[k];
                }
            } else if (FILE_[i] & FILE_[j]) {    //file
                from = min(i, j);
                to = max(i, j);
                for (int k = from + 8; k <= to - 8; k += 8) {
                    t |= POW2[k];
                }
            }
            if (!t) {
                t = 0xffffffffffffffffULL;
            }
            LINK_ROOKS[i][j] = t;
        }
    }
    //DISTANCE
    //for (int i = 0; i < 64; i++) {
    //    for (int j = 0; j < 64; j++) {
    //        DISTANCE[i][j] = max(abs(RANK_AT[i] - FILE_AT[j]), abs(RANK_AT[j] - FILE_AT[i]));
    //    }
    //}
    ///

}

Bits::~Bits() {
    for (int i = 0; i < 64; i++) {
        free(LINK_ROOKS[i]);
    }
    free(LINK_ROOKS);
}
