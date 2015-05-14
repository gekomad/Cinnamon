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


#if !defined(EVAL_H)
#define EVAL_H
#ifndef PERFT_MODE
#include "maindefine.h"

int evaluateMobility ( const int tipo );
void open_column ( const int, const u64, u64, const u64 );
int evaluate_pawn ( const int, u64, const u64, const u64, const int );
int evaluate_bishop ( const int, const u64, const u64, const int );
int evaluate_queen ( const int, u64, const int, const u64, const u64, const u64, const int );
int evaluate_knight ( const int, const int, const int, const u64, const u64 );
int evaluate_king ( const int, const u64, const u64, const int, const u64, const u64 );
int evaluate_rook ( const int, const u64, const u64, const int, const u64, const int );
int passed_and_space ( const int, u64, const u64 );
int lazy_eval (  );
int piece_attack ( const int, u64, const int );
int eval ( const int
#ifdef FP_MODE
	   , const int, const int
#endif
#ifdef HASH_MODE
	   , const u64
#endif
   );

#endif
#endif
