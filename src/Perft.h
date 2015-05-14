#ifndef PERFT_H_
#define PERFT_H_

#include "Search.h"
#include "Thread.h"
#include <iomanip>

class Perft {

public:
  Perft ( string fen, int depth, int nCpu, int HASH_SIZE );
  ~Perft (  );

private:

#pragma pack(push)
#pragma pack(1)
  typedef struct {
    u64 key;
    u64 *nMovesXply;
  } _ThashPerft;
#pragma pack(pop)

  int nCollisions, PERFT_HASH_SIZE, mainDepth, nCpu;
  static const u64 NULL_KEY = 0xffffffffffffffffULL;
  _ThashPerft *hash;

  void setResult ( u64 result ) {
    totMoves += result;
  } volatile u64 totMoves;

  class PerftThread:public Thread, public GenMoves {
  public:
    PerftThread ( int, string fen, int from, int to, Perft * Perft );
     PerftThread (  );
     virtual ~ PerftThread (  );
     template < int side > u64 search ( int depth, u64 key );
    virtual void run (  );
  private:
    int from, to, cpuID;
    Perft *perft;
  };
  vector < PerftThread * >threadList;
};
#endif
