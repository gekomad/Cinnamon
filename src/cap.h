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

#if !defined(CAP_H)
#define CAP_H
#include "bitmap.h"
#include "utility.h"
#include "extern.h"

int generateCap ( const int, const int );
int performKnight_Shift_Capture ( const int, const int, const u64, const int SIDE );
int performKing_Shift_Capture ( const int, const int, const u64, const int SIDE );
#endif
