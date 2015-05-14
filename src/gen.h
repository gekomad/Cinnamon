#if !defined(GEN_H)
#define GEN_H
#include "bitmap.h"
#include "utility.h"
#include "extern.h"
void generateMoves ( const int tipomove, const int alphabeta_side );
void un_perform_castle ( const int, const int );
void perform_castle ( const int, const int );
#include "eval.h"
void checkJumpPawn ( const int tipomove, const u64 sc, const int SIDE );
#endif
