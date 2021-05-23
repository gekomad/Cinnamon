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

u64 Bitboard::BITBOARD_DIAGONAL[64][256];
u64 Bitboard::BITBOARD_ANTIDIAGONAL[64][256];
u64 Bitboard::BITBOARD_FILE[64][256];
u64 Bitboard::BITBOARD_RANK[64][256];
volatile bool Bitboard::generated = false;
mutex Bitboard::mutexConstructor;

Bitboard::Bitboard() {
    std::lock_guard<std::mutex> lock(mutexConstructor);
    if (generated) {
        return;
    }
    tmpStruct = (_Ttmp *) malloc(sizeof(_Ttmp));
    u64 MASK_BIT_SET[64][64];
    memset(MASK_BIT_SET, 0, sizeof(MASK_BIT_SET));
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            int a = min(i, j);
            int b = max(i, j);
            MASK_BIT_SET[i][i] = 0;
            for (int e = a; e <= b; e++) {
                u64 r = (RANK[i] | POW2(i)) & (RANK[j] | POW2(j));
                if (r) {
                    MASK_BIT_SET[i][j] |= POW2(e) & r;
                } else {
                    r = (FILE_[i] | POW2(i)) & (FILE_[j] | POW2(j));
                    if (r) {
                        MASK_BIT_SET[i][j] |= POW2(e) & r;
                    } else {
                        r = (DIAGONAL[i] | POW2(i)) & (DIAGONAL[j] | POW2(j));
                        if (r) {
                            MASK_BIT_SET[i][j] |= POW2(e) & r;
                        } else {
                            r = (ANTIDIAGONAL[i] | POW2(i)) & (ANTIDIAGONAL[j] | POW2(j));
                            if (r) {
                                MASK_BIT_SET[i][j] |= POW2(e) & r;
                            }
                        }
                    }
                }
            }
            if (i == j) {
                MASK_BIT_SET[i][i] &= NOTPOW2(i);
            }
        }
    }
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            tmpStruct->MASK_BIT_SET_NOBOUND_TMP[i][j] = MASK_BIT_SET[i][j];
            tmpStruct->MASK_BIT_SET_NOBOUND_TMP[i][j] &= NOTPOW2(i);
            tmpStruct->MASK_BIT_SET_NOBOUND_TMP[i][j] &= NOTPOW2(j);
            MASK_BIT_SET[i][j] &= NOTPOW2(i);
        }
    }

    popolateAntiDiagonal();
    popolateDiagonal();
    popolateColumn();
    popolateRank();
    free(tmpStruct);
    tmpStruct = nullptr;
    generated = true;
}

void Bitboard::popolateDiagonal() {
    vector<u64> combinationsDiagonal;
    for (uchar pos = 0; pos < 64; pos++) {
        combinationsDiagonal = getCombination(constants::DIAGONAL[pos]);
        for (u64 allpieces:combinationsDiagonal) {
            uchar idx = diagonalIdx(pos, allpieces);
            BITBOARD_DIAGONAL[pos][idx] = performDiagShift(pos, allpieces) | performDiagCapture(pos, allpieces);
        }
    }
}

void Bitboard::popolateColumn() {
    vector<u64> combinationsColumn;
    for (uchar pos = 0; pos < 64; pos++) {
        combinationsColumn = getCombination(constants::FILE_[pos]);
        for (u64 allpieces:combinationsColumn) {
            uchar idx = fileIdx(pos, allpieces);
            BITBOARD_FILE[pos][idx] = performColumnShift(pos, allpieces) | performColumnCapture(pos, allpieces);
        }
    }
}

void Bitboard::popolateRank() {
    vector<u64> combinationsRank;
    for (uchar pos = 0; pos < 64; pos++) {
        combinationsRank = getCombination(constants::RANK[pos]);
        for (u64 allpieces:combinationsRank) {
            uchar idx = rankIdx(pos, allpieces);
            BITBOARD_RANK[pos][idx] = performRankShift(pos, allpieces) | performRankCapture(pos, allpieces);
        }
    }
}

void Bitboard::popolateAntiDiagonal() {
    vector<u64> combinationsAntiDiagonal;
    for (uchar pos = 0; pos < 64; pos++) {
        combinationsAntiDiagonal = getCombination(constants::ANTIDIAGONAL[pos]);
        for (u64 allpieces:combinationsAntiDiagonal) {
            uchar idx = antiDiagonalIdx(pos, allpieces);
            BITBOARD_ANTIDIAGONAL[pos][idx] =
                performAntiDiagShift(pos, allpieces) | performAntiDiagCapture(pos, allpieces);
        }
    }
}

u64 Bitboard::performDiagShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_UP[position];
    u64 k = q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanReverse(q)]
              : _bitboardTmp::MASK_BIT_SET_LEFT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_DOWN[position];
    k |= q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanForward(q)]
           : _bitboardTmp::MASK_BIT_SET_LEFT_UPPER[position];
    return k;

}

