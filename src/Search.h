#ifndef BOARD_H_
#define BOARD_H_
#include "maindefine.h"
#include "bitmap.h"
#include "Bits.h"
#include "Eval.h"
#include "GenMoves.h"
class Eval;

#ifdef FP_MODE
#define FUTIL_MARGIN 198
#define EXT_FUTILY_MARGIN 515
#define RAZOR_MARGIN 1071
#endif

#define in_check(side)((side)==BLACK ?attackSquare(BLACK,BITScanForward (chessboard[KING_BLACK ])) :(attackSquare(WHITE,BITScanForward (chessboard[KING_WHITE])) ))
#define R_adpt(tipo,depth) (2+(depth > (3+((n_pieces(tipo)<3)?2:0))))
#define null_ok(depth,side,n_pieces_side)(null_sem ? 0:(depth < 3 ?0:(n_pieces_side < 4 ? 0:1)))

typedef struct tagLINE {
  int cmove;
  Tmove argmove[MAX_PLY];
} LINE;

class Search:public GenMoves {
public:
  Search ( char *fen );
   virtual ~ Search (  );
  void setRunning ( int r );
  int getRunning (  );
  void setMaxTimeMillsec ( int n );
  void setMaxDepthSearch ( int n );
  int getMaxTimeMillsec (  );
  int getMaxDepthSearch (  );
  void setMainPly ( int m );
  void startClock (  );
  void setRandomParam (  );
  void writeParam ( char *param_file, int cd_param, bool append );
#ifdef TEST_MODE
  int getScore ( const int side
#ifdef FP_MODE
		 , const int alpha, const int beta
#endif
		 , int *a1, int *a2, int *a3, int *a4, int *a5, int *a6, int *a7, int *a8, int *a9, int *a10, int *a11 );
#endif
  int getScore ( int
#ifdef FP_MODE
		 , int, int
#endif
     );
  int search ( const int side, int depth, int alpha, int beta, LINE * pline
#ifdef DEBUG_MODE
	       , Tmove * Pmossa
#endif
     );
#ifdef DEBUG_MODE
  int LazyEvalCuts;
#endif
private:
  int running, mainDepth, maxTimeMillsec;
  Eval *eval;
  bool null_sem;
  struct timeb start_time;
  int checkTime (  );
  int quiescence ( int alpha, const int side, int beta, const char promotion_piece, int dep );
  void updatePv ( LINE * pline, const LINE * line, const Tmove * mossa );
};
#endif
