#ifndef TUNE_CRAFTY_MODE
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
} _ThashPerft;
#pragma pack(pop)

class Perft;
class PerftThread:public Thread, public GenMoves {
public:
  PerftThread ( int, string fen, int from, int to, Perft * Perft );
   PerftThread (  );
   virtual ~ PerftThread (  );
  u64 search ( const int side, int depth, u64 key );
  virtual void run (  );
private:
  int from, to;
  int cpuID;
  Perft *perft;

};
class Perft {
public:
  int PERFT_HASH_SIZE, mainDepth, nCpu;
  _ThashPerft *hash;
  int nCollisions;
  void setResult ( u64 result );
   Perft ( string fen, int depth, int nCpu, int HASH_SIZE );
  ~Perft (  );
private:
   vector < PerftThread * >perftList;
  u64 TOT;
};
#endif
#endif
