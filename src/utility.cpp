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
#include <time.h>
#include "maindefine.h"
#include "winboard.h"
#include "gen.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "extern.h"
#include "zobrist.h"
#include "search.h"

int
compare_int ( const void *a, const void *b ) {
  int *arg1 = ( int * ) a;
  int *arg2 = ( int * ) b;
  if ( *arg1 < *arg2 )
    return -1;
  else if ( *arg1 == *arg2 )
    return 0;
  else
    return 1;
}

int
compare_move ( const void *a, const void *b ) {
  Tmove *arg1 = ( Tmove * ) a;
  Tmove *arg2 = ( Tmove * ) b;
  if ( arg1->score < arg2->score )
    return -1;
  else if ( arg1->score == arg2->score )
    return 0;
  else
    return 1;
}

int
is_locked ( int pos, int pezzo, int side ) {
  int r = 0;

  /*  if ((tipo=IN_LINEA[pos][re_amico[side]]))
     {    */
  chessboard[pezzo] &= NOTTABLOG[pos];
  r = attack_square ( side, re_amico[side] );
  chessboard[pezzo] |= TABLOG[pos];
  //}
  //r+=in_check();
  //r= attack_square (side, re_amico[side]);
  return r;
}

int
BitCountSlow ( const u64 b ) {
  unsigned buf;
  register unsigned acc;
  buf = ( unsigned ) b;
  acc = buf;
  acc -= ( ( buf &= 0xEEEEEEEEUL ) >> 1 );
  acc -= ( ( buf &= 0xCCCCCCCCUL ) >> 2 );
  acc -= ( ( buf &= 0x88888888UL ) >> 3 );
  buf = ( unsigned ) ( b >> 32 );
  acc += buf;
  acc -= ( ( buf &= 0xEEEEEEEEUL ) >> 1 );
  acc -= ( ( buf &= 0xCCCCCCCCUL ) >> 2 );
  acc -= ( ( buf &= 0x88888888UL ) >> 3 );
  acc = ( acc & 0x0F0F0F0FUL ) + ( ( acc >> 4 ) & 0x0F0F0F0FUL );
  acc = ( acc & 0xFFFF ) + ( acc >> 16 );
  return ( ( acc & 0xFF ) + ( acc >> 8 ) );

};

/*
int piece_attack_defence (const int a,const int coloreAttaccato){
//restituisce +-= se e  conveniente fare la mossa
int c, rr ;
int d0, primo = 1;
Tchessboard chessboard2;
d0 = PIECES_VALUE[ get_piece_at (coloreAttaccato, TABLOG[a])];
memcpy (chessboard2, chessboard, sizeof (Tchessboard));
int result,xside;
u64 attaccanti;
u64 difensori;
xside = coloreAttaccato ^ 1;
attaccanti = 0;
difensori = 0;
double A=0,D=0;
int t;
int da_xside = 0;
int da_side = 0;
do{
attaccanti=calcola_attaccanti (a, coloreAttaccato);
if (!attaccanti && primo == 1)
return 0;
primo++;
c = 0;
while (attaccanti){
c = 1;
t = BitScanForward (attaccanti);
rr = get_piece_at (xside, TABLOG[t]);
#ifdef DEBUG_MODE
assert (rr != SQUARE_FREE);

#endif
A += (double)VALUEPAWN/(0x7FF & PIECES_VALUE[rr]);
chessboard[rr] &= NOTTABLOG[t];
attaccanti &= NOTTABLOG[t];
}
}while (c);
memcpy (chessboard, chessboard2, sizeof (Tchessboard));
do{
difensori=calcola_attaccanti (a, xside);
c = 0;
while (difensori){
c = 1;
t = BitScanForward (difensori);
rr = get_piece_at (coloreAttaccato, TABLOG[t]);
#ifdef DEBUG_MODE
assert (rr != SQUARE_FREE);
#endif
D+= (double)VALUEPAWN/(0x7FF & PIECES_VALUE[rr]);
chessboard[rr] &= NOTTABLOG[t];
difensori &= NOTTABLOG[t];
}
}while (c );
if (!D)
result=d0;
else
result=(int)(d0*A/D);
memcpy (chessboard, chessboard2, sizeof (Tchessboard));
if (result>5000)
result=980;

return result/100;

}

*/
void
init (  ) {

  ENP_POSSIBILE = -1;
  num_moves2 = num_moves = num_movesq = mate = 0;
  pvv_da = 65;
#ifdef FP_MODE
#ifdef DEBUG_MODE
  n_cut_fp = n_cut_razor = 0;
#endif
#endif
  FLG_WIN_WHITE = false;
  FLG_WIN_BLACK = false;
  memset ( gen_list, 0, sizeof ( gen_list ) );
  list_id = -1;
  maxdep = evaluateMobility_mode = EvalCuts = 0;

#ifdef PERFT_MODE
  n_perft = 0;
  listcount_n = 0;
#else


#ifdef DEBUG_MODE
  beta_efficency = 0.0;
  n_cut = 0;
  null_move_cut = 0;
#endif
  memset ( HistoryHeuristic, 0, sizeof ( HistoryHeuristic ) );
  memset ( KillerHeuristic, 0, sizeof ( KillerHeuristic ) );
#endif
  null_sem = 0;

#ifdef HASH_MODE

  memset ( hash_array[BLACK], 0, HASH_SIZE * sizeof ( Thash ) );
  memset ( hash_array[WHITE], 0, HASH_SIZE * sizeof ( Thash ) );

#ifdef DEBUG_MODE
  n_cut_hash = n_record_hash = collisions = 0;
#endif
#endif

}

/*
char slowLSB (int i){
char k = -1;
while (i){
k++;
if (i & 1)
break;
i >>= 1;
}
return k;
}
*/
char *
lowercase ( char *a ) {
  for ( unsigned t = 0; t < strlen ( a ); t++ )
    a[t] = ( char ) tolower ( a[t] );
  return a;
}

