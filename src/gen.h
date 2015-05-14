/*
Copyright (C) 2008
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !defined(GEN_H)
#define GEN_H
#include "bitmap.h"
#include "utility.h"
//#include "swapoff.h"
#include "extern.h"

void generateMoves ( const int tipomove, const int alphabeta_side );
void un_make_castle ( const int, const int );
void make_castle ( const int, const int );
#include "eval.h"
void checkJumpPawn ( const int tipomove, const u64 sc, const int SIDE );

#endif
