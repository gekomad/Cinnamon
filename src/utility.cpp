#include "stdafx.h"
#include <time.h>
#include "maindefine.h"
#include "winboard.h"
#include "gen.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "extern.h"
#include "zobrist.h"
#include "search.h"
#ifdef _MSC_VER
#include <windows.h>
#endif

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
}

#ifndef PERFT_MODE
/*
 int check_repetition() {
 int inc;
 for (int i = 0; i < stack_move1.next-2; i++) {
 inc = 1;
 for (int j = i + 1; j < stack_move1.next; j++) {
 if (stack_move1.board[i] == stack_move1.board[j])
 if ((++inc) > 2) {
 return inc;
 }
 }
 }
 return 0;
 }
 */
int
interior_node_recognition ( int n_pieces_side, int n_pieces_x_side, int side ) {
  if ( 1 == n_pieces_side == n_pieces_x_side )
    return 1;			//KK
  if ( ( n_pieces_side == 1 && n_pieces_x_side == 2 && ( chessboard[BISHOP_BLACK + ( side ^ 1 )] || chessboard[KNIGHT_BLACK + ( side ^ 1 )] ) ) || ( n_pieces_side == 2 && n_pieces_x_side == 1 && ( chessboard[BISHOP_BLACK + ( side )] || chessboard[KNIGHT_BLACK + ( side )] ) ) )
    return 1;			//KNK or KBK
  if ( n_pieces_side == 2 && n_pieces_x_side == 2 && chessboard[BISHOP_BLACK + ( side ^ 1 )] && chessboard[BISHOP_BLACK + ( side )] && COLORS[BITScanForward ( chessboard[BISHOP_BLACK + ( side )] )] == COLORS[BITScanForward ( chessboard[BISHOP_BLACK + ( side ^ 1 )] )] )
    return 1;			//KBKB
  return 0;
}

int
check_draw ( int n_pieces_side, int n_pieces_x_side, int side ) {
  if ( interior_node_recognition ( n_pieces_side, n_pieces_x_side, side ) )
    return 1;
  return 0;			//check_repetition();
}

#endif
int
compare_move ( const void *a, const void *b ) {
  Tmove *arg1 = ( Tmove * ) a;
  Tmove *arg2 = ( Tmove * ) b;
  if ( arg1->score < arg2->score )
    return 1;
  else if ( arg1->score == arg2->score )
    return 0;
  else
    return -1;
}

int
is_locked ( int pos, int pezzo, int side ) {
  ASSERT ( pezzo >= 0 && pezzo < 12 );
  ASSERT ( pos >= 0 && pos < 64 );
  int r = 0;
  chessboard[pezzo] &= NOTTABLOG[pos];
  r = attack_square ( side, Friend_king[side] );
  chessboard[pezzo] |= TABLOG[pos];
  return r;
}

void
init (  ) {
  ENP_POSSIBILE = -1;
  num_moves = num_movesq = mate = 0;

#ifdef FP_MODE
#ifdef DEBUG_MODE
  n_cut_fp = n_cut_razor = 0;
#endif
#endif
  list_id = -1;
  evaluateMobility_mode = LazyEvalCuts = 0;
#ifdef PERFT_MODE
  n_perft = 0;
  listcount_n = 0;
#else
#ifdef DEBUG_MODE
  beta_efficency1 = 0.0;
  n_cut = 0;
  null_move_cut = 0;
#endif
#endif
  null_sem = 0;
#ifdef HASH_MODE
#ifdef DEBUG_MODE
  n_cut_hash = n_record_hash = collisions = 0;
#endif
#endif
}

int
pushmove ( const int tipomove, const int da, const int a, const int SIDE, int promotion_piece ) {
  int return_code = 0;
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );

#ifndef PERFT_MODE
  if ( evaluateMobility_mode ) {
    if ( da >= 0 ) {		//TODO gestire arrocco
      ASSERT ( da >= 0 && a >= 0 );
      EVAL.attacked[da] |= tablog ( a );
      if ( ( TABLOG[a] & chessboard[KING_BLACK + ( SIDE ^ 1 )] ) )
	EVAL.king_attacked[da] = 1;
      EVAL.attackers[a] |= tablog ( da );
      ASSERT ( list_id < MAX_PLY && list_id >= 0 );
      ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
      gen_list[list_id][0].score++;
      return 0;
    }
    return 0;
  }