char *
trim ( char *str ) {
  char *s, *dst, *last = NULL;

  s = dst = str;

  while ( *s && isspace ( *s ) )
    s++;
  if ( !*s ) {
    *str = '\0';
    return str;
  }

  do {
    if ( !isspace ( *s ) )
      last = dst;
    *dst++ = *s++;
  }
  while ( *s );

  *( last + 1 ) = '\0';

  return str;

}

int
pushmove ( const int tipomove, const int da, const int a, const int SIDE ) {
  return pushmove ( tipomove, da, a, SIDE, -1 );
}

int
pushmove ( const int tipomove, const int da, const int a, const int SIDE, int promotion_piece ) {

#ifdef PERFT_MODE
  int t;
#endif

#ifdef DEBUG_MODE
  assert ( chessboard[KING_BLACK] );
  assert ( chessboard[KING_WHITE] );

#endif
  //if (is_locked (da, pezzo))
  //return;
  //if(attack_square (SIDE, re_amico))
  //  print();
  ///int re=BitScanForward(chessboard[KING_BLACK+XSIDE]);re_nemico
#ifndef PERFT_MODE
  if ( evaluateMobility_mode ) {
    if ( da >= 0 ) {		//TODO gestire arrocco
#ifdef DEBUG_MODE
      assert ( da >= 0 && a >= 0 );
#endif
      //        int pezzoda = get_piece_at (SIDE, TABLOG[da]);

      evalNode.attaccate[da] |= TABLOG[a];

      evalNode.king_attak[SIDE] += DISTANCE[re_amico[SIDE ^ 1]][a];

      if ( ( TABLOG[a] & chessboard[KING_BLACK + SIDE] ) )
	evalNode.re_attaccato[da] = 1;
      evalNode.attaccanti[a] |= TABLOG[da];
#ifdef DEBUG_MODE
      assert ( gen_list[list_id][0].score < MAX_MOVE );
#endif
      gen_list[list_id][0].score++;
      return 0;
    }
    return 0;
  }
#endif
  int pezzoa;
  if ( tipomove != ENPASSANT ) {
    pezzoa = get_piece_at ( ( SIDE ^ 1 ), TABLOG[a] );
    if ( ( pezzoa == KING_BLACK ) || ( pezzoa == KING_WHITE ) )
      return 1;
  }
  else
    pezzoa = SIDE ^ 1;
  int pezzoda = get_piece_at ( SIDE, TABLOG[da] );
#ifdef PERFT_MODE
  if ( ( t = inCheck ( da, a, tipomove, pezzoda, pezzoa, SIDE, promotion_piece ) ) == 1 )
    return 0;

#endif
  Tmove *mos;
#ifndef PERFT_MODE
#endif
#ifdef DEBUG_MODE
  assert ( list_id != -1 );
  assert ( gen_list[list_id][0].score < MAX_MOVE );
#endif
  mos = &gen_list[list_id][gen_list[list_id][0].score + 1];
  if ( tipomove == STANDARD || tipomove == ENPASSANT || tipomove == PROMOTION ) {
    mos->type = ( char ) tipomove;
    mos->from = ( char ) da;
    mos->to = ( char ) a;
    mos->side = ( char ) SIDE;
    mos->promotion_piece = ( char ) promotion_piece;
#ifndef PERFT_MODE
    mos->score = 0;
    //mvv lva
    if ( da == pvv_da && a == pvv_a )
      mos->score += 30000;
    ( PIECES_VALUE[pezzoa] >= PIECES_VALUE[pezzoda] ) ? mos->score += ( PIECES_VALUE[pezzoa] - PIECES_VALUE[pezzoda] ) * 2 : mos->score += PIECES_VALUE[pezzoa];
    mos->score += HistoryHeuristic[da][a];
    mos->score += KillerHeuristic[main_depth][da][a];
    mos->score += ( MOVE_ORDER[pezzoda][a] - MOVE_ORDER[pezzoda][da] );

#endif

  }
  else if ( tipomove == CASTLE ) {
    mos->type = ( char ) tipomove;
    mos->from = ( char ) da;	//corto lungo
    mos->side = ( char ) a;
    mos->capture = SQUARE_FREE;
#ifndef PERFT_MODE
    mos->score = 100;
#endif
  };
#ifdef DEBUG_MODE
  assert ( gen_list[list_id][0].score < MAX_MOVE );
#endif
  gen_list[list_id][0].score++;
  return 0;
};

void
un_make_castle ( const int da, const int SIDE ) {
  if ( SIDE == WHITE ) {
    if ( da == KINGSIDE ) {
#ifdef DEBUG_MODE
      assert ( get_piece_at ( SIDE, TABLOG_1 ) == KING_WHITE );
      assert ( get_piece_at ( SIDE, TABLOG_0 ) == 12 );
      assert ( get_piece_at ( SIDE, TABLOG_3 ) == 12 );
      assert ( get_piece_at ( SIDE, TABLOG_2 ) == ROOK_WHITE );
#endif
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_3 ) & NOTTABLOG_1;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_0 ) & NOTTABLOG_2;
    }
    else {
      chessboard[KING_WHITE] = ( chessboard[KING_WHITE] | TABLOG_3 ) & NOTTABLOG_5;
      chessboard[ROOK_WHITE] = ( chessboard[ROOK_WHITE] | TABLOG_7 ) & NOTTABLOG_4;
    }
  }
  else {
    if ( da == KINGSIDE ) {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_59 ) & NOTTABLOG_57;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_56 ) & NOTTABLOG_58;
    }
    else {
      chessboard[KING_BLACK] = ( chessboard[KING_BLACK] | TABLOG_59 ) & NOTTABLOG_61;
      chessboard[ROOK_BLACK] = ( chessboard[ROOK_BLACK] | TABLOG_63 ) & NOTTABLOG_60;
    }
  }
}



