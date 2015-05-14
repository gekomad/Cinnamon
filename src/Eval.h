#ifndef EVAL_H_
#define EVAL_H_

#include "maindefine.h"
#include "utils.h"
#include "GenMoves.h"

enum {
  OPEN, MIDDLE, FINAL
};


class Eval:public GenMoves {
public:
  Eval (  );
  virtual ~ Eval (  );
  void setRandomParam (  );
  void writeParam ( string param_file, int cd_param, bool append );
#ifdef TUNE_CRAFTY_MODE
  int getScore ( const int side, int *material_score, int *pawns_score, int *passed_pawns_score, int *knights_score, int *bishop_score, int *rooks_score, int *queens_score, int *kings_score, int *development_score, int *pawn_race_score, int *total_score );
#else
  int getScore ( const int side, const int alpha, const int beta );
#endif
  int lazyEval ( int side ) {
    return side ? lazyEvalWhite (  ) - lazyEvalBlack (  ) : lazyEvalBlack (  ) - lazyEvalWhite (  );
  }
#ifdef DEBUG_MODE
  int LazyEvalCuts;
  unsigned totmosse, totGen;
#endif
private:

#ifdef DEBUG_MODE
  int N_EVALUATION[2];
#endif

  void openColumn ( const int side );
  int evaluatePawn ( const int side );
  int evaluateBishop ( const int side );
  int evaluateQueen ( const int side );
  int evaluateKnight ( const int side );
  int evaluateRook ( const int side );
  int evaluateKing ( const int side );
  int getValue ( string s );
  int isPinned ( const int side, const uchar Position, const uchar piece );
  int lazyEvalBlack (  );
  int lazyEvalWhite (  );
  int PINNED_PIECE;
  int ATTACK_CENTRE;
  int ATTACK_KING;
  int OPEN_FILE_Q;
  int FORK_SCORE;
  int BONUS2BISHOP;
  int MOB;
  int KING_TRAPPED;
  int KNIGHT_TRAPPED;
  int BISHOP_TRAPPED;
  int ROOK_TRAPPED;
  int QUEEN_TRAPPED;
  int CONNECTED_ROOKS;
  int ROOK_BLOCKED;
  int ROOK_7TH_RANK;
  int OPEN_FILE;
  int UNDEVELOPED;
  int HALF_OPEN_FILE_Q;
  int DOUBLED_PAWNS;
  int PAWN_IN_RACE;
  int PAWN_7H;
  int PAWN_CENTRE;
  int FRIEND_NEAR_KING;
  int ENEMY_NEAR_KING;
  int PAWN_ISOLATED;
  int SPACE;
  int END_OPENING;
  int PAWN_NEAR_KING;
  int NEAR_xKING;
  int BONUS_11;
  int BISHOP_ON_QUEEN;
  int ENEMIES_PAWNS_ALL;
  int DOUBLED_ISOLATED_PAWNS;
  int BACKWARD_PAWN;
  int BACKWARD_OPEN_PAWN;
  int UNPROTECTED_PAWNS;
  int BISHOP_TRAPPED_DIAG;
  int ATTACK_F7_F2;
};
#endif
