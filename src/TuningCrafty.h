#ifdef TUNE_CRAFTY_MODE
#ifndef TEST_H_
#define TEST_H_

#include "IterativeDeeping.h"
#include "Search.h"
#include "ChessBoard.h"

class IterativeDeeping;
class Search;
typedef struct {
  Tchessboard chessboard;
  u64 RIGHT_CASTLE;
  int ENP_POSSIBILE;
  int friendKing[2];
  char side;
} CACHE_INIT_TAG;
typedef struct {
  char fen[200];
  float crafty_material;
  float crafty_pawns;
  float crafty_passed_pawns;
  float crafty_knights;
  float crafty_bishop;
  float crafty_rooks;
  float crafty_queens;
  float crafty_kings;
  float crafty_development;
  float crafty_pawn_race;
  float crafty_total;
} CRAFTY_TAG;
class TuningCrafty {
public:
  TuningCrafty ( string );
  virtual ~ TuningCrafty (  );
  void evalCraftyTuning ( string tetfile );

private:
   string craftyExe;
  Search *search;
  Eval *eval;
  IterativeDeeping *it;
  int wc ( string fileName );
  int fileSize ( const string FileName );
  CRAFTY_TAG *loadBin ( string, int * );
  CRAFTY_TAG *createBin ( string, int * );
  void extract_crafty_scores ( const string PATH_LOG, float *crafty_material, float *crafty_pawns, float *crafty_passed_pawns, float *crafty_knights, float *crafty_bishop, float *crafty_rooks, float *crafty_queens, float *crafty_kings, float *crafty_development, float *crafty_pawn_race, float *crafty_total );
};
#endif
#endif