int
attack_square ( const int side, const int Position ) {
#ifdef DEBUG_MODE
  check_side ( side );
  assert ( Position != -1 );
#endif
  int Position_Position_mod_8, xside;
  char Position_mod_8;
  u64 ALLPIECES;
#ifdef DEBUG_MODE
  assert ( BitCount ( chessboard[KING_BLACK + side] ) < 2 );
#endif
  xside = side ^ 1;
  if ( KNIGHT_MASK[Position] & chessboard[KNIGHT_BLACK + xside] )
    return 1;
  if ( KING_MASK[Position] & chessboard[KING_BLACK + xside] )
    return 1;
  //enpassant
  if ( PAWN_CAPTURE_MASK[side][Position] & chessboard[PAWN_BLACK + xside] )
    return 1;
  if ( !( VERT_ORIZZ[Position] & ( chessboard[ROOK_BLACK + xside] | chessboard[QUEEN_BLACK + xside] ) | LEFT_RIGHT[Position] & ( chessboard[QUEEN_BLACK + xside] | chessboard[BISHOP_BLACK + xside] ) ) ) {
    return 0;
  }

  ALLPIECES = square_all_bit_occupied (  ) | TABLOG[Position];
  Position_mod_8 = ROT45[Position];
  Position_Position_mod_8 = pos_posMod8[Position];
  if ( MOVIMENTO_MASK_CAT[( uchar ) ( ( shr ( ALLPIECES, Position_Position_mod_8 ) ) )][Position_mod_8] & ( shr ( chessboard[ROOK_BLACK + xside], Position_Position_mod_8 ) & 255 | shr ( chessboard[QUEEN_BLACK + xside], Position_Position_mod_8 ) & 255 ) ) {
    //   print();
    return 1;
  }
  //left

#ifdef DEBUG_MODE
  assert ( rotate_board_left_45 ( ALLPIECES, Position ) != 0 );
  if ( ( ( uchar ) ( rotate_board_left_45 ( ALLPIECES, Position ) & MOVES_BISHOP_LEFT_MASK[Position] ) ) != rotate_board_left_45 ( ALLPIECES, Position ) ) {


    assert ( 0 );
  }
  assert ( rotate_board_left_45 ( ALLPIECES, Position ) != 0 );
  assert ( rotate_board_left_45 ( ALLPIECES, Position ) == ( rotate_board_left_45 ( ALLPIECES, Position ) & MOVES_BISHOP_LEFT_MASK[Position] ) );
#endif
  if ( MOVIMENTO_MASK_CAT[rotate_board_left_45 ( ALLPIECES, Position )][Position_mod_8] & ( rotate_board_left_45 ( chessboard[BISHOP_BLACK + ( xside )],	//TODO ridurre
														   Position ) | rotate_board_left_45 ( chessboard[QUEEN_BLACK + xside], Position ) ) )
    return 1;

  /*right \ */

#ifdef DEBUG_MODE
  assert ( rotate_board_right_45 ( ALLPIECES, Position ) != -1 );
  assert ( rotate_board_right_45 ( ALLPIECES, Position ) == ( uchar ) ( rotate_board_right_45 ( ALLPIECES, Position ) & MOVES_BISHOP_RIGHT_MASK[Position] ) );
#endif
  if ( MOVIMENTO_MASK_CAT[rotate_board_right_45 ( ALLPIECES, Position ) | TABLOG[Position_mod_8]][Position_mod_8] & ( rotate_board_right_45 ( chessboard[BISHOP_BLACK + ( xside )],	//TODO ridurre
																	      Position ) | rotate_board_right_45 ( chessboard[QUEEN_BLACK + ( xside )], Position ) ) )
    return 1;

  if ( MOVIMENTO_MASK_CAT[rotate_board_90 ( ALLPIECES, Position )][ROT45ROT_90_MASK[Position]] & ( ( rotate_board_90 ( chessboard[ROOK_BLACK + xside], Position ) | rotate_board_90 ( chessboard[QUEEN_BLACK + xside], Position ) ) ) )
    return 1;
  return 0;
}

#ifdef PERFT_MODE
int
inCheck ( const int da, const int a, const int tipo, const int pezzoda, const int pezzoa, const int SIDE, int promotion_piece ) {
  u64 da1, a1 = 0;
  int result = 0;
  if ( tipo == STANDARD ) {
#ifdef DEBUG_MODE
    assert ( pezzoda != SQUARE_FREE );
    if ( pezzoa == KING_BLACK || pezzoa == KING_WHITE ) {
      print (  );
    }
    assert ( pezzoa != KING_BLACK );
    assert ( pezzoa != KING_WHITE );
#endif
    da1 = chessboard[pezzoda];
    if ( pezzoa != SQUARE_FREE ) {
      a1 = chessboard[pezzoa];
      chessboard[pezzoa] &= NOTTABLOG[a];
    };
    chessboard[pezzoda] &= NOTTABLOG[da];
    chessboard[pezzoda] |= TABLOG[a];
#ifdef DEBUG_MODE
    assert ( chessboard[KING_BLACK] );
    assert ( chessboard[KING_WHITE] );
#endif
    result = attack_square ( SIDE, BitScanForward ( chessboard[KING_BLACK + SIDE] ) );

    chessboard[pezzoda] = da1;
    if ( pezzoa != SQUARE_FREE )
      chessboard[pezzoa] = a1;
  }
  else if ( tipo == CASTLE ) {
    make_castle ( da, a );	//a=SIDE
    result = attack_square ( a, BitScanForward ( chessboard[KING_BLACK + a] ) );

    un_make_castle ( da, a );
  }
  else if ( tipo == PROMOTION ) {
    u64 a1 = chessboard[pezzoa];
    u64 da1 = chessboard[pezzoda];
    u64 p1 = chessboard[promotion_piece];
    chessboard[pezzoda] &= NOTTABLOG[da];
    chessboard[pezzoa] &= NOTTABLOG[a];
    chessboard[promotion_piece] = chessboard[promotion_piece] | TABLOG[a];
    result = attack_square ( SIDE, BitScanForward ( chessboard[KING_BLACK + SIDE] ) );
    chessboard[pezzoa] = a1;
    chessboard[pezzoda] = da1;
    chessboard[promotion_piece] = p1;

  }
  else if ( tipo == ENPASSANT ) {
    u64 a1 = chessboard[SIDE ^ 1];
    u64 da1 = chessboard[SIDE];
    chessboard[SIDE] &= NOTTABLOG[da];
    chessboard[SIDE] |= TABLOG[a];
    if ( SIDE )
      chessboard[SIDE ^ 1] &= NOTTABLOG[a - 8];
    else
      chessboard[SIDE ^ 1] &= NOTTABLOG[a + 8];
    result = attack_square ( SIDE, BitScanForward ( chessboard[KING_BLACK + SIDE] ) );

    chessboard[pezzoa] = a1;
    chessboard[pezzoda] = da1;
  }
#ifdef DEBUG_MODE
  else
    assert ( 0 );
#endif
  return result;
};

