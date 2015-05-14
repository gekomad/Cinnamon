/*
Copyright (C) 2008-2010
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

int change_side ( int side );
uchar get_piece_at ( int side, u64 tablogpos );
u64 Chessboard ( int x );
u64 tablog ( int x );
#endif
#endif
