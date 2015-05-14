#ifndef TUNE_CRAFTY_MODE
#include "Perft.h"
#include <iomanip>
PerftThread::PerftThread (  ) {
}

PerftThread::PerftThread ( int cpuID1, string fen1, int from1, int to1, Perft * perft1 ):
GenMoves (  ) {
  perftMode = true;
  if ( !fen1.empty (  ) )
    INITIAL_FEN = fen1;
  loadFen ( INITIAL_FEN );
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

void
Perft::setResult ( u64 result ) {
  TOT += result;
}
Perft::Perft ( string fen, int depth, int nCpu1, int hash_size ) {
  PerftThread *p = new PerftThread (  );
  if ( !fen.empty (  ) )
    p->loadFen ( fen );
  p->setPerft ( true );
  p->display (  );
  struct timeb start1, end1;
  TOT = nCollisions = 0;
  int side = p->getSide (  )? 1 : 0;
  this->PERFT_HASH_SIZE = hash_size * 1024 * 1024 / ( sizeof ( u64 ) * depth + sizeof ( _ThashPerft ) );
  this->hash = NULL;
  if ( PERFT_HASH_SIZE ) {
    hash = ( _ThashPerft * ) calloc ( PERFT_HASH_SIZE, sizeof ( _ThashPerft ) );
    myassert ( hash );
    for ( int i = 0; i < PERFT_HASH_SIZE; i++ ) {
      this->hash[i].key = 0xffffffffffffffffULL;
      hash[i].nMovesXply = ( u64 * ) malloc ( ( depth - 1 ) * sizeof ( u64 ) );
      myassert ( hash[i].nMovesXply );
      for ( int j = 0; j < depth - 1; j++ ) {
	hash[i].nMovesXply[j] = 0xffffffffffffffffULL;
      }
    }
  }
  ftime ( &start1 );
  this->mainDepth = depth - 1;
  this->nCpu = nCpu1;
  p->incListId (  );
  u64 friends = p->getBitBoard ( side );
  u64 enemies = p->getBitBoard ( side ^ 1 );
  u64 dummy = 0;
  p->generateCap ( side, enemies, friends, &dummy );
  p->generateMoves ( side, friends | enemies );
  int listcount = p->getListCount (  );
  delete ( p );
  p = NULL;
  myassert ( nCpu1 > 0 );
  int block = listcount / nCpu1 - 1;
  int s = 1;
  cout << "split:";
  for ( int i = 0; i < nCpu1 - 1; i++ ) {
    cout << " cpu#" << i << " from: " << s << " to: " << s + block << flush;
    perftList.push_back ( new PerftThread ( i, fen, s, s + block, this ) );
    perftList[i]->start (  );
    s += block + 1;
  }
  cout << " cpu#" << nCpu1 - 1 << " from: " << s << " to: " << listcount << flush;
  perftList.push_back ( new PerftThread ( nCpu1 - 1, fen, s, listcount, this ) );
  perftList[nCpu1 - 1]->start (  );
  for ( int i = 0; i < nCpu1; i++ ) {
    perftList[i]->join (  );
    perftList[i]->stop (  );
    delete perftList[i];
  }
  ftime ( &end1 );
  cout << "\ncollisions: " << nCollisions << endl << flush;
  cout << "TOT: [" << TOT << "] in " << setprecision ( 2 ) << diff_time ( end1, start1 ) / 1000.0 << " seconds" << endl;

}

u64
PerftThread::search ( const int side, int depth, u64 key ) {
  if ( depth == 0 ) {
    return 1;
  }
  _ThashPerft *phashe = NULL;
  if ( depth >= 2 && perft->hash ) {
    phashe = &( perft->hash[key % perft->PERFT_HASH_SIZE] );
    if ( key == phashe->key && phashe->nMovesXply[depth - 2] != 0xffffffffffffffffULL ) {
      return phashe->nMovesXply[depth - 2];
    }
  }

  u64 n_perft = 0;
  int ii, listcount;
  _Tmove *move;
  incListId (  );
  u64 friends = getBitBoard ( side );
  u64 enemies = getBitBoard ( side ^ 1 );
  if ( generateCap ( side, enemies, friends, &key ) ) {

    decListId (  );
    return 0;
  }
  generateMoves ( side, friends | enemies );
  listcount = getListCount (  );
  if ( !listcount ) {
    decListId (  );
    return 0;
  }

  for ( ii = 1; ii <= listcount; ii++ ) {

    move = getList ( ii );
    u64 keyold = key;

    makemove ( move, &key );
    ASSERT ( key == makeZobristKey (  ) );
    n_perft += search ( side ^ 1, depth - 1, key );
    takeback ( move, &key, keyold );

  }
  resetList (  );
  decListId (  );
  if ( phashe ) {
    if ( phashe->key == key ) {
      phashe->nMovesXply[depth - 2] = n_perft;
    }
    else if ( phashe->key == 0xffffffffffffffffULL ) {
      phashe->nMovesXply[depth - 2] = n_perft;
      phashe->key = key;
    }
    else {
      perft->nCollisions++;
    }
  }
  return n_perft;
}

void
PerftThread::run (  ) {

  int ii;
  struct timeb start1, end1;
  int TimeTaken = 0;

  ftime ( &start1 );
  init (  );
  _Tmove *move;
  incListId (  );
  resetList (  );

  u64 friends = getBitBoard ( sideToMove );
  u64 enemies = getBitBoard ( sideToMove ^ 1 );
  u64 dummy = 0;
  generateCap ( sideToMove, enemies, friends, &dummy );
  generateMoves ( sideToMove, friends | enemies );
  u64 tot = 0;
  u64 key = makeZobristKey (  );
  u64 keyold = key;

  for ( ii = to; ii >= from; ii-- ) {
    u64 n_perft = 0;
    move = getList ( ii );

    makemove ( move, &key );
    n_perft = search ( sideToMove ^ 1, perft->mainDepth, key );
    takeback ( move, &key, keyold );

    char y;
    char x = FEN_PIECE[sideToMove == WHITE ? getPieceAtWhite ( TABLOG[move->from] ) : getPieceAtBlack ( TABLOG[move->from] )];
    if ( x == 'p' || x == 'P' )
      x = ' ';
    if ( move->capturedPiece != SQUARE_FREE )
      y = '*';
    else
      y = '-';
    cout << "\n#" << ii << " cpuID# " << cpuID;
    cout << "\t" << x << decodeBoardinv ( move->type, move->from, sideToMove ) << y << decodeBoardinv ( move->type, move->to, sideToMove ) << "\t" << n_perft << " ";
    cout << flush;
    tot += n_perft;
  }
  decListId (  );
  ftime ( &end1 );
  TimeTaken = diff_time ( end1, start1 );
  cout << "\n" << tot << " nodes in " << ( double ) TimeTaken / 1000 << " seconds";

  perft->setResult ( tot );
#ifdef DEBUG_MODE
  if ( TimeTaken > 500 ) {
    cout << endl << "nodes per second " << tot * 1000 / TimeTaken;
  }
#endif
  cout << endl << flush;

}
PerftThread::~PerftThread (  ) {
}

#endif