#endif

void
myassert ( const void *a, const char *b ) {
  if ( !a ) {
    printf ( b );
    fflush ( stdout );
    exit ( 1 );
  }
}

char *
decodeBoardinv ( const int a, const int side ) {
  if ( a == 7 )
    return "a1";
  if ( a == 15 )
    return "a2";
  if ( a == 23 )
    return "a3";
  if ( a == 31 )
    return "a4";
  if ( a == 39 )
    return "a5";
  if ( a == 47 )
    return "a6";
  if ( a == 55 )
    return "a7";
  if ( a == 63 )
    return "a8";
  if ( a == 6 )
    return "b1";
  if ( a == 14 )
    return "b2";
  if ( a == 22 )
    return "b3";
  if ( a == 30 )
    return "b4";
  if ( a == 38 )
    return "b5";
  if ( a == 46 )
    return "b6";
  if ( a == 54 )
    return "b7";
  if ( a == 62 )
    return "b8";
  if ( a == 5 )
    return "c1";
  if ( a == 13 )
    return "c2";
  if ( a == 21 )
    return "c3";
  if ( a == 29 )
    return "c4";
  if ( a == 37 )
    return "c5";
  if ( a == 45 )
    return "c6";
  if ( a == 53 )
    return "c7";
  if ( a == 61 )
    return "c8";
  if ( a == 4 )
    return "d1";
  if ( a == 12 )
    return "d2";
  if ( a == 20 )
    return "d3";
  if ( a == 28 )
    return "d4";
  if ( a == 36 )
    return "d5";
  if ( a == 44 )
    return "d6";
  if ( a == 52 )
    return "d7";
  if ( a == 60 )
    return "d8";
  if ( a == 3 )
    return "e1";
  if ( a == 11 )
    return "e2";
  if ( a == 19 )
    return "e3";
  if ( a == 27 )
    return "e4";
  if ( a == 35 )
    return "e5";
  if ( a == 43 )
    return "e6";
  if ( a == 51 )
    return "e7";
  if ( a == 59 )
    return "e8";
  if ( a == 2 )
    return "f1";
  if ( a == 10 )
    return "f2";
  if ( a == 18 )
    return "f3";
  if ( a == 26 )
    return "f4";
  if ( a == 34 )
    return "f5";
  if ( a == 42 )
    return "f6";
  if ( a == 50 )
    return "f7";
  if ( a == 58 )
    return "f8";
  if ( a == 1 )
    return "g1";
  if ( a == 9 )
    return "g2";
  if ( a == 17 )
    return "g3";
  if ( a == 25 )
    return "g4";
  if ( a == 33 )
    return "g5";
  if ( a == 41 )
    return "g6";
  if ( a == 49 )
    return "g7";
  if ( a == 57 )
    return "g8";
  if ( a == 0 )
    return "h1";
  if ( a == 8 )
    return "h2";
  if ( a == 16 )
    return "h3";
  if ( a == 24 )
    return "h4";
  if ( a == 32 )
    return "h5";
  if ( a == 40 )
    return "h6";
  if ( a == 48 )
    return "h7";
  if ( a == 56 )
    return "h8";
  if ( a == KINGSIDE && side )
    return "e1g1";
  if ( a == KINGSIDE && !side )
    return "e8g8";
  if ( a == QUEENSIDE && side )
    return "e1c1";
  if ( a == QUEENSIDE && !side )
    return "e8c8";

#ifdef DEBUG_MODE
  printf ( "\n|%d|", a );
  assert ( 0 );
#endif
  return "";
}

#ifndef PERFT_MODE
void
update_pv ( LINE * pline, const LINE * line, const Tmove * mossa, const int depth ) {
  memcpy ( &( pline->argmove[0] ), mossa, sizeof ( Tmove ) );
  memcpy ( pline->argmove + 1, line->argmove, line->cmove * sizeof ( Tmove ) );
  pline->cmove = line->cmove + 1;
  // questa mossa ha causato un taglio, quindi si incrementa il valore
  //di history cosi viene ordinata in alto la prossima volta che la
  //cerchiamo
  HistoryHeuristic[mossa->from][mossa->to] += depth;
  KillerHeuristic[depth][mossa->from][mossa->to] = ( int ) TABLOG[depth];
}

int
fileLung ( char *FileName ) {
  struct stat file;
  if ( !stat ( FileName, &file ) )
    return file.st_size;
  //printf ("\nerrore get_filesize %s", FileName);fflush (stdout);
  return 0;
}

int
wc ( const char *a ) {
  FILE *stream;
  int i = 0;
  char line[1000];
  stream = fopen ( a, "r+b" );
#ifdef DEBUG_MODE
  assert ( stream );
#endif
  while ( fgets ( line, 1000, stream ) != NULL )
    i++;
  fclose ( stream );
  return i;
}

