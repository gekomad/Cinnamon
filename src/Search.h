#ifndef BOARD_H_
#define BOARD_H_
#include "maindefine.h"
#include "bitmap.h"
#include "utils.h"
#include "Eval.h"

#define in_check(side)((side)==BLACK ?attackSquare(BLACK,BITScanForward (chessboard[KING_BLACK ])) :(attackSquare(WHITE,BITScanForward (chessboard[KING_WHITE])) ))
#define R_adpt(type,depth) (2+(depth > (3+((getNpieces(type)<3)?2:0))))
#define null_ok(depth,side,n_pieces_side)(nullSem ? 0:(depth < 3 ?0:(n_pieces_side < 4 ? 0:1)))


typedef struct {
  int cmove;
  _Tmove argmove[MAX_PLY];
} _TpvLine;

class Search:public Eval, public Hash {
public:


  int checkTime (  );
   Search (  );
  void setNullMove ( bool b );
   virtual ~ Search (  );
  void setRunning ( int r );
  int getRunning (  );
  void setMaxTimeMillsec ( int n );
  void setMaxDepthSearch ( int n );
  int getMaxTimeMillsec (  );
  int getMaxDepthSearch (  );
  void setMainPly ( int m );
  void startClock (  );
  int search ( u64 key, const int side, int depth, int alpha, int beta, _TpvLine * pline );

protected:

private:

  int running, mainDepth, maxTimeMillsec;
  bool nullSem;
  struct timeb start_time;
  bool checkDraw ( u64 key );
  //  bool checkInsufficientMaterial() ;
  void sortHashMoves ( int list_id, _Thash * );
  int quiescence ( u64 key, int alpha, const int side, int beta, const char promotionPiece, int dep );
  void updatePv ( _TpvLine * pline, const _TpvLine * line, const _Tmove * move );
  void updatePv ( _TpvLine * pline, const _TpvLine * line, const int from, const int to, const int side );

};
#endif
