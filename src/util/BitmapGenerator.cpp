#include "BitmapGenerator.h"

BitMapGenerator::BitMapGenerator() {
    Bits::getInstance();
    popolateDiagonal();
    popolateAntiDiagonal();
}

void BitMapGenerator::popolateDiagonal() {
    vector<u64> elements;
    for (int pos = 0; pos < 64; pos++) {
        elements.clear();
        u64 diag =_board::LEFT_DIAG[pos] | POW2[pos];
        while(diag) {
            int o = Bits::BITScanForward(diag);
            elements.push_back(o);
            RESET_LSB(diag);
        }
        vector<vector<u64>> res = getPermutation(elements);
        u64 allpieces = 0;
        for (int i = 0; i < res.size(); i++) {
            for (int y:res[i]) {
                _assert(y < 64);
                allpieces |= POW2[y];
            }

            uchar idx = diagonalIdx(pos, allpieces);
            u64 mapDiag = performDiagShift(pos, allpieces);
            cout << "ROTATE_BITMAP_DIAGONAL[pos:" << pos << "][idx:" << (int) idx << "]=" << "0x" << mapDiag << "ULL (allpieces: " << hex << "0x" << allpieces << "ULL)\n";
            Bits::ROTATE_BITMAP_DIAGONAL[pos][idx] = mapDiag;
        }
    }
}

void BitMapGenerator::popolateAntiDiagonal() {
    vector<u64> elements;
    for (int pos = 0; pos < 64; pos++) {
        elements.clear();
        u64 diag =_board::RIGHT_DIAG[pos] | POW2[pos];
        while(diag) {
            int o = Bits::BITScanForward(diag);
            elements.push_back(o);
            RESET_LSB(diag);
        }
        vector<vector<u64>> res = getPermutation(elements);
        u64 allpieces = 0;
        for (int i = 0; i < res.size(); i++) {
            for (int y:res[i]) {
                _assert(y < 64);
                allpieces |= POW2[y];
            }

            uchar idx = antiDiagonalIdx(pos, allpieces);
            u64 mapDiag = performAntiDiagShift(pos, allpieces);
            cout << "ROTATE_BITMAP_ANTIDIAGONAL[pos:" << pos << "][idx:" << (int) idx << "]=" << "0x" << mapDiag << "ULL (allpieces: " << hex << "0x" << allpieces << "ULL)\n";
            Bits::ROTATE_BITMAP_ANTIDIAGONAL[pos][idx] = mapDiag;
        }
    }
}


u64 BitMapGenerator::performDiagShift(const int position, const u64 allpieces) {
    /*
        LEFT
             /
            /
           /
*/
    u64 q = allpieces & MASK_BIT_UNSET_LEFT_UP[position];
    u64 k = q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_LEFT_LOWER[position];
    q = allpieces & MASK_BIT_UNSET_LEFT_DOWN[position];
    k |= q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_LEFT_UPPER[position];
    return k;

}

u64 BitMapGenerator::performAntiDiagShift(const int position, const u64 allpieces) {
    /*
        RIGHT
        \
         \
          \
*/
    u64 q = allpieces & MASK_BIT_UNSET_RIGHT_UP[position];
    u64 k = q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanReverse(q)] : MASK_BIT_SET_RIGHT_LOWER[position];
    q = allpieces & MASK_BIT_UNSET_RIGHT_DOWN[position];
    k |= q ? Bits::MASK_BIT_SET_NOBOUND[position][Bits::BITScanForward(q)] : MASK_BIT_SET_RIGHT_UPPER[position];
    return k;
}

uchar BitMapGenerator::diagonalIdx(const int position, u64 allpieces) {
    const u64 File = 0x8080808080808080ull;//FILE_[position];
    u64 diagonalMaskEx_sq = _board::LEFT_DIAG[position] | POW2[position];
    allpieces = ((diagonalMaskEx_sq & allpieces) * File) >> 56;
    return allpieces;
}


uchar BitMapGenerator::antiDiagonalIdx(const int position, u64 allpieces) {
    const u64 File = 0x8080808080808080ull;//FILE_[position];
    u64 diagonalMaskEx_sq = _board::RIGHT_DIAG[position] | POW2[position];
    allpieces = ((diagonalMaskEx_sq & allpieces) * File) >> 56;
    return allpieces;
}


vector<u64> BitMapGenerator::combinations_recursive(const vector<u64> &elems, unsigned long req_len,
                                                    vector<unsigned long> &pos, unsigned long depth,
                                                    unsigned long margin) {
    vector<u64> res;
    // Have we selected the number of required elements?
    if (depth >= req_len) {
        for (unsigned long ii = 0; ii < pos.size(); ++ii) {
            res.push_back(elems[pos[ii]]);
//            cout <<"X"<< elems[pos[ii]];
        }
//        cout << endl;
        return res;
    }

    // Are there enough remaining elements to be selected?
    // This test isn't required for the function to be correct, but
    // it can save a good amount of futile function calls.
    if ((elems.size() - margin) < (req_len - depth))
        return res;

    // Try to select new elements to the right of the last selected one.
    for (unsigned long ii = margin; ii < elems.size(); ++ii) {
        pos[depth] = ii;

        vector<u64> A = combinations_recursive(elems, req_len, pos, depth + 1, ii + 1);
        res.insert(res.end(), A.begin(), A.end());
    }

    return res;
}

vector<u64>  BitMapGenerator::combinations(const vector<u64> &elems, unsigned long comb_len) {
    assert(comb_len > 0 && comb_len <= elems.size());
    vector<unsigned long> positions(comb_len, 0);
    return combinations_recursive(elems, comb_len, positions, 0, 0);

}


vector<vector<u64>> BitMapGenerator::getPermutation(vector<u64> elements) {
    vector<vector<u64>> res(256);
    vector<u64> v;
    int nVector = 0;
    for (int comb_len = 1; comb_len < elements.size() + 1; comb_len++) {
        v = combinations(elements, comb_len);
        int k = 0;

        for (int rr:v) {
            //  cout << "(" << rr << ")";
          //  cout << nVector << endl;
            res[nVector].push_back(rr);
            if (++k == comb_len) {
                k = 0;
                nVector++;
                assert(nVector < 256);
//                res.resize(nVector);
                // cout << "\n";
            }
        }
        //  cout << "\n";
    }
    res.resize(nVector);
    return res;
}



