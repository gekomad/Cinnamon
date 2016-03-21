#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include "../namespaces/def.h"
#include "Bits.h"
#include "Random.h"

using namespace _def;
using std::copy;
using std::cout;
using std::endl;
using std::vector;

class BitmapGenerator {
public:
    BitmapGenerator();
    static u64 ROTATE_BITMAP_DIAGONAL[64][256];
    static u64 ROTATE_BITMAP_ANTIDIAGONAL[64][256];

    static uchar diagonalIdx(const int position, u64 allpieces,u64 key) {
        //const u64 File = key;//FILE_[position];
        u64 diagonalMaskEx_sq = _board::LEFT_DIAG[position] | POW2[position];//TODO
        allpieces = ((diagonalMaskEx_sq & allpieces) * key)>>56 ;
        return allpieces;
    }


    uchar antiDiagonalIdx(const int position, u64 allpieces) {
        const u64 File = 0x8080808080808080ull;//FILE_[position];
        u64 diagonalMaskEx_sq = _board::RIGHT_DIAG[position] | POW2[position];
        allpieces = ((diagonalMaskEx_sq & allpieces) * File) >> 56;
        return allpieces;
    }

private:
    vector<u64> res[64];
    void genPerm();
    bool popolateDiagonal(u64 key);

    void popolateAntiDiagonal();

    vector<u64> combinations_recursive(const vector<u64> &elems, unsigned long req_len,
                                       vector<unsigned long> &pos, unsigned long depth,
                                       unsigned long margin);

    vector<u64> combinations(const vector<u64> &elems, unsigned long comb_len);

    u64 performDiagShift(const int position, const u64 allpieces);

    u64 performAntiDiagShift(const int position, const u64 allpieces);

public:
    vector<u64> getPermutation(vector<u64> elements);
};

