#include "stdafx.h"
#ifndef PERFT_MODE
#include "maindefine.h"
#include "utility.h"
#include "search.h"
#include "gen.h"
#include "cap.h"
#include "eval.h"
#include "zobrist.h"
#ifdef _MSC_VER
#include "LIMITS.H"
#endif
#include "extern.h"
#include "math.h"
int openbookLeaf_count = 0;
int
search_book ( const int i, const int j, const u64 key ) {
  ASSERT ( j != -1 );
  int k = ( int ) ceil ( ( j + i ) / 2.0 );
  if ( key == openbook[k].key )
    return k;
  else if ( key > openbook[k].key ) {
    if ( i != j )
      return search_book ( k, j, key );
  }
  else {
    if ( i != j )
      return search_book ( i, k - 1, key );
  }
  return -1;
}

int
search_openbook ( const u64 key, const int side ) {
  check_side ( side );
  int t = search_book ( 0, OPENBOOK_SIZE - 1, key );
  if ( t == -1 )
    return -1;
  if ( side == -1 )
    return t;
  if ( side == WHITE && openbook[t].from_white != -1 )
    return t;
  if ( side == BLACK && openbook[t].from_black != -1 )
    return t;
  return -1;
}

TopenbookLeaf *
search_book_tree ( TopenbookLeaf * root, const u64 key ) {
  if ( !root )
    return NULL;
  if ( root->key == key )
    return root;
  if ( key > root->key )
    return search_book_tree ( root->r, key );
  return search_book_tree ( root->l, key );
}

;

int
compare_openbook ( const void *a, const void *b ) {
  Topenbook *arg1 = ( Topenbook * ) a;
  Topenbook *arg2 = ( Topenbook * ) b;
  if ( arg1->key > arg2->key )
    return 1;
  else if ( arg1->key == arg2->key )
    return 0;
  else
    return -1;
}

void
insert_openbook_leaf ( TopenbookLeaf ** root, Topenbook * x ) {
  if ( !( *root ) ) {
    openbookLeaf_count++;
    ( *root ) = ( TopenbookLeaf * ) calloc ( 1, sizeof ( TopenbookLeaf ) );
    ( *root )->to_black = x->to_black;
    ( *root )->from_black = x->from_black;
    ( *root )->to_white = x->to_white;
    ( *root )->from_white = x->from_white;
    ( *root )->eval = x->eval;
    ( *root )->key = x->key;
    ( *root )->l = NULL;
    ( *root )->r = NULL;
    return;
  }
  if ( x->key == ( *root )->key )
    return;
  else if ( x->key > ( *root )->key ) {
    if ( ( *root )->r == NULL ) {
      openbookLeaf_count++;
      ( *root )->r = ( TopenbookLeaf * ) calloc ( 1, sizeof ( TopenbookLeaf ) );
      ( *root )->r->to_black = x->to_black;
      ( *root )->r->from_black = x->from_black;
      ( *root )->r->to_white = x->to_white;
      ( *root )->r->from_white = x->from_white;
      ( *root )->r->eval = x->eval;
      ( *root )->r->key = x->key;
      ( *root )->r->l = NULL;
      ( *root )->r->r = NULL;
      return;
    }
    else {
      insert_openbook_leaf ( &( ( *root )->r ), x );
      return;
    }
  }
  else if ( x->key < ( *root )->key ) {
    if ( ( *root )->l == NULL ) {
      openbookLeaf_count++;
      ( *root )->l = ( TopenbookLeaf * ) calloc ( 1, sizeof ( TopenbookLeaf ) );
      ( *root )->l->to_black = x->to_black;
      ( *root )->l->from_black = x->from_black;
      ( *root )->l->to_white = x->to_white;
      ( *root )->l->from_white = x->from_white;
      ( *root )->l->eval = x->eval;
      ( *root )->l->key = x->key;
      ( *root )->l->l = NULL;
      ( *root )->l->r = NULL;
      return;
    }
    else {
      insert_openbook_leaf ( &( ( *root )->l ), x );
      return;
    }
  }
}

void
serialize_book ( TopenbookLeaf * root ) {
  if ( !root )
    return;
  int er = 0;
  serialize_book ( root->r );
  serialize_book ( root->l );
  ASSERT ( root );
  openbook[ob_count].to_black = root->to_black;
  openbook[ob_count].from_black = root->from_black;
  openbook[ob_count].to_white = root->to_white;
  openbook[ob_count].from_white = root->from_white;
  openbook[ob_count].eval = root->eval;
  openbook[ob_count].key = root->key;
  if ( openbook[ob_count].from_black != -1 )
    if ( openbook[ob_count].to_black < 0 )
      er = 1;
  if ( openbook[ob_count].from_white != -1 )
    if ( openbook[ob_count].to_white < 0 )
      er = 1;
  if ( !er )
    ob_count++;
}

