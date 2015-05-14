#if !defined(_Debug_h)
#define _Debug_h
#ifdef DEBUG_MODE
#include <iostream>
#include <bitset>
#include <limits>
#include <stdio.h>
#include <stdlib.h>
#include "maindefine.h"
#include <string.h>
#include "bitmap.h"
#include "extern.h"
Tmove *get_gen_list ( int list_id, int i );
int get_gen_list_count ( int list_id );
int change_side ( int side );
uchar get_piece_at ( int side, u64 tablogpos );
u64 Chessboard ( int x );
u64 tablog ( int x );
#endif
#endif
