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

#include "stdafx.h"
#ifdef HASH_MODE
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
#ifdef DEBUG_MODE
  assert ( j != -1 );
#endif
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

};

int
search_openbook ( const u64 key, const int side ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int t = search_book ( 0, OPENBOOK_SIZE - 1, key );
  if ( t == -1 )
    return -1;
  if ( side == -1 )
    return t;
  if ( side == WHITE && openbook[t].da_white != -1 )
    return t;
  if ( side == BLACK && openbook[t].da_black != -1 )
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
};



void
swap ( Topenbook * a, Topenbook * b ) {
  Topenbook t;
  memcpy ( &t, a, sizeof ( Topenbook ) );
  memcpy ( a, b, sizeof ( Topenbook ) );
  memcpy ( b, &t, sizeof ( Topenbook ) );
}

void
QuickSort_book ( int beg, int end ) {
  int l, r;
//      printf("\n%d %d",beg,end);
  if ( end > beg + 1 ) {
    u64 piv = openbook[beg].key;
    l = beg + 1;
    r = end;
    while ( l < r ) {
      if ( openbook[l].key <= piv )
	l++;
      else
	swap ( &openbook[l], &openbook[--r] );
    }
    swap ( &openbook[--l], &openbook[beg] );
    QuickSort_book ( beg, l );
    QuickSort_book ( r, end );
  }
}

