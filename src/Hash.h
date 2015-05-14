#ifndef HASH_H_
#define HASH_H_
#include "maindefine.h"
#include "bitmap.h"
#include "utils.h"

#define hashfEXACT  0
#define  hashfALPHA 1
#define hashfBETA  2

#pragma pack(push)
#pragma pack(1)
typedef struct {
  u64 key;
  short score;
  uchar flags:2;
  uchar depth:6;
  uchar from:6;
  uchar to:6;
  uchar entryAge:1;
} _Thash;

#pragma pack(pop)

class Hash {
public:

  void clearHash (  );
#ifdef DEBUG_MODE
  unsigned n_record_hash, collisions;
  unsigned n_cut_hash;
  int n_cut_hashA, n_cut_hashE, n_cut_hashB, cutFailed, probeHash;
#endif
  void recordHash ( bool running, _Thash * phashe_greater, _Thash * phashe_always, const char depth, const char flags, const u64 key, const int score, _Tmove * bestMove );
  void clearAge (  );
  void setHashSize ( int mb );

   Hash (  );
   virtual ~ Hash (  );
protected:
  int HASH_SIZE;
  _Thash *hash_array_greater[2];
  _Thash *hash_array_always[2];
private:

  void dispose (  );
};

#endif