char
decodeBoard ( char *a ) {
  if ( !strcmp ( a, "a1" ) )
    return 7;
  if ( !strcmp ( a, "a2" ) )
    return 15;
  if ( !strcmp ( a, "a3" ) )
    return 23;
  if ( !strcmp ( a, "a4" ) )
    return 31;
  if ( !strcmp ( a, "a5" ) )
    return 39;
  if ( !strcmp ( a, "a6" ) )
    return 47;
  if ( !strcmp ( a, "a7" ) )
    return 55;
  if ( !strcmp ( a, "a8" ) )
    return 63;
  if ( !strcmp ( a, "b1" ) )
    return 6;;
  if ( !strcmp ( a, "b2" ) )
    return 14;
  if ( !strcmp ( a, "b3" ) )
    return 22;
  if ( !strcmp ( a, "b4" ) )
    return 30;
  if ( !strcmp ( a, "b5" ) )
    return 38;
  if ( !strcmp ( a, "b6" ) )
    return 46;
  if ( !strcmp ( a, "b7" ) )
    return 54;
  if ( !strcmp ( a, "b8" ) )
    return 62;
  if ( !strcmp ( a, "c1" ) )
    return 5;;
  if ( !strcmp ( a, "c2" ) )
    return 13;
  if ( !strcmp ( a, "c3" ) )
    return 21;
  if ( !strcmp ( a, "c4" ) )
    return 29;
  if ( !strcmp ( a, "c5" ) )
    return 37;
  if ( !strcmp ( a, "c6" ) )
    return 45;
  if ( !strcmp ( a, "c7" ) )
    return 53;
  if ( !strcmp ( a, "c8" ) )
    return 61;
  if ( !strcmp ( a, "d1" ) )
    return 4;;
  if ( !strcmp ( a, "d2" ) )
    return 12;
  if ( !strcmp ( a, "d3" ) )
    return 20;
  if ( !strcmp ( a, "d4" ) )
    return 28;
  if ( !strcmp ( a, "d5" ) )
    return 36;
  if ( !strcmp ( a, "d6" ) )
    return 44;
  if ( !strcmp ( a, "d7" ) )
    return 52;
  if ( !strcmp ( a, "d8" ) )
    return 60;
  if ( !strcmp ( a, "e1" ) )
    return 3;;
  if ( !strcmp ( a, "e2" ) )
    return 11;
  if ( !strcmp ( a, "e3" ) )
    return 19;
  if ( !strcmp ( a, "e4" ) )
    return 27;
  if ( !strcmp ( a, "e5" ) )
    return 35;
  if ( !strcmp ( a, "e6" ) )
    return 43;
  if ( !strcmp ( a, "e7" ) )
    return 51;
  if ( !strcmp ( a, "e8" ) )
    return 59;
  if ( !strcmp ( a, "f1" ) )
    return 2;;
  if ( !strcmp ( a, "f2" ) )
    return 10;
  if ( !strcmp ( a, "f3" ) )
    return 18;
  if ( !strcmp ( a, "f4" ) )
    return 26;
  if ( !strcmp ( a, "f5" ) )
    return 34;
  if ( !strcmp ( a, "f6" ) )
    return 42;
  if ( !strcmp ( a, "f7" ) )
    return 50;
  if ( !strcmp ( a, "f8" ) )
    return 58;
  if ( !strcmp ( a, "g1" ) )
    return 1;;
  if ( !strcmp ( a, "g2" ) )
    return 9;
  if ( !strcmp ( a, "g3" ) )
    return 17;
  if ( !strcmp ( a, "g4" ) )
    return 25;
  if ( !strcmp ( a, "g5" ) )
    return 33;
  if ( !strcmp ( a, "g6" ) )
    return 41;
  if ( !strcmp ( a, "g7" ) )
    return 49;
  if ( !strcmp ( a, "g8" ) )
    return 57;
  if ( !strcmp ( a, "h1" ) )
    return 0;;
  if ( !strcmp ( a, "h2" ) )
    return 8;
  if ( !strcmp ( a, "h3" ) )
    return 16;;
  if ( !strcmp ( a, "h4" ) )
    return 24;
  if ( !strcmp ( a, "h5" ) )
    return 32;
  if ( !strcmp ( a, "h6" ) )
    return 40;
  if ( !strcmp ( a, "h7" ) )
    return 48;
  if ( !strcmp ( a, "h8" ) )
    return 56;
  printf ( "\n||%s||", a );
  fflush ( stdout );
#ifdef DEBUG_MODE
  assert ( 0 );
#endif
  return -1;
}

#endif
char
getFenInv ( const char a ) {

  if ( a == 'r' )
    return 2;
  if ( a == 'n' )
    return 6;
  if ( a == 'b' )
    return 4;
  if ( a == 'q' )
    return 10;
  if ( a == 'k' )
    return 8;
  if ( a == 'p' )
    return 0;
  if ( a == 'P' )
    return 1;
  if ( a == 'R' )
    return 3;
  if ( a == 'B' )
    return 5;
  if ( a == 'Q' )
    return 11;
  if ( a == 'K' )
    return 9;
  if ( a == 'N' )
    return 7;
  if ( a == '-' )
    return 12;
  return -1;

}

char
getFen ( const int a ) {
  char result;
  switch ( a ) {
  case 2:
    result = 'r';
    break;
  case 6:
    result = 'n';
    break;
  case 4:
    result = 'b';
    break;
  case 10:
    result = 'q';
    break;
  case 8:
    result = 'k';
    break;
  case 0:
    result = 'p';
    break;
  case 1:
    result = 'P';
    break;
  case 3:
    result = 'R';
    break;
  case 5:
    result = 'B';
    break;
  case 11:
    result = 'Q';
    break;
  case 9:
    result = 'K';
    break;
  case 7:
    result = 'N';
    break;
  case 12:
    result = '-';
    break;
  default:
    result = '?';
  };
  return result;
}