u64 Bitboard::performColumnShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_UP[position];
    u64 k = q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanReverse(q)]
              : _bitboardTmp::MASK_BIT_SET_VERT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_DOWN[position];
    k |= q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanForward(q)]
           : _bitboardTmp::MASK_BIT_SET_VERT_UPPER[position];
    return k;
}


u64 Bitboard::performRankShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT[position];
    u64 k = q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanForward(q)]
              : _bitboardTmp::MASK_BIT_SET_ORIZ_LEFT[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT[position];
    k |= q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanReverse(q)]
           : _bitboardTmp::MASK_BIT_SET_ORIZ_RIGHT[position];
    return k;
}

u64 Bitboard::performAntiDiagShift(const int position, const u64 allpieces) {
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_UP[position];
    u64 k = q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanReverse(q)]
              : _bitboardTmp::MASK_BIT_SET_RIGHT_LOWER[position];
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_DOWN[position];
    k |= q ? tmpStruct->MASK_BIT_SET_NOBOUND_TMP[position][BITScanForward(q)]
           : _bitboardTmp::MASK_BIT_SET_RIGHT_UPPER[position];
    return k;
}

u64 Bitboard::performColumnCapture(const int position, const u64 allpieces) {
    u64 q;
    u64 k = 0;
    u64 x = allpieces & FILE_[position];
    q = x & _bitboardTmp::MASK_BIT_UNSET_UP[position];
    if (q && allpieces & POW2(BITScanReverse(q))) {
        k |= POW2(BITScanReverse(q));
    }
    q = x & _bitboardTmp::MASK_BIT_UNSET_DOWN[position];
    if (q && allpieces & POW2(BITScanForward(q))) {
        k |= POW2(BITScanForward(q));
    }
    return k;
}

u64 Bitboard::performRankCapture(const int position, const u64 allpieces) {
    u64 q;
    u64 k = 0;
    u64 x = allpieces & RANK[position];
    q = x & _bitboardTmp::MASK_BIT_UNSET_LEFT[position];
    if (q && allpieces & POW2(BITScanReverse(q))) {
        k |= POW2(BITScanReverse(q));
    }
    q = x & _bitboardTmp::MASK_BIT_UNSET_RIGHT[position];
    if (q && allpieces & POW2(BITScanForward(q))) {
        k |= POW2(BITScanForward(q));
    }
    return k;
}

u64 Bitboard::performAntiDiagCapture(const int position, const u64 allpieces) {
    int bound;
    u64 k = 0;
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_UP[position];
    if (q) {
        bound = BITScanReverse(q);
        if (allpieces & POW2(bound)) {
            k |= POW2(bound);
        }
    }
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_RIGHT_DOWN[position];
    if (q) {
        bound = BITScanForward(q);
        if (allpieces & POW2(bound)) {
            k |= POW2(bound);
        }
    }
    return k;
}


u64 Bitboard::performDiagCapture(const int position, const u64 allpieces) {
    u64 k = 0;
    int bound;
    u64 q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_UP[position];
    if (q) {
        bound = BITScanReverse(q);
        if (allpieces & POW2(bound)) {
            k |= POW2(bound);
        }
    }
    q = allpieces & _bitboardTmp::MASK_BIT_UNSET_LEFT_DOWN[position];
    if (q) {
        bound = BITScanForward(q);
        if (allpieces & POW2(bound)) {
            k |= POW2(bound);
        }
    }

    return k;
}

vector<u64> Bitboard::combinations(const vector<u64> &elems,
                                   const int len,
                                   vector<int> &pos,
                                   const int depth,
                                   const int margin) {
    vector<u64> res;
    if (depth >= len) {
        for (int po : pos) {
            res.push_back(elems[po]);
        }
        return res;
    }

    if (((int) elems.size() - margin) < (len - depth))
        return res;

    for (unsigned ii = margin; ii < elems.size(); ++ii) {
        pos[depth] = ii;
        vector<u64> A = combinations(elems, len, pos, depth + 1, ii + 1);
        res.insert(res.end(), A.begin(), A.end());
    }

    return res;
}

vector<u64>  Bitboard::combinations(const vector<u64> &elems, const int len) {
    vector<int> positions(len, 0);
    return combinations(elems, len, positions, 0, 0);

}

vector<u64> Bitboard::getCombination(u64 elements) {
    vector<u64> res;
    for (; elements; RESET_LSB(elements)) {
        const int o = BITScanForward(elements);
        res.push_back(o);
    }
    return getCombination(res);
}

vector<u64> Bitboard::getCombination(const vector<u64> elements) {
    vector<u64> res;
    vector<u64> v;
    u64 bits = 0;

    for (unsigned len = 1; len < elements.size() + 1; len++) {
        v = combinations(elements, len);
        unsigned k = 0;
        for (int rr:v) {
            bits |= POW2(rr);
            if (++k == len) {
                res.push_back(bits);
                bits = 0;
                k = 0;
            }
        }
    }
    return res;
}
