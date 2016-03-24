#include <algorithm>
#include <vector>
#include "../namespaces/def.h"
#include "Bits.h"

using namespace _def;
using std::vector;

//Kindergarten
class BitmapGenerator {
public:
    BitmapGenerator();

    static u64 getRankFileShift(const int position, const u64 allpieces) {
        return (BITMAP_SHIFT_FILE[position][fileIdx(position, allpieces)]) |
               BITMAP_SHIFT_RANK[position][rankIdx(position, allpieces)];
    }

    static u64 getDiagAntiDiagShift(const int position, const u64 allpieces) {
        return BITMAP_SHIFT_DIAGONAL[position][diagonalIdx(position, allpieces)] |
               BITMAP_SHIFT_ANTIDIAGONAL[position][antiDiagonalIdx(position, allpieces)];
    }

private:

    const static u64 MAGIC_KEY_DIAG_ANTIDIAG = 0x101010101010101ULL;
    const static u64 MAGIC_KEY_FILE_RANK = 0x102040810204080ULL;

    static u64 BITMAP_SHIFT_DIAGONAL[64][256];
    static u64 BITMAP_SHIFT_ANTIDIAGONAL[64][256];
    static u64 BITMAP_SHIFT_FILE[64][256];
    static u64 BITMAP_SHIFT_RANK[64][256];

    vector<u64> combinationsDiagonal[64];
    vector<u64> combinationsAntiDiagonal[64];
    vector<u64> combinationsColumn[64];
    vector<u64> combinationsRank[64];

    u64 MASK_BIT_SET_NOBOUND[64][64];
    char MASK_BIT_SET_NOBOUND_COUNT[64][64];

    vector<u64> getCombination(vector<u64> elements);

    static uchar rankIdx(const int position, const u64 allpieces) {
        return allpieces >> RANK_ATx8[position];
    }

    static uchar fileIdx(const int position, const u64 allpieces) {
        return ((allpieces & FILE_[position]) * MAGIC_KEY_FILE_RANK) >> 56;
    }

    static uchar diagonalIdx(const int position, const u64 allpieces) {
        return ((allpieces & _board::LEFT_DIAG[position]) * MAGIC_KEY_DIAG_ANTIDIAG) >> 56;
    };

    static uchar antiDiagonalIdx(const int position, const u64 allpieces) {
        return ((allpieces & _board::RIGHT_DIAG[position]) * MAGIC_KEY_DIAG_ANTIDIAG) >> 56;
    }

    void popolateColumn();

    void genCombination();

    void popolateDiagonal();

    void popolateAntiDiagonal();

    vector<u64> combinations(const vector<u64> &elems, int len, vector<int> &pos, int depth, int margin);

    vector<u64> combinations(const vector<u64> &elems, int len);

    u64 performDiagShift(const int position, const u64 allpieces);

    u64 performDiagCapture(const int position, const u64 allpieces);

    u64 performAntiDiagCapture(const int position, const u64 allpieces);

    u64 performAntiDiagShift(const int position, const u64 allpieces);

    vector<u64> getCombination(u64 elements);

    void popolateRank();

    u64 performRankShift(const int position, const u64 allpieces);

    u64 performColumnCapture(const int position, const u64 allpieces);

    u64 performRankCapture(const int position, const u64 allpieces);

    u64 performColumnShift(const int position, const u64 allpieces);

};