void
BoardToFEN ( char *FEN ) {
  int x, y, l = 0, i = 0, sq;
  char row[8];
  int q;
  strcpy ( FEN, "" );
  for ( y = 0; y < 8; y++ ) {
    i = l = 0;
    strcpy ( row, "" );
    for ( x = 0; x < 8; x++ ) {
      sq = ( y * 8 ) + x;
      q = get_piece_at ( 0, TABLOG[63 - sq] );
      if ( q == SQUARE_FREE )
	q = get_piece_at ( 1, TABLOG[63 - sq] );
      if ( q == SQUARE_FREE )
	l++;
      else {
	if ( l > 0 ) {
	  row[i] = ( char ) ( l + 48 );
	  i++;
	}
	l = 0;

	row[i] = getFen ( q );

	i++;
      }
    }
    if ( l > 0 ) {
      row[i] = ( char ) ( l + 48 );
      i++;
    }
    strncat ( FEN, row, i );
    if ( y < 7 )
      strcat ( FEN, "/" );
  }

  if ( black_move )
    strcat ( FEN, " b " );
  else
    strcat ( FEN, " w " );
  if ( !CASTLE_NOT_POSSIBLE_KINGSIDE[WHITE] )
    strcat ( FEN, "K" );
  if ( !CASTLE_NOT_POSSIBLE_QUEENSIDE[WHITE] )
    strcat ( FEN, "Q" );
  if ( !CASTLE_NOT_POSSIBLE_KINGSIDE[BLACK] )
    strcat ( FEN, "k" );
  if ( !CASTLE_NOT_POSSIBLE_QUEENSIDE[BLACK] )
    strcat ( FEN, "q" );

  /* if (B->castle&1) strcat(FEN,"K");
     if (B->castle&2) strcat(FEN,"Q");
     if (B->castle&4) strcat(FEN,"k");
     if (B->castle&8) strcat(FEN,"q");
     if (B->castle==0) strcat(FEN,"-");

     if (B->ep==-1) strcat(FEN," -");
     else {
     strcat(FEN," ");
     ch[0] = (char)((B->ep)&7) + 97;
     ch[1] = (char)(56 - ((B->ep>>3)&7));
     ch[2]=0;
     strcat(FEN,ch);
     } */


}

#ifdef DEBUG_MODE
void
controlloRipetizioni ( Tmove * myarray, int right ) {

  for ( int t = 0; t < right; t++ )
    for ( int j = t + 1; j < right; j++ ) {
      if ( myarray[t].da == myarray[j].da && myarray[t].a == myarray[j].a && myarray[t].promotion_piece == myarray[j].promotion_piece ) {
	print (  );
	printf ( "\n %d %d", myarray[j].da, myarray[t].a );
	assert ( 0 );
      }
    }


}
#endif
void
debug ( char *msg ) {

  outf = fopen ( debugfile, "a+" );
  fwrite ( msg, 1, strlen ( msg ), outf );
  fwrite ( "\n", 1, 1, outf );
  fclose ( outf );

};

void
print (  ) {
  //if ( xboard )
  //return;
  int t;
  char x;
  char FEN[1000];
  printf ( "\n     a   b   c   d   e   f   g   h" );

  for ( t = 0; t <= 63; t++ ) {

    if ( !( t % 8 ) )
      printf ( "\n   ---------------------------------\n" );

    x = getFen ( get_piece_at ( 1, TABLOG[63 - t] ) );
    if ( x == '-' )
      x = getFen ( get_piece_at ( 0, TABLOG[63 - t] ) );
    if ( x == '-' )
      x = ' ';
    switch ( t ) {
    case 0:
      printf ( " 8 | " );
      break;
    case 8:
      printf ( " 7 | " );
      break;
    case 16:
      printf ( " 6 | " );
      break;
    case 24:
      printf ( " 5 | " );
      break;
    case 32:
      printf ( " 4 | " );
      break;
    case 40:
      printf ( " 3 | " );
      break;
    case 48:
      printf ( " 2 | " );
      break;
    case 56:
      printf ( " 1 | " );
      break;
    }
    if ( x != ' ' )
      printf ( "%c", x );
    else if ( t == 0 || t == 2 || t == 4 || t == 6 || t == 9 || t == 11 || t == 13 || t == 15 || t == 16 || t == 18 || t == 20 || t == 22 || t == 25 || t == 27 || t == 29 || t == 31 || t == 32 || t == 34 || t == 36 || t == 38 || t == 41 || t == 43 || t == 45 || t == 47 || t == 48 || t == 50 || t == 52 || t == 54 || t == 57 || t == 59 || t == 61 || t == 63 )
      printf ( " " );
    else
      printf ( "." );
    printf ( " | " );



  };
  printf ( "\n" );

  printf ( "   ---------------------------------\n" );
  printf ( "     a   b   c   d   e   f   g   h\n\n" );


  BoardToFEN ( FEN );
  printf ( "\n%s", FEN );
#ifdef TEST_MODE
  printf ( "(%s)", test_ris );
#endif
  printf ( "\n" );
  fflush ( stdout );

}



int
diff_time ( struct timeb a, struct timeb b ) {
  return ( int ) ( 1000 * ( a.time - b.time ) + ( a.millitm - b.millitm ) );
}

void
free_fen_stack (  ) {
  for ( int i = 0; i < FEN_STACK.count; i++ )
    free ( FEN_STACK.fen[i] );
  FEN_STACK.count = 0;
}

void
push_fen (  ) {
  char FEN[1000];
  BoardToFEN ( FEN );
  FEN_STACK.fen[FEN_STACK.count] = ( char * ) malloc ( sizeof ( FEN ) + 1 );
  strcpy ( FEN_STACK.fen[FEN_STACK.count], FEN );
  FEN_STACK.count++;
}

void
pop_fen (  ) {
  if ( FEN_STACK.count == 1 )
    return;
  FEN_STACK.count--;
  loadfen ( FEN_STACK.fen[FEN_STACK.count - 1] );
  free ( FEN_STACK.fen[FEN_STACK.count] );
  FEN_STACK.fen[FEN_STACK.count] = NULL;

}

