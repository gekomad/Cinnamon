#ifdef PERFT_MODE
#ifndef TEST_MODE
#ifndef PERFT_H_
#define PERFT_H_
#include "maindefine.h"
#include "Search.h"
#include "Thread.h"
#include <vector>
#pragma pack(push)
#pragma pack(1)
typedef struct {
  u64 key;
  u64 *nMovesXply;
} ThashPerft;
#pragma pack(pop)

class RootPerft;
class Perft:public Thread, public GenMoves {
public:
  Perft ( int, char *fen, int from, int to, RootPerft * rootPerft );
   virtual ~ Perft (  );
  u64 search ( const int SIDE, int depth );
  virtual void run (  );
private:
  int from, to;
  int cpuID;
  RootPerft *rootPerft;

};
class RootPerft:public GenMoves {
public:
  int HASH_SIZE, mainDepth, nCpu;
  ThashPerft *hash;
  int nCollisions;
  void setResult ( u64 result ) {
    TOT += result;
  } RootPerft ( char *fen, int depth, int nCpu, int HASH_SIZE );
  ~RootPerft (  );
private:
  vector < Perft * >perftList;
  u64 TOT;
};
#endif
#endif
#endif
