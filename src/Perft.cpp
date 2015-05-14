#ifdef PERFT_MODE
#ifndef TEST_MODE
#include "Perft.h"
#include <iomanip>
Perft::Perft ( int cpuID, char *fen, int from, int to, RootPerft * rootPerft ):
GenMoves ( fen ) {
  this->cpuID = cpuID;
  this->rootPerft = rootPerft;
  this->from = from;
  this->to = to;
}

RootPerft::~RootPerft (  ) {
  for ( int i = 0; i < HASH_SIZE; i++ )
    free ( hash[i].nMovesXply );
  free ( hash );
}

RootPerft::RootPerft ( char *fen, int depth, int nCpu, int hash_size ):
GenMoves ( fen ) {
  struct timeb start1, end1;
  TOT = nCollisions = 0;

  this->HASH_SIZE = hash_size * 1024 * 1000 / ( sizeof ( u64 ) * depth + sizeof ( ThashPerft ) );
  this->hash = NULL;
  if ( HASH_SIZE ) {
    hash = ( ThashPerft * ) calloc ( HASH_SIZE, sizeof ( ThashPerft ) );
    myassert ( hash );
    for ( int i = 0; i < HASH_SIZE; i++ ) {
      this->hash[i].key = 0xffffffffffffffffULL;
      hash[i].nMovesXply = ( u64 * ) malloc ( ( depth - 1 ) * sizeof ( u64 ) );
      myassert ( hash[i].nMovesXply );
      for ( int j = 0; j < depth - 1; j++ ) {
	hash[i].nMovesXply[j] = 0xffffffffffffffffULL;
      }
    }
    cout << "ok\n" << flush;
  }
  ftime ( &start1 );
  this->mainDepth = depth - 1;
  this->nCpu = nCpu;
  incListId (  );
  generateCap (  );
  generateMoves (  );
  int listcount = getListCount (  );
  resetList (  );
  decListId (  );
  myassert ( nCpu > 0 );
  int block = listcount / nCpu - 1;
  int s = 1;
  cout << "split:";
  for ( int i = 0; i < nCpu - 1; i++ ) {
    cout << " cpu#" << i << " from: " << s << " to: " << s + block << flush;
    perftList.push_back ( new Perft ( i, fen, s, s + block, this ) );
    perftList[i]->start (  );
    s += block + 1;
  }
  cout << " cpu#" << nCpu - 1 << " from: " << s << " to: " << listcount << flush;
  perftList.push_back ( new Perft ( nCpu - 1, fen, s, listcount, this ) );
  perftList[nCpu - 1]->start (  );
  for ( int i = 0; i < nCpu; i++ ) {
    perftList[i]->join (  );
    delete perftList[i];
  }
  ftime ( &end1 );
  cout << "\ncollisions: " << nCollisions << endl << flush;
  cout << "TOT: [" << TOT << "] in " << setprecision ( 2 ) << diff_time ( end1, start1 ) / 1000.0 << " seconds" << endl;

}

u64
Perft::search ( const int SIDE, int depth ) {
  if ( depth == 0 ) {
    return 1;
  }
  u64 key = 0;
  ThashPerft *phashe = NULL;
  if ( depth >= 2 && rootPerft->hash ) {
    key = makeZobristKey ( SIDE );
    phashe = &( rootPerft->hash[key % rootPerft->HASH_SIZE] );
    if ( key == phashe->key && phashe->nMovesXply[depth - 2] != 0xffffffffffffffffULL ) {
      return phashe->nMovesXply[depth - 2];
    }
  }
  u64 n_perft = 0;
  int ii, listcount;
  Tmove *mossa;
  incListId (  );
  if ( generateCap ( STANDARD_MOVE_MASK, SIDE ) ) {
    decListId (  );
    return 0;
  }
  generateMoves ( STANDARD_MOVE_MASK, SIDE );
  listcount = getListCount (  );
  if ( !listcount ) {
    decListId (  );
    return 0;
  }
  for ( ii = 1; ii <= listcount; ii++ ) {
    mossa = getList ( ii );
    makemove ( mossa );
    n_perft += search ( SIDE ^ 1, depth - 1 );
    takeback ( mossa );
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
      rootPerft->nCollisions++;
    }
  }
  return n_perft;
}

void
Perft::run (  ) {

  int ii, listcount;
  struct timeb start1, end1;
  int TimeTaken = 0;

  ftime ( &start1 );
  init (  );
  Tmove *mossa;
  incListId (  );
  resetList (  );
  generateCap (  );
  generateMoves (  );
  listcount = getListCount (  );
  u64 tot = 0;
  for ( ii = to; ii >= from; ii-- ) {
    u64 n_perft1 = 0;
    mossa = getList ( ii );
    u64 key = 0;
    if ( rootPerft->hash )
      makeZobristKey ( ( !blackMove ) ^ 1 );
    makemove ( mossa );
    n_perft1 = search ( ( !blackMove ) ^ 1, rootPerft->mainDepth );
    takeback ( mossa );
    char y;
    char x = FEN_PIECE[get_piece_at ( ( !blackMove ), TABLOG[mossa->from] )];
    if ( x == 'p' || x == 'P' )
      x = ' ';
    if ( mossa->capture != 12 )
      y = '*';
    else
      y = '-';
    cout << "\n#" << ii << " cpuID# " << cpuID;
    cout << "\t" << x << decodeBoardinv ( mossa->type, mossa->from, !blackMove ) << y << decodeBoardinv ( mossa->type, mossa->to, !blackMove ) << "\t" << n_perft1 << " ";
    cout << flush;
    tot += n_perft1;
  }
  decListId (  );
  ftime ( &end1 );
  TimeTaken = diff_time ( end1, start1 );
  cout << "\n" << tot << " nodes in " << ( double ) TimeTaken / 1000 << " seconds";

  rootPerft->setResult ( tot );
#ifdef DEBUG_MODE
  if ( TimeTaken > 500 ) {
    cout << endl << "nodes per second " << tot * 1000 / TimeTaken;
  }
#endif
  cout << endl << flush;

}
Perft::~Perft (  ) {
}

#endif
#endif
