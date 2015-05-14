#include "Hash.h"


Hash::Hash (  ) {
  hash_array_greater[BLACK] = hash_array_greater[WHITE] = NULL;
  hash_array_always[BLACK] = hash_array_always[WHITE] = NULL;
  HASH_SIZE = 0;

#ifdef DEBUG_MODE
  n_cut_hashA = n_cut_hashE = n_cut_hashB = cutFailed = probeHash = 0;
  n_cut_hash = n_record_hash = collisions = 0;
#endif
#ifndef NO_HASH_MODE
  setHashSize ( 64 );
#endif
}

void
Hash::clearAge (  ) {
  for ( int i = 0; i < HASH_SIZE; i++ ) {
    hash_array_greater[BLACK][i].entryAge = 0;
    hash_array_greater[WHITE][i].entryAge = 0;
  }
}

void
Hash::clearHash (  ) {
  if ( !HASH_SIZE )
    return;
  memset ( hash_array_greater[BLACK], 0, sizeof ( _Thash ) * HASH_SIZE );
  memset ( hash_array_greater[WHITE], 0, sizeof ( _Thash ) * HASH_SIZE );
}

void
Hash::setHashSize ( int mb ) {
  dispose (  );
  if ( mb ) {
    HASH_SIZE = mb * 1024 * 1000 / ( sizeof ( _Thash ) * 4 );
    hash_array_greater[BLACK] = ( _Thash * ) calloc ( HASH_SIZE, sizeof ( _Thash ) );
    hash_array_greater[WHITE] = ( _Thash * ) calloc ( HASH_SIZE, sizeof ( _Thash ) );
    myassert ( hash_array_greater[BLACK] );
    myassert ( hash_array_greater[WHITE] );
    hash_array_always[BLACK] = ( _Thash * ) calloc ( HASH_SIZE, sizeof ( _Thash ) );
    hash_array_always[WHITE] = ( _Thash * ) calloc ( HASH_SIZE, sizeof ( _Thash ) );
    myassert ( hash_array_always[BLACK] );
    myassert ( hash_array_always[WHITE] );
  }
}

void
Hash::recordHash ( bool running, _Thash * phashe_greater, _Thash * phashe_always, const char depth, const char flags, const u64 key, const int score, _Tmove * bestMove ) {
#ifndef NO_HASH_MODE

  ASSERT ( key );
  if ( !running )
    return;

  ASSERT ( abs ( score ) <= 32200 );
  _Thash *phashe = phashe_greater;
  phashe->key = key;
  phashe->score = score;
  phashe->flags = flags;
  phashe->depth = depth;

  if ( bestMove && bestMove->from != bestMove->to ) {
    phashe->from = bestMove->from;
    phashe->to = bestMove->to;
  }
  else {
    phashe->from = phashe->to = 0;
  }
  /////
  phashe = phashe_always;
  if ( phashe->key && phashe->depth >= depth && phashe->entryAge ) {
    INC ( collisions );
    return;
  }
  INC ( n_record_hash );
  phashe->key = key;
  phashe->score = score;
  phashe->flags = flags;
  phashe->depth = depth;

  phashe->entryAge = 1;
  if ( bestMove && bestMove->from != bestMove->to ) {
    phashe->from = bestMove->from;
    phashe->to = bestMove->to;
  }
  else {
    phashe->from = phashe->to = 0;
  }
#endif
}

void
Hash::dispose (  ) {
  if ( hash_array_greater[BLACK] )
    free ( hash_array_greater[BLACK] );
  if ( hash_array_greater[WHITE] )
    free ( hash_array_greater[WHITE] );
  if ( hash_array_always[BLACK] )
    free ( hash_array_always[BLACK] );
  if ( hash_array_always[WHITE] )
    free ( hash_array_always[WHITE] );
  hash_array_greater[BLACK] = hash_array_greater[WHITE] = NULL;
  hash_array_always[BLACK] = hash_array_always[WHITE] = NULL;
  HASH_SIZE = 0;
}

Hash::~Hash (  ) {
  dispose (  );
}
