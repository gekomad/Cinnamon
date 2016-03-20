#include "BitmapGenerator.h"

BitMapGenerator::BitMapGenerator() {
    Bits::getInstance();
    vector<u64> elements;
    for (int i = 0; i < 8; i++)
        elements.push_back(pow(2, i));

    vector<vector<u64>> res = go(elements);
    for (int i = 0; i < res.size(); i++) {
        for (int y:res[i]) cout << "(" << y << ")";
        cout << "\n";
    }

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


vector<vector<u64>> BitMapGenerator::go(vector<u64> elements) {
    vector<vector<u64>> res(256);
    vector<u64> v;
    int nVector = 0;
    for (int comb_len = 1; comb_len < elements.size() + 1; comb_len++) {
        v = combinations(elements, comb_len);
        int k = 0;

        for (int rr:v) {
            //  cout << "(" << rr << ")";
            cout << nVector << endl;
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
