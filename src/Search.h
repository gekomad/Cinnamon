#ifndef SEARCH_H_
#define SEARCH_H_
#include <sys/timeb.h>
#include "Eval.h"
#include "namespaces.h"
#include <climits>

class Search:public Eval, public Hash {

public:

  Search (  );
  virtual ~ Search (  );
  void setRunning ( int );
  void setPonder ( bool );
  void pushMovesPath ( char );
  void clearMovesPath (  );
  void setNullMove ( bool );
  void setMaxTimeMillsec ( int );
  int getMaxTimeMillsec (  );
  void startClock (  );
  int getRunning (  );

protected:

  typedef struct {
    int cmove;
    _Tmove argmove[GenMoves::MAX_PLY];
  } _TpvLine;

  string getMovesPath (  );
  void setMainPly ( int );
  int search ( int depth, int alpha, int beta, _TpvLine * pline );
#ifdef DEBUG_MODE
  unsigned cumulativeMovesCount, totGen;
#endif
private:

  void setMaxDepthSearch ( int );
  int getMaxDepthSearch (  );
  bool checkDraw ( u64 key );
  bool ponder;
  int checkTime (  );
  string movesPath;
  int running, mainDepth, maxTimeMillsec;
  bool nullSearch;
  struct timeb startTime;
   template < int side > int search ( int depth, int alpha, int beta, _TpvLine * pline, int );
  bool checkInsufficientMaterial ( int );
  void sortHashMoves ( int listId, _Thash * );
   template < int side > int quiescence ( int alpha, int beta, const char promotionPiece, int, int depth );
  void updatePv ( _TpvLine * pline, const _TpvLine * line, const _Tmove * move );
//    template <int side> void updatePv(_TpvLine * pline, const _TpvLine * line, const int from, const int to);
};
#endif
