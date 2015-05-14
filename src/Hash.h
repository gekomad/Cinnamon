#ifndef HASH_H_
#define HASH_H_
#include <iostream>
#include <string.h>
#include "namespaces.h"
using namespace _board;

class Hash {
public:
  enum { hashfEXACT = 0, hashfALPHA = 1, hashfBETA = 2 };
   Hash (  );
   virtual ~ Hash (  );
  void setHashSize ( int mb );
  int getHashSize (  );
  void clearHash (  );

protected:

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

  int HASH_SIZE;
  _Thash *hash_array_greater[2];
  _Thash *hash_array_always[2];
  void recordHash ( bool running, _Thash * phashe_greater, _Thash * phashe_always, const char depth, const char flags, const u64 key, const int score, _Tmove * bestMove );
  void clearAge (  );
#ifdef DEBUG_MODE
  unsigned nRecordHashA, nRecordHashB, nRecordHashE, collisions;
  unsigned n_cut_hash;
  int n_cut_hashA, n_cut_hashE, n_cut_hashB, cutFailed, probeHash;
#endif

private:
  void dispose (  );
};

#endif
