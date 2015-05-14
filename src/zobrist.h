#if !defined(ZOBRIST_H)
#define ZOBRIST_H
#include "debug.h"
#ifndef PERFT_MODE
#include "extern.h"
#include "openbook.h"

const u64 zobrist_key[14][64] = {
#include "random.txt"
};

FORCEINLINE u64
makeZobristKey (  ) {
  int i, position;
  u64 result = 0, x2;
  for ( i = 0; i < 12; i++ ) {
    x2 = Chessboard ( i );
    while ( x2 ) {
      position = BITScanForward ( x2 );
      result ^= zobrist_key[i][position];
      x2 &= NOTTABLOG[position];
    }
  }
  return result;
}

FORCEINLINE u64
makeZobristKey ( const int side ) {
  int position;
  u64 result = makeZobristKey (  );
  u64 x2 = ~square_all_bit_occupied (  );
  while ( x2 ) {
    position = BITScanForward ( x2 );
    result ^= zobrist_key[12 + side][position];
    x2 &= NOTTABLOG[position];
  }
  return result;
}

#ifdef HASH_MODE
__inline void
RecordHash ( const char depth, const char flags, const int SIDE, const u64 key, int score ) {
  check_side ( SIDE );
  Thash *phashe = &hash_array[SIDE][key % HASH_SIZE];
  if ( depth < phashe->depth )
    return;
#ifdef DEBUG_MODE
  if ( !key )
    myassert ( 0, "" );
#endif
#ifdef DEBUG_MODE
  if ( phashe->key != 0 && phashe->key != key )
    ++collisions;
  ++n_record_hash;
#endif
  phashe->key = key;
  phashe->score = score;	//TODO ordinamento hash
  phashe->flags = flags;
  phashe->depth = depth;
}

/*
 FORCEINLINE u64 makeZobristKey_pawn(){

 int position;
 u64 result = 0, x2;
 for (int i = 0;i < 4;i++){  //PAWNS e torri
 x2=chessboard[i];
 while(x2){
 position = BITScanForward(x2);
 result ^= zobrist_key[position][i];
 x2 &= NOTTABLOG[position];
 }
 }
 return result;
 }

 FORCEINLINE void RecordHash_pawn (const u64 key,const int score){
 Thash *phashe = &hash_array_pawn[key % HASH_SIZE_PAWN];

 #ifdef DEBUG_MODE
 ASSERT(key != 0);
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
