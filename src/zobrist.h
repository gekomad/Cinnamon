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

#if !defined(ZOBRIST_H)
#define ZOBRIST_H
#ifdef HASH_MODE
#ifndef PERFT_MODE
#include "extern.h"
#include "openbook.h"

const u64 zobrist_key[13][64] = {
#include "random.txt"
};

FORCEINLINE u64
makeZobristKey (  ) {
  int i, position;
  u64 result = 0, x2;
  x2 = ~case_all_bit_occupate (  );
  while ( x2 ) {
    position = BitScanForward ( x2 );
    result ^= zobrist_key[12][position];
    x2 &= NOTTABLOG[position];
  }
  for ( i = 0; i < 12; i++ ) {
    x2 = chessboard[i];
    while ( x2 ) {
      position = BitScanForward ( x2 );
      result ^= zobrist_key[i][position];
      x2 &= NOTTABLOG[position];
    }
  }
  return result;
};

/*
__inline void RecordHash_always (const char depth,  const char flags, const int side,const u64 key,const int score){
	Thash *phashe = &hash_array_always[side][key % HASH_SIZE];
	//if (depth <= phashe->depth )return;

	#ifdef DEBUG_MODE
	assert (key != 0);
	#endif
	if (phashe->key !=0 && phashe->key != key){
			#ifdef DEBUG_MODE
			++collisions;
			#endif
			return;
	}
	#ifdef DEBUG_MODE
	++n_record_hash;
	#endif
	phashe->key = key;
	phashe->score = score;//TODO ordinamento hash

	phashe->flags = flags;
	phashe->depth =  depth;
};
*/
__inline void
RecordHash ( const char depth, const char flags, const int SIDE, const u64 key, int score ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  if ( !SIDE )
    score = -score;
//      RecordHash_always(depth,  flags, side,key, score);
  Thash *phashe = &hash_array[SIDE][key % HASH_SIZE];
  if ( depth < phashe->depth )
    return;

#ifdef DEBUG_MODE
  assert ( key != 0 );
#endif
  if ( phashe->key != 0 && phashe->key != key ) {
#ifdef DEBUG_MODE
    ++collisions;
#endif
    return;
  }
#ifdef DEBUG_MODE
  ++n_record_hash;
#endif
  phashe->key = key;
  phashe->score = score;	//TODO ordinamento hash

  phashe->flags = flags;
  phashe->depth = depth;
};


/*
FORCEINLINE u64 makeZobristKey_pawn(){

  int position;
  u64 result = 0, x2;
  for (int i = 0;i < 4;i++){  //PAWNS e torri
     x2=chessboard[i];
     while(x2){
			position = BitScanForward(x2);
			result ^= zobrist_key[position][i];
			x2 &= NOTTABLOG[position];
     }
   }
  return result;
}

FORCEINLINE void RecordHash_pawn (const u64 key,const int score){
	Thash *phashe = &hash_array_pawn[key % HASH_SIZE_PAWN];

	#ifdef DEBUG_MODE
	assert (key != 0);
	#endif
	if (phashe->key !=0 && phashe->key != key){
			#ifdef DEBUG_MODE
			++collisions_pawn;
			#endif
			return;
	}
	#ifdef DEBUG_MODE
	++n_record_hash_pawn;
	#endif
	phashe->key = key;
	phashe->score = score;



};*/
#endif

#endif
#endif
