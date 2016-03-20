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
    vector<u64> combinations_recursive(const vector<u64> &elems, unsigned long req_len,
                                       vector<unsigned long> &pos, unsigned long depth,
                                       unsigned long margin) ;

    vector<u64> combinations(const vector<u64> &elems, unsigned long comb_len);
public:
    vector<vector<u64>> go(vector<u64> elements) ;
};

