#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>
#include "../namespaces/def.h"
#include "Bits.h"

using namespace _def;
using std::copy;
using std::cout;
using std::endl;
using std::vector;

class BitMapGenerator {
public:
    BitMapGenerator();

private:
    void popolateDiagonal();
    void popolateAntiDiagonal();

    vector<u64> combinations_recursive(const vector<u64> &elems, unsigned long req_len,
                                       vector<unsigned long> &pos, unsigned long depth,
                                       unsigned long margin);

    uchar diagonalIdx(const int position, u64 allpieces);

    uchar antiDiagonalIdx(const int position, u64 allpieces);

    vector<u64> combinations(const vector<u64> &elems, unsigned long comb_len);

    u64 performDiagShift(const int position, const u64 allpieces);

    u64 performAntiDiagShift(const int position, const u64 allpieces);

public:
    vector<vector<u64>> getPermutation(vector<u64> elements);
};

