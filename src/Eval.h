#ifndef EVAL_H_
#define EVAL_H_

#include "maindefine.h"
#include "Bits.h"
#include "Search.h"
#include "GenMoves.h"
class Search;

class Eval {
public:
  Eval ( char *iniFile, Search * s );
   virtual ~ Eval (  );
  void readParam ( char *param_file );
  void setRandomParam (  );
  void writeParam ( char *param_file, int cd_param, bool append );

  int score ( const int side
#ifdef FP_MODE
	      , const int alpha, const int beta
#endif
     );
#ifdef TEST_MODE
  int score ( const int side
#ifdef FP_MODE
	      , const int alpha, const int beta
#endif
	      , int *material_score, int *pawns_score, int *passed_pawns_score, int *knights_score, int *bishop_score, int *rooks_score, int *queens_score, int *kings_score, int *development_score, int *pawn_race_score, int *total_score );
#endif
  int lazyEval ( int SIDE ) {
    return SIDE ? lazyEvalWhite (  ) - lazyEvalBlack (  ) : lazyEvalBlack (  ) - lazyEvalWhite (  );
} private:
   STRUCTURE_TAG * structure;
  Search *search;
#ifdef DEBUG_MODE
  int N_EVALUATION[2];
#endif
  int evaluateMobility ( const int side );
  void openColumn ( const int side );
  int evaluatePawn ( const int side );
  int evaluateBishop ( const int side );
  int evaluateQueen ( const int side );
  int evaluateKnight ( const int side );
  int evaluateRook ( const int side );
  int evaluateKing ( const int side );
  int getValue ( char *s );
  int isPinned ( const int side, const char Position, const char piece );
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
  int BLOCK_PAWNS;
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
  int BONUS_CASTLE;
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
