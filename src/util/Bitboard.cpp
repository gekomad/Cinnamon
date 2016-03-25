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

#include "Bitboard.h"

u64 Bitboard::BITMAP_SHIFT_DIAGONAL[64][256];
u64 Bitboard::BITMAP_SHIFT_ANTIDIAGONAL[64][256];
u64 Bitboard::BITMAP_SHIFT_FILE[64][256];
u64 Bitboard::BITMAP_SHIFT_RANK[64][256];

Bitboard::Bitboard() {

    u64 MASK_BIT_SET[64][64];
    memset(MASK_BIT_SET, 0, sizeof(MASK_BIT_SET));
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int a = min(i, j);
            int b = max(i, j);
            MASK_BIT_SET[i][i] = 0;
            for (int e = a; e <= b; e++) {
                u64 r = (RANK[i] | POW2[i]) & (RANK[j] | POW2[j]);
                if (r) {
                    MASK_BIT_SET[i][j] |= POW2[e] & r;
                } else {
                    r = (FILE_[i] | POW2[i]) & (FILE_[j] | POW2[j]);
                    if (r) {
                        MASK_BIT_SET[i][j] |= POW2[e] & r;
                    } else {
                        r = (DIAGONAL[i] | POW2[i]) & (DIAGONAL[j] | POW2[j]);
                        if (r) {
                            MASK_BIT_SET[i][j] |= POW2[e] & r;
                        } else {
                            r = (ANTIDIAGONAL[i] | POW2[i]) & (ANTIDIAGONAL[j] | POW2[j]);
                            if (r) {
                                MASK_BIT_SET[i][j] |= POW2[e] & r;
                            }
                        }
                    }
                }
            }
            if (i == j) {
                MASK_BIT_SET[i][i] &= NOTPOW2[i];
            }
        }
    }
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            MASK_BIT_SET_NOBOUND[i][j] = MASK_BIT_SET[i][j];
            MASK_BIT_SET_NOBOUND[i][j] &= NOTPOW2[i];
            MASK_BIT_SET_NOBOUND[i][j] &= NOTPOW2[j];
            MASK_BIT_SET[i][j] &= NOTPOW2[i];
        }
    }
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            MASK_BIT_SET_NOBOUND_COUNT[i][j] = Bits::bitCount(MASK_BIT_SET_NOBOUND[i][j]);
        }
    }

    genCombination();

    popolateAntiDiagonal();
    popolateDiagonal();
    popolateColumn();
    popolateRank();
    cout << dec;

}

void Bitboard::genCombination() {
    for (uchar pos = 0; pos < 64; pos++) {
        combinationsDiagonal[pos] = getCombination(_board::DIAGONAL[pos]);
        combinationsColumn[pos] = getCombination(_board::FILE_[pos]);
        combinationsRank[pos] = getCombination(_board::RANK[pos]);
        combinationsAntiDiagonal[pos] = getCombination(_board::ANTIDIAGONAL[pos]);
    }
}

void Bitboard::popolateDiagonal() {
    for (uchar pos = 0; pos < 64; pos++) {
        for (u64 allpieces:combinationsDiagonal[pos]) {
            uchar idx = diagonalIdx(pos, allpieces);
            BITMAP_SHIFT_DIAGONAL[pos][idx] = performDiagShift(pos, allpieces) | performDiagCapture(pos, allpieces);
        }
    }
}

void Bitboard::popolateColumn() {
    for (uchar pos = 0; pos < 64; pos++) {
        for (u64 allpieces:combinationsColumn[pos]) {
            uchar idx = fileIdx(pos, allpieces);
            BITMAP_SHIFT_FILE[pos][idx] = performColumnShift(pos, allpieces) | performColumnCapture(pos, allpieces);
        }

    }
}

void Bitboard::popolateRank() {
    for (uchar pos = 0; pos < 64; pos++) {
        for (u64 allpieces:combinationsRank[pos]) {
            uchar idx = rankIdx(pos, allpieces);
            BITMAP_SHIFT_RANK[pos][idx] = performRankShift(pos, allpieces) | performRankCapture(pos, allpieces);
        }
    }
}

void Bitboard::popolateAntiDiagonal() {
    for (uchar pos = 0; pos < 64; pos++) {
        for (u64 allpieces:combinationsAntiDiagonal[pos]) {
            uchar idx = antiDiagonalIdx(pos, allpieces);
            BITMAP_SHIFT_ANTIDIAGONAL[pos][idx] = performAntiDiagShift(pos, allpieces) | performAntiDiagCapture(pos, allpieces);
        }
    }
}

u64 Bitboard::performDiagShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_UP[position];
    u64 k = q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : _bitboardTmp::MASK_BIT_SET_LEFT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_DOWN[position];
    k |= q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : _bitboardTmp::MASK_BIT_SET_LEFT_UPPER[position];
    return k;

}

