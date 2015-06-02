#include "Perft.h"

Perft::PerftThread::PerftThread (  ) {
}

Perft::PerftThread::PerftThread ( int cpuID1, string fen1, int from1, int to1, Perft * perft1 ):
GenMoves (  ) {
  perftMode = true;
  loadFen ( fen1 );
  this->cpuID = cpuID1;
  this->perft = perft1;
  this->from = from1;
  this->to = to1;
}

Perft::~Perft (  ) {
  for ( int i = 0; i < PERFT_HASH_SIZE; i++ )
    free ( hash[i].nMovesXply );
  free ( hash );
}

Perft::Perft ( string fen, int depth, int nCpu1, int hash_size ) {
  PerftThread *p = new PerftThread (  );
  if ( !fen.empty (  ) )
    p->loadFen ( fen );
  p->setPerft ( true );
  p->display (  );
  struct timeb start1, end1;
  totMoves = nCollisions = 0;
  int side = p->getSide (  )? 1 : 0;
  this->PERFT_HASH_SIZE = hash_size * 1024 * 1024 / ( sizeof ( u64 ) * depth + sizeof ( _ThashPerft ) );
  this->hash = nullptr;
  if ( PERFT_HASH_SIZE ) {
    hash = ( _ThashPerft * ) calloc ( PERFT_HASH_SIZE, sizeof ( _ThashPerft ) );
    assert ( hash );
    for ( int i = 0; i < PERFT_HASH_SIZE; i++ ) {
      this->hash[i].key = NULL_KEY;
      hash[i].nMovesXply = ( u64 * ) malloc ( ( depth - 1 ) * sizeof ( u64 ) );
      assert ( hash[i].nMovesXply );
      for ( int j = 0; j < depth - 1; j++ ) {
	hash[i].nMovesXply[j] = NULL_KEY;
      }
    }
  }
  ftime ( &start1 );
  this->mainDepth = depth - 1;
  this->nCpu = nCpu1;
  p->incListId (  );

  u64 friends = side ? p->getBitBoard < WHITE > (  ) : p->getBitBoard < BLACK > (  );
  u64 enemies = side ? p->getBitBoard < BLACK > (  ) : p->getBitBoard < WHITE > (  );

  p->generateCaptures ( side, enemies, friends );
  p->generateMoves ( side, friends | enemies );
  int listcount = p->getListSize (  );
  delete ( p );
  p = nullptr;
  ASSERT ( nCpu1 > 0 );
  int block = listcount / nCpu1;
  int i, s = 0;
  for ( i = 0; i < nCpu1 - 1; i++ ) {
    threadList.push_back ( new PerftThread ( i, fen, s, s + block, this ) );
    s += block;
  }
  threadList.push_back ( new PerftThread ( i, fen, s, listcount, this ) );

  for ( auto it = threadList.begin (  ); it != threadList.end (  ); ++it ) {
    ( *it )->start (  );
  }

  for ( auto it = threadList.begin (  ); it != threadList.end (  ); ++it ) {
    ( *it )->join (  );
    ( *it )->stop (  );
    delete *it;
  }

  ftime ( &end1 );
#ifdef DEBUG_MODE
  cout << endl << endl << "collisions: " << nCollisions;
#endif
  int t = _time::diffTime ( end1, start1 ) / 1000;

  int days = t / 60 / 60 / 24;
  int hours = ( t / 60 / 60 ) % 24;
  int minutes = ( t / 60 ) % 60;
  int seconds = t % 60;
  cout << endl << endl << "Perft moves: " << totMoves << " in ";
  if ( days )
    cout << days << " days, ";
  if ( days || hours )
    cout << hours << " hours, ";
  if ( days || hours || minutes )
    cout << minutes << " minutes, ";
  if ( !days )
    cout << seconds << " seconds";
  if ( t )
    cout << " (" << ( totMoves / t ) / 1000 - ( ( totMoves / t ) / 1000 ) % 1000 << "k nodes per seconds" << ")";
  cout << endl << flush;

}

