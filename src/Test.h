#ifdef TEST_MODE
#ifndef TEST_H_
#define TEST_H_

#include "IterativeDeeping.h"
#include "Search.h"
#include "ChessBoard.h"

class IterativeDeeping;
class Search;
class Test {
public:
  Test (  );
  virtual ~ Test (  );
  void test_eval_crafty_tuning ( const char *testfile, bool create_crafy_score_bin );
  void test_eval_crafty ( const char *testfile );
  void test_epd ( char *testfile, int );
  void extract_end_book ( const char *testfile );
  void extract_crafty_scores ( const char *PATH_LOG, float *crafty_material, float *crafty_pawns, float *crafty_passed_pawns, float *crafty_knights, float *crafty_bishop, float *crafty_rooks, float *crafty_queens, float *crafty_kings, float *crafty_development, float *crafty_pawn_race, float *crafty_total );
private:
   Search * search;
  Eval *eval;
  IterativeDeeping *it;
  int wc ( const char *fileName );
  int fileSize ( const char *FileName );
};
#endif
#endif