int
loadfen ( char *ss ) {

  int i, ii, t, p;
  char ch;
  char *x;
  int s[64];
  char a[2];
  for ( i = 0; i <= 11; i++ )
    chessboard[i] = 0;

  i = 0;
  ii = 0;
  if ( strlen ( ss ) )
    do {
      ch = ss[ii];
      //if(!ch)break;
      switch ( ch ) {
      case 'r':
	s[i++] = 2;
	break;
      case 'n':
	s[i++] = 6;
	break;
      case 'b':
	s[i++] = 4;
	break;
      case 'q':
	s[i++] = 10;
	break;
      case 'k':
	s[i++] = 8;
	break;
      case 'p':
	s[i++] = 0;
	break;
      case 'P':
	s[i++] = 1;
	break;
      case 'R':
	s[i++] = 3;
	break;
      case 'B':
	s[i++] = 5;
	break;
      case 'Q':
	s[i++] = 11;
	break;
      case 'K':
	s[i++] = 9;
	break;
      case 'N':
	s[i++] = 7;
	break;
      case '/':;
	break;
      case ' ':;
	break;
      case '-':;
	break;
      case 'w':;
	break;
      case 10:;
	break;
      case 13:;
	break;
      default:
      {
	if ( ch > 47 && ch < 58 ) {
	  a[0] = ch;
	  a[1] = 0;
	  for ( t = 1; t <= atoi ( a ); t++ )
	    s[i++] = SQUARE_FREE;
	}
	else {
	  printf ( "Bad FEN position format.|%c|\n%s", ch, ss );
	  printf ( "\nerror" );
	  return 0;
	};
      };

      }
      ii++;
    }
    while ( i < 64 );
  for ( i = 0; i <= 63; i++ ) {
    p = s[63 - i];
    if ( p != SQUARE_FREE )
      chessboard[p] |= TABLOG[i];
  };
  if ( ( x = strstr ( ss, " b " ) ) )
    black_move = 1;
  else if ( ( x = strstr ( ss, " w " ) ) )
    black_move = 0;
  else
    printf ( "Bad FEN position format.\n%s", ss );
  x += 3;
#ifdef TEST_MODE
  /*const char *ris1 = strstr (ss, ";");
     const char *ris2 = strstr (ss, " bm ");
     memset (test_ris, 0, sizeof (test_ris));
     memcpy (test_ris, ris2 + 4, ris1 - ris2 - 4);
   */
  char *ris1 = strstr ( ss, " id WAC" );
  if ( !( ris1 ) )
    ris1 = strstr ( ss, " id \"WAC" );
  if ( ris1 ) {
    ris1--;
    if ( *( ris1 - 1 ) == '+' )
      ris1--;
    memset ( test_ris, 0, sizeof ( test_ris ) );
    memcpy ( test_ris, ris1 - 2, 2 );
  }
#endif
  CASTLE_DONE[0] = 1;
  CASTLE_DONE[1] = 1;
  CASTLE_NOT_POSSIBLE[0] = 1;
  CASTLE_NOT_POSSIBLE[1] = 1;
  CASTLE_NOT_POSSIBLE_QUEENSIDE[0] = 1;
  CASTLE_NOT_POSSIBLE_QUEENSIDE[1] = 1;
  CASTLE_NOT_POSSIBLE_KINGSIDE[0] = 1;
  CASTLE_NOT_POSSIBLE_KINGSIDE[1] = 1;
  ENP_POSSIBILE = -1;
  i = 0;
  while ( ( unsigned ) i < strlen ( x ) /*[i] != ' ' */  ) {
    if ( x[i] == 'K' ) {
      CASTLE_DONE[1] = 0;
      CASTLE_NOT_POSSIBLE[1] = 0;
      CASTLE_NOT_POSSIBLE_KINGSIDE[1] = 0;
    }
    else if ( x[i] == 'k' ) {
      CASTLE_DONE[0] = 0;
      CASTLE_NOT_POSSIBLE[0] = 0;
      CASTLE_NOT_POSSIBLE_KINGSIDE[0] = 0;
    }
    else if ( x[i] == 'Q' ) {
      CASTLE_DONE[1] = 0;
      CASTLE_NOT_POSSIBLE[1] = 0;
      CASTLE_NOT_POSSIBLE_QUEENSIDE[1] = 0;
    }
    else if ( x[i] == 'q' ) {
      CASTLE_DONE[0] = 0;
      CASTLE_NOT_POSSIBLE[0] = 0;
      CASTLE_NOT_POSSIBLE_QUEENSIDE[0] = 0;
    }
    i++;
  };
  /*    if (black_move )
     {
     SIDE = BLACK;
     XSIDE = WHITE;
     }
     else
     {
     XSIDE = BLACK;
     SIDE = WHITE;
     } */

  re_amico[black_move] = BitScanForward ( chessboard[KING_BLACK + black_move] );

  re_amico[black_move ^ 1] = BitScanForward ( chessboard[KING_BLACK + ( black_move ^ 1 )] );

  init (  );
  return 1;
}

uchar
rotate_board_left_45 ( const u64 ss, const int pos ) {

  if ( !ss )
    return 0;
  register uchar result = 0;
#ifdef HAS_64BITS
#include "rotate_board_left_45_64bit.txt"
#else
  unsigned s1 = ( unsigned ) ss;
  unsigned s2 = shift32 ( ss );
#include "rotate_board_left_45_32bit.txt"
#endif
  return result;
}

uchar
rotate_board_right_45 ( const u64 ss, const int pos ) {

  if ( !ss )
    return 0;
  register uchar result = 0;
#ifdef HAS_64BITS
#include "rotate_board_right_45_64bit.txt"
#else
  unsigned s1 = ( unsigned ) ss;
  unsigned s2 = shift32 ( ss );

#include "rotate_board_right_45_32bit.txt"
#endif
  return result;
}



uchar
rotate_board_90 ( u64 ss, const int pos ) {

  if ( !ss )
    return 0;
  register uchar result = 0;
#ifdef HAS_64BITS
#include "rotate_board_90_64bit.txt"
#else
  unsigned s1 = ( unsigned ) ss;
  unsigned s2 = shift32 ( ss );

#include "rotate_board_90_32bit.txt"
#endif

  return result;

}