template < int side > u64
Perft::PerftThread::search ( int depth ) {
  if ( depth == 0 ) {
    return 1;
  }
  _ThashPerft *
    phashe = nullptr;
  if ( depth >= 2 && perft->hash ) {
    phashe = &( perft->hash[zobristKey % perft->PERFT_HASH_SIZE] );
    if ( zobristKey == phashe->key && phashe->nMovesXply[depth - 2] != NULL_KEY ) {
      return phashe->nMovesXply[depth - 2];
    }
  }
  u64
    n_perft = 0;
  int
    listcount;
  _Tmove *
    move;
  incListId (  );
  u64
    friends = getBitBoard < side > (  );
  u64
    enemies = getBitBoard < side ^ 1 > (  );
  if ( generateCaptures < side > ( enemies, friends ) ) {
    decListId (  );
    return 0;
  }
  generateMoves < side > ( friends | enemies );
  listcount = getListSize (  );
  if ( !listcount ) {
    decListId (  );
    return 0;
  }
  for ( int ii = 0; ii < listcount; ii++ ) {

    move = getMove ( ii );
    u64
      keyold = zobristKey;

    makemove ( move, false, false );

    n_perft += search < side ^ 1 > ( depth - 1 );
    takeback ( move, keyold, false );
  }
  resetList (  );
  decListId (  );
  if ( phashe ) {
    if ( phashe->key == zobristKey ) {
      phashe->nMovesXply[depth - 2] = n_perft;
    }
    else if ( phashe->key == NULL_KEY ) {
      phashe->nMovesXply[depth - 2] = n_perft;
      phashe->key = zobristKey;
    }
    else {
      perft->nCollisions++;
    }
  }
  return n_perft;
}

void
Perft::PerftThread::run (  ) {
  init (  );
  _Tmove *
    move;
  incListId (  );
  resetList (  );

  u64
    friends = sideToMove ? getBitBoard < WHITE > (  ) : getBitBoard < BLACK > (  );
  u64
    enemies = sideToMove ? getBitBoard < BLACK > (  ) : getBitBoard < WHITE > (  );

  generateCaptures ( sideToMove, enemies, friends );
  generateMoves ( sideToMove, friends | enemies );
  u64
    tot = 0;
  makeZobristKey (  );
  u64
    keyold = zobristKey;

  for ( int ii = to - 1; ii >= from; ii-- ) {
    u64
      n_perft = 0;
    move = getMove ( ii );

    makemove ( move, false, false );
    n_perft = ( sideToMove ^ 1 ) == WHITE ? search < WHITE > ( perft->mainDepth ) : search < BLACK > ( perft->mainDepth );
    takeback ( move, keyold, false );

    char
      y;
    char
      x = FEN_PIECE[sideToMove ? getPieceAt < WHITE > ( POW2[move->from] ) : getPieceAt < BLACK > ( POW2[move->from] )];
    if ( x == 'p' || x == 'P' )
      x = ' ';
    if ( move->capturedPiece != SQUARE_FREE )
      y = '*';
    else
      y = '-';
    cout << endl << "#" << ii + 1 << " cpuID# " << cpuID;
    if ( ( decodeBoardinv ( move->type, move->to, sideToMove ) ).length (  ) > 2 )
      cout << "\t" << decodeBoardinv ( move->type, move->to, sideToMove ) << "\t" << n_perft << " ";
    else
      cout << "\t" << x << decodeBoardinv ( move->type, move->from, sideToMove ) << y << decodeBoardinv ( move->type, move->to, sideToMove ) << "\t" << n_perft << " ";
    cout << flush;
    tot += n_perft;
  }

  decListId (  );
  perft->setResult ( tot );
}

Perft::PerftThread::~PerftThread (  ) {
}
