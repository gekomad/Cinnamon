#if !defined(CAP_H)
#define CAP_H
#include "bitmap.h"
#include "utility.h"
#include "extern.h"

int generateCap ( const int, const int );
int performKnight_Shift_Capture ( const int, const int, const u64, const int SIDE );
int performKing_Shift_Capture ( const int, const int, const u64, const int SIDE );
#endif