#ifndef PERFT_MODE
u64
get_u64_column ( char a ) {
  switch ( a ) {
  case 'a':
    return FILE_7;
    break;
  case 'b':
    return FILE_6;
    break;
  case 'c':
    return FILE_5;
    break;
  case 'd':
    return FILE_4;
    break;
  case 'e':
    return FILE_3;
    break;
  case 'f':
    return FILE_2;
    break;
  case 'g':
    return FILE_1;
    break;
  case 'h':
    return FILE_0;
    break;
  default:
    return 0;
    break;
  }
}



int
is_number ( char c ) {
  if ( c >= 48 && c <= 57 )
    return 1;
  return 0;
}

int
fen2pos ( char *fen, int *from, int *to, int SIDE, u64 key ) {

#ifdef HASH_MODE
  use_book = 0;
#endif
  int score;
  eval ( SIDE
#ifdef FP_MODE
	 , -_INFINITE, _INFINITE
#endif
	 , key );

  int o, try_locked;
  char dummy[3];
  u64 column = 0xFFFFFFFFFFFFFFFFULL;
  char pezzo_da = -1;
  list_id = 0;
  //castle
  if ( fen[0] == 'O' ) {
    if ( !SIDE ) {
      *from = 59;
      if ( !strcmp ( fen, "O-O" ) )
	*to = 57;
      else
	*to = 62;
    }
    else {
      *from = 3;
      if ( !strcmp ( fen, "O-O" ) )
	*to = 1;
      else
	*to = 6;
    }
    if ( !strcmp ( fen, "O-O" ) )
      pushmove ( CASTLE, KINGSIDE, SIDE, -1 );
    else
      pushmove ( CASTLE, QUEENSIDE, SIDE, -1 );
    Tmove *mossa = &gen_list[0][1];
    makemove ( mossa );
    score = eval ( SIDE
#ifdef FP_MODE
		   , -_INFINITE, _INFINITE
#endif
		   , key );
    return score;
  }
  //promotion
  if ( strstr ( fen, "=" ) ) {
    dummy[0] = fen[0];
    dummy[1] = fen[1];
    dummy[2] = 0;
    *to = decodeBoard ( dummy );
    if ( SIDE )
      *from = *to - 8;
    else
      *from = *to + 8;

    char p = getFenInv ( strstr ( fen, "=" )[1] );
    pushmove ( PROMOTION, *from, *to, SIDE, p );
    Tmove *mossa = &gen_list[0][1];
    makemove ( mossa );
    score = eval ( SIDE
#ifdef FP_MODE
		   , -_INFINITE, _INFINITE
#endif
		   , key );
    return score;
  }

  if ( strlen ( fen ) >= 2 && tolower ( fen[0] ) == fen[0] && !is_number ( fen[0] ) )
    column = get_u64_column ( fen[0] );
  else if ( strlen ( fen ) > 3 && fen[1] != 'x' && fen[0] != 'x' && tolower ( fen[1] ) == fen[1] && !is_number ( fen[1] ) )
    column = get_u64_column ( fen[1] );

  if ( strlen ( fen ) >= 2 && is_number ( fen[0] ) )
    column = RANK[fen[0] - 48 - 1];
  else if ( strlen ( fen ) > 3 && fen[1] != 'x' && fen[0] != 'x' && is_number ( fen[1] ) )
    column = RANK[fen[1] - 48 - 1];
  if ( toupper ( fen[0] ) == fen[0] ) {
    if ( !SIDE )
      pezzo_da = getFenInv ( ( char ) tolower ( fen[0] ) );
    else
      pezzo_da = getFenInv ( fen[0] );
  }
  *to = decodeBoard ( fen + strlen ( fen ) - 2 );
  if ( ( *to ) == -1 )
    myassert ( 0, "e" );
  u64 attaccanti2, attaccanti = evalNode.attaccanti[*to];
  attaccanti &= get_pieces ( SIDE ) | chessboard[SIDE];
  int c = BitCount ( attaccanti );
  if ( !c ) {
    printf ( "\nerror en passant?" );
    *from = -1;
    return 0;
    //myassert (0, "errord");
  }
  if ( c == 1 )
    *from = BitScanForward ( attaccanti );
  else {
    *from = -1;
    /*  if (strlen (fen) == 2)
       {
       if (!SIDE)
       pezzo_da = 0;
       else
       pezzo_da = 1;
       }
       else
       {
       /* if (!SIDE)
       o = tolower (fen[0]);
       else
       o = fen[0];
       pezzo_da = getFenInv ((char) o);
       #ifdef DEBUG_MODE
       assert(pezzo_da!=-1);
       #endif
       } */
    try_locked = 0;
    attaccanti2 = attaccanti;
    while ( attaccanti ) {
      o = BitScanForward ( attaccanti );
      if ( ( pezzo_da == -1 || pezzo_da != -1 && chessboard[pezzo_da] & TABLOG[o] & column ) && TABLOG[o] & column ) {
	if ( *from != -1 ) {
	  try_locked = 1;
	  *from = -1;
	  attaccanti = attaccanti2;
	  break;
	};
	*from = o;
      };
      attaccanti &= NOTTABLOG[o];
    };
    if ( try_locked )
      while ( attaccanti ) {
	o = BitScanForward ( attaccanti );
	if ( ( pezzo_da == -1 || pezzo_da != -1 && chessboard[pezzo_da] & TABLOG[o] & column ) && TABLOG[o] & column && !is_locked ( o, get_piece_at ( SIDE, TABLOG[o] ), SIDE ) ) {
	  if ( *from != -1 ) {
	    printf ( "\nambiguous, skip " );
	    break;
	  };
	  *from = o;
	};
	attaccanti &= NOTTABLOG[o];
      };
  }

  pushmove ( STANDARD, *from, *to, SIDE );
  Tmove *mossa = &gen_list[0][1];
  makemove ( mossa );

  score = eval ( SIDE
#ifdef FP_MODE
		 , -_INFINITE, _INFINITE
#endif
		 , key );
  return score;
}

#endif