u64 Bitboard::performColumnShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_UP[position];
    u64 k = q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : _bitboardTmp::MASK_BIT_SET_VERT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_DOWN[position];
    k |= q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : _bitboardTmp::MASK_BIT_SET_VERT_UPPER[position];
    return k;
}


u64 Bitboard::performRankShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT[position];
    u64 k = q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : _bitboardTmp::MASK_BIT_SET_ORIZ_LEFT[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT[position];
    k |= q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : _bitboardTmp::MASK_BIT_SET_ORIZ_RIGHT[position];
    return k;
}

u64 Bitboard::performColumnCapture(const int position, const u64 allpieces) {
    u64 q;
    u64 k = 0;
    u64 x = allpieces & FILE_[position];
    if (x & allpieces) {
        q = x & _bitboardTmp::MASK_BIT_UNSET_UP[position];
        if (q && allpieces & POW2[Bits::BITScanReverse(q)]) {
            k |= POW2[Bits::BITScanReverse(q)];
        }
        q = x & _bitboardTmp::MASK_BIT_UNSET_DOWN[position];
        if (q && allpieces & POW2[Bits::BITScanForward(q)]) {
            k |= POW2[Bits::BITScanForward(q)];
        }
    }
    return k;
}

u64 Bitboard::performRankCapture(const int position, const u64 allpieces) {
    u64 q;
    u64 k = 0;
    u64 x = allpieces & RANK[position];
    if (x & allpieces) {
        q = x & _bitboardTmp::MASK_BIT_UNSET_LEFT[position];
        if (q && allpieces & POW2[Bits::BITScanReverse(q)]) {
            k |= POW2[Bits::BITScanReverse(q)];
        }
        q = x & _bitboardTmp::MASK_BIT_UNSET_RIGHT[position];
        if (q && allpieces & POW2[Bits::BITScanForward(q)]) {
            k |= POW2[Bits::BITScanForward(q)];
        }
    }
    return k;
}

u64 Bitboard::performAntiDiagCapture(const int position, const u64 allpieces) {
    int bound;
    u64 k = 0;
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_UP[position];
    if (q) {
        bound = Bits::BITScanReverse(q);
        if (allpieces & POW2[bound]) {
            k |= POW2[bound];
        }
    }
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_DOWN[position];
    if (q) {
        bound = Bits::BITScanForward(q);
        if (allpieces & POW2[bound]) {
            k |= POW2[bound];
        }
    }
    return k;
}


u64 Bitboard::performDiagCapture(const int position, const u64 allpieces) {
    u64 k = 0;
    int bound;
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_UP[position];
    if (q) {
        bound = Bits::BITScanReverse(q);
        if (allpieces & POW2[bound]) {
            k |= POW2[bound];
        }
    }
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_DOWN[position];
    if (q) {
        bound = Bits::BITScanForward(q);
        if (allpieces & POW2[bound]) {
            k |= POW2[bound];
        }
    }

    return k;
}

u64 Bitboard::performAntiDiagShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_UP[position];
    u64 k = q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : _bitboardTmp::MASK_BIT_SET_RIGHT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_DOWN[position];
    k |= q ? MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : _bitboardTmp::MASK_BIT_SET_RIGHT_UPPER[position];
    return k;
}

vector<u64> Bitboard::combinations(const vector<u64> &elems, int len, vector<int> &pos, int depth, int margin) {
    vector<u64> res;
    if (depth >= len) {
        for (int ii = 0; ii < pos.size(); ++ii) {
            res.push_back(elems[pos[ii]]);
        }
        return res;
    }

    if ((elems.size() - margin) < (len - depth))
        return res;

    for (int ii = margin; ii < elems.size(); ++ii) {
        pos[depth] = ii;
        vector<u64> A = combinations(elems, len, pos, depth + 1, ii + 1);
        res.insert(res.end(), A.begin(), A.end());
    }

    return res;
}

vector<u64>  Bitboard::combinations(const vector<u64> &elems, int len) {
    ASSERT(len > 0 && len <= elems.size());
    vector<int> positions(len, 0);
    return combinations(elems, len, positions, 0, 0);

}

vector<u64> Bitboard::getCombination(u64 elements) {
    vector<u64> res;
    while (elements) {
        int o = Bits::BITScanForward(elements);
        res.push_back(o);
        RESET_LSB(elements);
    }
    return getCombination(res);
}

vector<u64> Bitboard::getCombination(vector<u64> elements) {
    vector<u64> res;
    vector<u64> v;
    u64 bits = 0;

    for (int len = 1; len < elements.size() + 1; len++) {
        v = combinations(elements, len);
        int k = 0;
        for (int rr:v) {
            bits |= POW2[rr];
            if (++k == len) {
                res.push_back(bits);
                bits = 0;
                k = 0;

            }
        }
    }
    return res;
}