void
insert_openbook_leaf ( TopenbookLeaf ** root, Topenbook * x ) {

  if ( !( *root ) ) {
    openbookLeaf_count++;
    ( *root ) = ( TopenbookLeaf * ) calloc ( 1, sizeof ( TopenbookLeaf ) );
    ( *root )->a_black = x->a_black;
    ( *root )->da_black = x->da_black;
    ( *root )->a_white = x->a_white;
    ( *root )->da_white = x->da_white;
    ( *root )->eval = x->eval;
    ( *root )->key = x->key;
    ( *root )->l = NULL;
    ( *root )->r = NULL;

    return;
  }
  if ( x->key == ( *root )->key ) {

    return;
  }
  else if ( x->key > ( *root )->key ) {
    if ( ( *root )->r == NULL ) {
      openbookLeaf_count++;
      ( *root )->r = ( TopenbookLeaf * ) calloc ( 1, sizeof ( TopenbookLeaf ) );
      ( *root )->r->a_black = x->a_black;
      ( *root )->r->da_black = x->da_black;
      ( *root )->r->a_white = x->a_white;
      ( *root )->r->da_white = x->da_white;
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
      ( *root )->l->a_black = x->a_black;
      ( *root )->l->da_black = x->da_black;
      ( *root )->l->a_white = x->a_white;
      ( *root )->l->da_white = x->da_white;
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
serializza_book ( TopenbookLeaf * root ) {

  if ( !root )
    return;

  serializza_book ( root->r );
  serializza_book ( root->l );
#ifdef DEBUG_MODE
  assert ( root );
#endif
  openbook[ob_count].a_black = root->a_black;
  openbook[ob_count].da_black = root->da_black;
  openbook[ob_count].a_white = root->a_white;
  openbook[ob_count].da_white = root->da_white;
  openbook[ob_count].eval = root->eval;
  openbook[ob_count].key = root->key;

  if ( openbook[ob_count].da_black != -1 )
    if ( openbook[ob_count].a_black < 0 )
      printf ( "\nerror" );;
  if ( openbook[ob_count].da_white != -1 )
    if ( openbook[ob_count].a_white <= 0 )
      printf ( "\nerror" );;

  ob_count++;
};

int
load_open_book (  ) {
  long s;
  FILE *F;
  //printf ("load book...");

  OPENBOOK_SIZE = fileLung ( OPENBOOK_FILE ) / sizeof ( Topenbook );
  if ( !OPENBOOK_SIZE )
    return 0;
  openbook = ( Topenbook * ) calloc ( 1, OPENBOOK_SIZE * sizeof ( Topenbook ) );

  if ( !openbook ) {
    printf ( "\nerror" );
    return 0;
  }


  if ( ( F = fopen ( OPENBOOK_FILE, "r+b" ) ) == NULL ) {
    printf ( "\n%s not found\n", OPENBOOK_FILE );
    free ( openbook );
    return 0;
  }

  s = fread ( openbook, 1, OPENBOOK_SIZE * sizeof ( Topenbook ), F );
  if ( s != ( long ) ( OPENBOOK_SIZE * sizeof ( Topenbook ) ) ) {
    printf ( "\nerror31" );
    free ( openbook );
    fclose ( F );
    return 0;
  }
  fclose ( F );
  return 1;

}

/*
void
update_open_book_eval (const char *TXT_FILE)
{
  u64 u;

  printf ("\nupdate_open_book_eval...");
  FILE *stream;
  char *line = (char *) calloc (1, 1000);
  int c = 0;

  stream = fopen (TXT_FILE, "r+b");
#ifdef DEBUG_MODE
  assert (stream);
#endif

  while (fgets (line, 1000, stream) != NULL)
    {
      c++;
      if (!(c % 10000))
	printf ("\n%d", c);
      loadfen (line);
      u = makeZobristKey ();
#ifdef DEBUG_MODE
      assert (u);
#endif
      TopenbookLeaf *i = search_book_tree (openbook_tree, u);
      if (i != NULL)
	{
	  i->eval = eval (WHITE, _INFINITE, 0
#ifdef HASH_MODE
			  , 0
#endif
	    );
	}
      else
	{
	  Topenbook e;
	  e.key = u;
	  e.da_white = -1;
	  e.da_black = -1;
	  e.eval = eval (WHITE, _INFINITE, 0
#ifdef HASH_MODE
			 , 0
#endif
	    );
	  insert_openbook_leaf (&openbook_tree, &e);
	}
    }
  fclose (stream);
  free (line);

}
*/

void
create_open_book ( const char *ENORMOUS_TXT_FILE ) {

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

  stream = fopen ( ENORMOUS_TXT_FILE, "r+b" );

  if ( !stream ) {
    printf ( "\n%s not found. (get it in ftp://ftp.cis.uab.edu/pub/hyatt/pgn/enormous.zip and convert it with pgn2epd.exe < enormous.pgn >enormous.epd. http://remi.coulom.free.fr/)", ENORMOUS_TXT_FILE );
    return;
  }
  int aa = 0;
  while ( fgets ( line, 1000, stream ) != NULL ) {
    // strcpy(line,"5bk1/3nq1np/2b1p1p1/3pPp2/2pP1QPP/4NN2/2P2PBK/4B3 w - f6 bm exf6;");
    printf ( "%d %s", ++aa, line );

    score = -_INFINITE;
    init (  );
    loadfen ( line );
    //print();
    u = makeZobristKey (  );
#ifdef DEBUG_MODE
    assert ( u );
#endif
    if ( strstr ( line, " b " ) )
      side = 0;
    else if ( strstr ( line, " w " ) )
      side = 1;
    else
      myassert ( 0, "error" );
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
    // printf("\n%d %d",from,to);  


    Topenbook e;
    i = search_book_tree ( openbook_tree, u );
    if ( from >= 0 && i == NULL ) {

      if ( side == WHITE ) {

	e.key = u;
	e.da_white = ( char ) from;
	e.a_white = ( char ) to;
	e.da_black = -1;
	e.eval = score;
	insert_openbook_leaf ( &openbook_tree, &e );
      }
      else {

	e.key = u;
	e.da_black = ( char ) from;
	e.a_black = ( char ) to;
	e.da_white = -1;
	e.eval = score;
	insert_openbook_leaf ( &openbook_tree, &e );
      };
    }

  }

  fclose ( stream );

  //update_open_book_eval (UPDATE_TXT_FILE);
  openbook = ( Topenbook * ) calloc ( 1, openbookLeaf_count * sizeof ( Topenbook ) );
#ifdef DEBUG_MODE
  assert ( openbook );
#endif
  printf ( "\nserializza_book..." );
  serializza_book ( openbook_tree );
  printf ( "\nQuickSort_book..." );
  QuickSort_book ( 0, openbookLeaf_count );
  printf ( "\nscrivo %d nodes...", openbookLeaf_count );
  stream = fopen ( OPENBOOK_FILE, "w+b" );
  fwrite ( openbook, 1, openbookLeaf_count * sizeof ( Topenbook ), stream );
  fclose ( stream );
  printf ( "\nfine" );
}
#endif
#endif