#endif
  int pezzoa;
  if ( tipomove != ENPASSANT && tipomove != CASTLE ) {
    pezzoa = get_piece_at ( ( change_side ( SIDE ) ), TABLOG[a] );
    if ( ( pezzoa == KING_BLACK ) || ( pezzoa == KING_WHITE ) )
      return 1;
  }
  else if ( tipomove != CASTLE )
    pezzoa = change_side ( SIDE );
#ifdef PERFT_MODE
  int pezzoda = -5;
  if ( tipomove != CASTLE )
    pezzoda = get_piece_at ( SIDE, tablog ( da ) );
  if ( inCheck ( da, a, tipomove, pezzoda, pezzoa, SIDE, promotion_piece ) )
    return 0;

#endif
  Tmove *mos;
  ASSERT ( list_id != -1 );
  ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
  mos = &gen_list[list_id][++gen_list[list_id][0].score];
#ifdef DEBUG_MODE
  memcpy ( mos->board, chessboard, sizeof ( Tchessboard ) );
#endif
  if ( tipomove == STANDARD || tipomove == ENPASSANT || tipomove == PROMOTION ) {
#ifndef PERFT_MODE
    int pezzoda = get_piece_at ( SIDE, tablog ( da ) );
#endif
    mos->type = ( char ) tipomove;
    mos->from = ( char ) da;
    ASSERT ( mos->from >= 0 && mos->from < 64 );
    mos->to = ( char ) a;
    mos->side = ( char ) SIDE;
    mos->promotion_piece = ( char ) promotion_piece;
#ifndef PERFT_MODE
    mos->score = 0;
    mos->score += ( PIECES_VALUE[pezzoa] >= PIECES_VALUE[pezzoda] ) ? ( PIECES_VALUE[pezzoa] - PIECES_VALUE[pezzoda] ) * 2 : PIECES_VALUE[pezzoa];
    mos->score += HistoryHeuristic[da][a];
    mos->score += KillerHeuristic[main_depth][da][a];
    ASSERT ( pezzoda >= 0 && pezzoda < 12 && a >= 0 && a < 64 && da >= 0 && da < 64 );
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
  ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
  return 0;
}