int
load_open_book (  ) {
  long s;
  FILE *F;
  OPENBOOK_SIZE = fileLung ( OPENBOOK_FILE ) / sizeof ( Topenbook );
  if ( !OPENBOOK_SIZE )
    return 0;
  openbook = ( Topenbook * ) calloc ( 1, OPENBOOK_SIZE * sizeof ( Topenbook ) );
  if ( !openbook ) {
    printf ( "\nerror" );
    return 0;
  }
  if ( ( F = fopen ( OPENBOOK_FILE, "r+b" ) ) == NULL ) {
    return 0;
  }
  s = ( long ) fread ( openbook, 1, OPENBOOK_SIZE * sizeof ( Topenbook ), F );
  if ( s != ( long ) ( OPENBOOK_SIZE * sizeof ( Topenbook ) ) ) {
    printf ( "\nerror" );
    free ( openbook );
    fclose ( F );
    return 0;
  }
  fclose ( F );
  return 1;
}

void
create_open_book ( const char *BOOK_TXT_FILE ) {
  if ( openbook ) {
    free ( openbook );
    openbook = 0;
    use_book = 0;
  };
  u64 u = 0;
  char *r1;
  char *r2;
  char dummy[20];
  int side = -1;
  TopenbookLeaf *i;
  int score = -1;
  openbook_tree = NULL;
  printf ( "\nCREATE BOOK..." );
  FILE *stream;
  char line[1000];
  int from, to, k;
  stream = fopen ( BOOK_TXT_FILE, "r+b" );
  if ( !stream ) {
    printf ( "\n%s not found. (use  pgn2epd.exe < strong_match.pgn >book.epd)", BOOK_TXT_FILE );
    return;
  }
  int count = 0;
  while ( fgets ( line, 1000, stream ) != NULL ) {
    count++;
    score = -_INFINITE;
    init (  );
    loadfen ( line );
    int totP = n_pieces ( WHITE ) + n_pieces ( BLACK ) + BitCount ( chessboard[PAWN_WHITE] ) + BitCount ( chessboard[PAWN_BLACK] );
    if ( totP != 32 && totP > 4 )
      continue;
    printf ( "%d %s", count, line );
    if ( strstr ( line, " b " ) )
      side = 0;
    else if ( strstr ( line, " w " ) )
      side = 1;
    else
      myassert ( 0, "error" );
    u = makeZobristKey ( side );
    ASSERT ( u );
    k = 0;
    r1 = strstr ( line, ";" );
    if ( ( r1 - 1 )[0] == '+' )
      r1--;
    if ( ( r1 - 1 )[0] == '#' )
      r1--;
    while ( r1[k] != ' ' )
      k--;
    r2 = r1 + k + 1;
    memset ( dummy, 0, sizeof ( dummy ) );
    strncpy ( dummy, r2, r1 - r2 );
    score = fen2pos ( dummy, &from, &to, side, u );
    Topenbook e;
    i = search_book_tree ( openbook_tree, u );
    if ( from >= 0 && i == NULL ) {
      if ( side == WHITE ) {
	e.key = u;
	e.from_white = ( char ) from;
	e.to_white = ( char ) to;
	e.from_black = -1;
	e.eval = score;
	insert_openbook_leaf ( &openbook_tree, &e );
      }
      else {
	e.key = u;
	e.from_black = ( char ) from;
	e.to_black = ( char ) to;
	e.from_white = -1;
	e.eval = score;
	insert_openbook_leaf ( &openbook_tree, &e );
      };
    }
  }
  fclose ( stream );
  //update_open_book_eval (UPDATE_TXT_FILE);
  openbook = ( Topenbook * ) calloc ( 1, openbookLeaf_count * sizeof ( Topenbook ) );
  ASSERT ( openbook );
  printf ( "\nserialize_book..." );
  serialize_book ( openbook_tree );
  printf ( "\nsort_book..." );
  qsort ( openbook, openbookLeaf_count, sizeof ( Topenbook ), compare_openbook );
  printf ( "\nwrite %d nodes...", openbookLeaf_count );
  stream = fopen ( OPENBOOK_FILE, "w+b" );
  fwrite ( openbook, 1, openbookLeaf_count * sizeof ( Topenbook ), stream );
  fclose ( stream );
  printf ( "\nend\n" );
}
#endif
