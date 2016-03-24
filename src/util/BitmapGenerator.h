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

    const static u64 MAGIC_KEY = 0x101010101010101ULL;
    const static u64 MAGIC_KEY_FILE = 0x102040810204080ULL;

    static u64 BITMAP_SHIFT_DIAGONAL[64][256];
    static u64 BITMAP_SHIFT_ANTIDIAGONAL[64][256];
    static u64 BITMAP_SHIFT_COLUMN[64][256];
    static u64 BITMAP_SHIFT_RANK[64][256];

#define diagonalIdx(position, allpieces) ( (uchar)((( (allpieces) & _board::LEFT_DIAG[position] ) * BitmapGenerator::MAGIC_KEY) >> 56 ))

#define antiDiagonalIdx(position, allpieces) ((uchar) ((( (allpieces) & _board::RIGHT_DIAG[position] ) * BitmapGenerator::MAGIC_KEY) >> 56))

#define columnIdx(position, allpieces) ((uchar)((((allpieces) & FILE_[position]) * BitmapGenerator::MAGIC_KEY_FILE) >>56))

#define rankIdx(position, allpieces) ((uchar)((allpieces) >> RANK_ATx8[position]))

private:
    vector<u64> combinationsDiagonal[64];
    vector<u64> combinationsAntiDiagonal[64];
    vector<u64> combinationsColumn[64];
    vector<u64> combinationsRank[64];

    u64 MASK_BIT_SET_NOBOUND[64][64];
    char MASK_BIT_SET_NOBOUND_COUNT[64][64];

    vector<u64> getPermutation(vector<u64> elements);

    void popolateColumn();

    void genPermutation();

    void popolateDiagonal();

    void popolateAntiDiagonal();

    vector<u64> combinations(const vector<u64> &elems, int len, vector<int> &pos, int depth, int margin);

    vector<u64> combinations(const vector<u64> &elems, int len);

    u64 performDiagShift(const int position, const u64 allpieces);

    u64 performDiagCapture(const int position, const u64 allpieces);

    u64 performAntiDiagCapture(const int position, const u64 allpieces);

    u64 performAntiDiagShift(const int position, const u64 allpieces);

    vector<u64> getPermutation(u64 elements);

    void popolateRank();

    u64 performRankShift(const int position, const u64 allpieces);

    u64 performColumnCapture(const int position, const u64 allpieces);

    u64 performRankCapture(const int position, const u64 allpieces);

    u64 performColumnShift(const int position, const u64 allpieces);

};