void
un_perform_castle ( const int da, const int SIDE ) {
  if ( SIDE == WHITE ) {
    if ( da == KINGSIDE ) {
      ASSERT ( get_piece_at ( SIDE, TABLOG_1 ) == KING_WHITE );
      ASSERT ( get_piece_at ( SIDE, TABLOG_0 ) == 12 );
      ASSERT ( get_piece_at ( SIDE, TABLOG_3 ) == 12 );
      ASSERT ( get_piece_at ( SIDE, TABLOG_2 ) == ROOK_WHITE );
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
  check_side ( side );
  ASSERT ( Position != -1 );
  int Position_Position_mod_8, xside;
  char Position_mod_8;
  u64 ALLPIECES;

  //ASSERT (BitCount (chessboard[KING_BLACK + side]) < 2);

  xside = change_side ( side );
  if ( KNIGHT_MASK[Position] & Chessboard ( KNIGHT_BLACK + xside ) )
    return 1;
  if ( NEAR_MASK[Position] & chessboard[KING_BLACK + xside] )
    return 1;
  //enpassant
  if ( PAWN_CAPTURE_MASK[side][Position] & chessboard[PAWN_BLACK + xside] )
    return 1;

  ASSERT ( Position >= 0 && Position < 64 );

  if ( !( VERT_ORIZZ[Position] & ( chessboard[ROOK_BLACK + xside] | Chessboard ( QUEEN_BLACK + xside ) ) | LEFT_RIGHT[Position] & ( Chessboard ( QUEEN_BLACK + xside ) | chessboard[BISHOP_BLACK + xside] ) ) ) {
    return 0;
  }
  ALLPIECES = square_all_bit_occupied (  ) | TABLOG[Position];
  Position_mod_8 = ROT45[Position];

  ASSERT ( Position >= 0 && Position < 64 );

  Position_Position_mod_8 = pos_posMod8[Position];
  if ( MASK_CAPT_MOV[( uchar ) ( ( ( ALLPIECES >> Position_Position_mod_8 ) ) )][Position_mod_8] & ( ( chessboard[ROOK_BLACK + xside] >> Position_Position_mod_8 ) & 255 | ( chessboard[QUEEN_BLACK + xside] >> Position_Position_mod_8 ) & 255 ) ) {
    return 1;
  }
  //left
  if ( MASK_CAPT_MOV[rotate_board_left_45 ( ALLPIECES, Position )][Position_mod_8] & ( rotate_board_left_45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotate_board_left_45 ( chessboard[QUEEN_BLACK + xside], Position ) ) )
    return 1;
  /*right \ */
  if ( MASK_CAPT_MOV[rotate_board_right_45 ( ALLPIECES, Position ) | TABLOG[Position_mod_8]][Position_mod_8] & ( rotate_board_right_45 ( chessboard[BISHOP_BLACK + ( xside )], Position ) | rotate_board_right_45 ( chessboard[QUEEN_BLACK + ( xside )], Position ) ) )
    return 1;

  ASSERT ( Position >= 0 && Position < 64 );

  if ( MASK_CAPT_MOV[rotate_board_90 ( ALLPIECES & VERTICAL[Position] )][ROT45ROT_90_MASK[Position]] & ( ( rotate_board_90 ( chessboard[ROOK_BLACK + xside] & VERTICAL[Position] ) | rotate_board_90 ( chessboard[QUEEN_BLACK + xside] & VERTICAL[Position] ) ) ) )
    return 1;
  return 0;
}

#ifdef PERFT_MODE
int
inCheck ( const int da, const int a, const int tipo, const int pezzoda, const int pezzoa, const int SIDE, int promotion_piece ) {
  u64 da1, a1 = 0;
  int result = 0;
  if ( tipo == STANDARD ) {

    ASSERT ( pezzoda != SQUARE_FREE );
    ASSERT ( pezzoa != KING_BLACK );
    ASSERT ( pezzoa != KING_WHITE );

    da1 = chessboard[pezzoda];
    if ( pezzoa != SQUARE_FREE ) {
      a1 = Chessboard ( pezzoa );
      chessboard[pezzoa] &= NOTTABLOG[a];
    };
    chessboard[pezzoda] &= NOTTABLOG[da];
    chessboard[pezzoda] |= tablog ( a );

    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );

    result = attack_square ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    chessboard[pezzoda] = da1;
    if ( pezzoa != SQUARE_FREE )
      chessboard[pezzoa] = a1;
  }
  else if ( tipo == CASTLE ) {
    perform_castle ( da, a );	//a=SIDE
    result = attack_square ( a, BITScanForward ( chessboard[KING_BLACK + a] ) );
    un_perform_castle ( da, a );
  }
  else if ( tipo == PROMOTION ) {
    u64 a1;
    if ( pezzoa != SQUARE_FREE )
      a1 = Chessboard ( pezzoa );
    u64 da1 = Chessboard ( pezzoda );
    u64 p1 = Chessboard ( promotion_piece );
    chessboard[pezzoda] &= NOTTABLOG[da];
    if ( pezzoa != SQUARE_FREE )
      chessboard[pezzoa] &= NOTTABLOG[a];
    chessboard[promotion_piece] = Chessboard ( promotion_piece ) | TABLOG[a];
    result = attack_square ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    if ( pezzoa != SQUARE_FREE )
      chessboard[pezzoa] = a1;
    chessboard[pezzoda] = da1;
    chessboard[promotion_piece] = p1;
  }
  else if ( tipo == ENPASSANT ) {
    u64 a1 = Chessboard ( change_side ( SIDE ) );
    u64 da1 = Chessboard ( SIDE );
    chessboard[SIDE] &= NOTTABLOG[da];
    chessboard[SIDE] |= tablog ( a );
    if ( SIDE )
      chessboard[change_side ( SIDE )] &= NOTTABLOG[a - 8];
    else
      chessboard[change_side ( SIDE )] &= NOTTABLOG[a + 8];
    result = attack_square ( SIDE, BITScanForward ( chessboard[KING_BLACK + SIDE] ) );
    chessboard[change_side ( SIDE )] = a1;
    chessboard[SIDE] = da1;
  }
#ifdef DEBUG_MODE
  else
    ASSERT ( 0 );
#endif
  return result;
}
#endif

void
myassert ( int a, const char *b ) {
  if ( !a ) {
    printf ( "%s", b );
    printf ( "\n" );
    fflush ( stdout );
    exit ( 1 );
  }
}

const char *
decodeBoardinv ( const int a, const int side ) {
  if ( a >= 0 && a < 64 )
    return BOARD[a];
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
  ASSERT ( 0 );
#endif
  return "";

}

#ifndef PERFT_MODE
void
update_pv ( LINE * pline, const LINE * line, const Tmove * mossa, const int depth ) {
#ifdef DEBUG_MODE
  ASSERT ( line->cmove < MAX_PLY - 1 );
#endif
  memcpy ( &( pline->argmove[0] ), mossa, sizeof ( Tmove ) );
  memcpy ( pline->argmove + 1, line->argmove, line->cmove * sizeof ( Tmove ) );
  assert ( line->cmove >= 0 );
  pline->cmove = line->cmove + 1;
  // questa mossa ha causato un taglio, quindi si incrementa il valore
  //di history cosi viene ordinata in alto la prossima volta che la
  //cerchiamo
  /*if (mossa->from >= 0) {
     HistoryHeuristic[mossa->from][mossa->to] += (int) TABLOG[depth];
     KillerHeuristic[depth][mossa->from][mossa->to] = (int) TABLOG[depth];
     } */
}

int
fileLung ( char *FileName ) {
  struct stat file;
  if ( !stat ( FileName, &file ) )
    return file.st_size;
  //printf ("\nerrore get_filesize %s", FileName);fflush (stdout);
  return 0;
}

char
decodeBoard ( char *a ) {
  for ( int i = 0; i < 64; i++ ) {
    if ( !strcmp ( a, BOARD[i] ) )
      return i;
  }
  printf ( "\n||%s||", a );
  fflush ( stdout );
  ASSERT ( 0 );
  return -1;
}

#endif

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

  /* if (B->castle&1) strcat(FEN,"K"); TODO
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
print ( Tchessboard s ) {
  Tchessboard bk;
  memcpy ( bk, chessboard, sizeof ( Tchessboard ) );
  memcpy ( chessboard, s, sizeof ( Tchessboard ) );
  print ( "\n================== STACK BOARD ==================" );
  memcpy ( chessboard, bk, sizeof ( Tchessboard ) );
}
#endif
void
print ( char *s ) {
  printf ( "%s", s );
  print (  );
}

void
print (  ) {
  if ( xboard )
    return;
  int t;
  char x;
  char FEN[1000];
  printf ( "\n.....a   b   c   d   e   f   g   h" );
  for ( t = 0; t <= 63; t++ ) {
    if ( !( t % 8 ) ) {
      printf ( "\n...---------------------------------\n" );
    };
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
  printf ( "...---------------------------------\n" );
  printf ( ".....a   b   c   d   e   f   g   h\n\n" );
  BoardToFEN ( FEN );
  printf ( "\n%s", FEN );
#ifdef TEST_MODE
  printf ( "(%s)", test_ris );

#endif
  printf ( "\n" );
  fflush ( stdout );
}

#ifdef TEST_MODE
int
extract_test_result ( char *ris1 ) {

  int i, result = 1;
  char *x;
  char dummy[3];
  if ( ris1[strlen ( ris1 ) - 1] == ';' ) {
    result = 0;
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  if ( ris1[strlen ( ris1 ) - 1] == '+' ) {
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  if ( ris1[strlen ( ris1 ) - 1] == '#' ) {
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  i = ( int ) strlen ( ris1 );
  memset ( dummy, 0, sizeof ( dummy ) );
  switch ( i ) {
  case 2:
    strncpy ( dummy, ris1, 2 );
    break;
  case 3:
    if ( !strcmp ( ris1, "O-O" ) ) {
      if ( black_move )
	strcpy ( dummy, "g8" );
      else
	strcpy ( dummy, "g1" );
    }
    else if ( ris1[0] < 91 )
      strncpy ( dummy, ris1 + 1, 2 );
    else
      strncpy ( dummy, ris1, 2 );
    break;
  case 4:
    strncpy ( dummy, ris1 + 2, 2 );
    break;
  case 5:
    if ( !strcmp ( ris1, "O-O-O" ) ) {
      if ( black_move )
	strcpy ( dummy, "c8" );
      else
	strcpy ( dummy, "c1" );
    }
    else if ( ris1[3] == '+' ) {
      if ( ris1[0] < 91 )
	strncpy ( dummy, ris1 + 1, 2 );
      else
	strncpy ( dummy, ris1, 2 );
    }
    else if ( x = strstr ( ris1, "x" ) )
      strncpy ( dummy, x + 1, 2 );
    else
      strncpy ( dummy, ris1 + 3, 2 );
    break;
  case 6:
    strncpy ( dummy, ris1 + 3, 2 );
    break;
  default:
    printf ( "\nPARSE ERROR" );
    break;
  }
  if ( !( dummy[0] >= 'a' && dummy[0] <= 'h' && dummy[1] >= '1' && dummy[1] <= '8' ) )
    printf ( "\nPARSE ERROR" );
  strcat ( test_ris, dummy );
  strcat ( test_ris, " " );
  return result;
}

void
get_test_result ( char *ss ) {
  memset ( test_ris, 0, sizeof ( test_ris ) );
  char *ris1 = strstr ( ss, " bm " );
  if ( !ris1 )
    ris1 = strstr ( ss, " am " );
  ASSERT ( ris1 );
  char dummy[100];
  ris1 += 4;
  char *ris2;
  do {
    ris2 = strstr ( ris1, " " );
    ASSERT ( ris2 );
    strncpy ( dummy, ris1, ris2 - ris1 );
    dummy[ris2 - ris1] = 0;
    ris1 += ris2 - ris1 + 1;
  }
  while ( extract_test_result ( dummy ) );
  ris1 = dummy;
  strcpy ( dummy, test_ris );
  do {
    ris2 = strstr ( dummy, " " );
    if ( ris2 ) {
      strncpy ( dummy, ris1, ris2 - ris1 );
      dummy[ris2 - ris1] = 0;
      ris1 += ris2 - ris1 + 1;
    }
    if ( decodeBoard ( dummy ) == -1 )
      myassert ( 0, "PARSE ERROR" );
  }
  while ( ris2 );
}
#endif
int
loadfen ( char *ss ) {
  return loadfen ( ss, 1 );
}

int
loadfen ( char *ss, int check ) {
  int i, ii, t, p;
  char ch;
  char *x;
  int s[64];
  char a[2];
  memset ( chessboard, 0, sizeof ( chessboard ) );

  i = 0;
  ii = 0;
  if ( strlen ( ss ) )
    do {
      ch = ss[ii];
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
      case '/':
	;
	break;
      case ' ':
	;
	break;
      case '-':
	;
	break;
      case 'w':
	;
	break;
      case 10:
	;
	break;
      case 13:
	;
	break;
      default:{
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
      }
      }
      ii++;
    } while ( i < 64 );
  for ( i = 0; i <= 63; i++ ) {
    p = s[63 - i];
    if ( p != SQUARE_FREE )
      chessboard[p] |= tablog ( i );
  };
  if ( ( x = strstr ( ss, " b " ) ) )
    black_move = 1;
  else if ( ( x = strstr ( ss, " w " ) ) )
    black_move = 0;
  else
    printf ( "Bad FEN position format.\n%s", ss );
  x += 3;
#ifdef TEST_MODE
  if ( check )
    get_test_result ( ss );

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
  while ( ( unsigned ) i < strlen ( x ) && x[i] != ' ' ) {
    switch ( x[i++] ) {
    case 'K':
      CASTLE_DONE[WHITE] = 0;
      CASTLE_NOT_POSSIBLE[WHITE] = 0;
      CASTLE_NOT_POSSIBLE_KINGSIDE[WHITE] = 0;
      break;
    case 'k':
      CASTLE_DONE[BLACK] = 0;
      CASTLE_NOT_POSSIBLE[BLACK] = 0;
      CASTLE_NOT_POSSIBLE_KINGSIDE[BLACK] = 0;
      break;
    case 'Q':
      CASTLE_DONE[WHITE] = 0;
      CASTLE_NOT_POSSIBLE[WHITE] = 0;
      CASTLE_NOT_POSSIBLE_QUEENSIDE[WHITE] = 0;
      break;
    case 'q':
      CASTLE_DONE[BLACK] = 0;
      CASTLE_NOT_POSSIBLE[BLACK] = 0;
      CASTLE_NOT_POSSIBLE_QUEENSIDE[BLACK] = 0;
      break;
    };
  };
  START_CASTLE_NOT_POSSIBLE[0] = CASTLE_NOT_POSSIBLE[0];
  START_CASTLE_NOT_POSSIBLE_QUEENSIDE[0] = CASTLE_NOT_POSSIBLE_QUEENSIDE[0];
  START_CASTLE_NOT_POSSIBLE_KINGSIDE[0] = CASTLE_NOT_POSSIBLE_KINGSIDE[0];
  START_CASTLE_DONE[0] = CASTLE_DONE[0];
  START_CASTLE_NOT_POSSIBLE[1] = CASTLE_NOT_POSSIBLE[1];
  START_CASTLE_NOT_POSSIBLE_QUEENSIDE[1] = CASTLE_NOT_POSSIBLE_QUEENSIDE[1];
  START_CASTLE_NOT_POSSIBLE_KINGSIDE[1] = CASTLE_NOT_POSSIBLE_KINGSIDE[1];
  START_CASTLE_DONE[1] = CASTLE_DONE[1];
  Friend_king[black_move] = BITScanForward ( chessboard[KING_BLACK + black_move] );
  Friend_king[black_move ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( black_move ^ 1 )] );
  init (  );
#ifndef PERFT_MODE
  //memset(&stack_move1, 0, sizeof(stack_move1));
  //stack_move1.board[stack_move1.next++] = makeZobristKey();
#endif
  return !black_move;
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
  u64 key2;
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
    /*if (!strcmp(fen, "O-O"))
       pushmove(CASTLE, KINGSIDE, SIDE, -1);
       else
       pushmove(CASTLE, QUEENSIDE, SIDE, -1); */
    Tmove *mossa = &gen_list[0][1];
    makemove ( mossa, &key2 );
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
    char p = getFenInv[strstr ( fen, "=" )[1]];
    pushmove ( PROMOTION, *from, *to, SIDE, p );
    Tmove *mossa = &gen_list[0][1];
    makemove ( mossa, &key2 );
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
      pezzo_da = getFenInv[( char ) tolower ( fen[0] )];
    else
      pezzo_da = getFenInv[fen[0]];
  }
  *to = decodeBoard ( fen + strlen ( fen ) - 2 );
  if ( ( *to ) == -1 )
    myassert ( 0, "e" );
  u64 attaccanti2, attaccanti = EVAL.attackers[*to];
  attaccanti &= get_pieces ( SIDE ) | chessboard[SIDE];
  int c = BitCount ( attaccanti );
  if ( !c ) {
    printf ( "\nerror en passant?" );
    *from = -1;
    return 0;
  }
  if ( c == 1 )
    *from = BITScanForward ( attaccanti );
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
       ASSERT(pezzo_da!=-1);
       #endif
       } */
    try_locked = 0;
    attaccanti2 = attaccanti;
    while ( attaccanti ) {
      o = BITScanForward ( attaccanti );
      if ( ( ( ( pezzo_da == -1 ) || ( pezzo_da != -1 ) ) && ( ( chessboard[pezzo_da] & TABLOG[o] & column ) ) ) && ( TABLOG[o] & column ) ) {
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
	o = BITScanForward ( attaccanti );
	if ( ( ( pezzo_da == -1 || pezzo_da != -1 ) && ( ( chessboard[pezzo_da] & TABLOG[o] & column ) ) ) && TABLOG[o] & column && !is_locked ( o, get_piece_at ( SIDE, TABLOG[o] ), SIDE ) ) {
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
  makemove ( mossa, &key2 );
  score = eval ( SIDE
#ifdef FP_MODE
		 , -_INFINITE, _INFINITE
#endif
		 , key );
  return score;
}

#endif
