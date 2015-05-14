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
#ifndef PERFT_MODE
#include "bitmap.h"
#include "maindefine.h"
#include "utility.h"
#include "gen.h"
#include "search.h"
#include "zobrist.h"
#include "winboard.h"
#include <stdio.h>
#include <sys/timeb.h>
#ifdef _MSC_VER
#include "LIMITS.H"
#endif
#include "extern.h"
#include "cap.h"
#include "swapoff.h"
#include "openbook.h"
int
evaluateMobility ( const int tipo ) {
  //return 0;
  evaluateMobility_mode = 1;
  list_id++;
#ifdef DEBUG_MODE
  assert ( list_id < MAX_PLY );
#endif
  generateCap ( STANDARD, tipo );
  generateMoves ( STANDARD, tipo );
  int listcount = gen_list[list_id][0].score;
  if ( listcount == -1 )
    listcount = 0;
  gen_list[list_id][0].score = 0;
  list_id--;
  evaluateMobility_mode = 0;
  return listcount;
};

void
open_column ( const int side, const u64 ALLPIECES, u64 torri_amiche, const u64 PAWNS_enemies ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int o;
  u64 u;
  while ( torri_amiche ) {
    o = BitScanForward ( torri_amiche );
    if ( ( u = ( VERTICAL[o] & ALLPIECES ) ) == ( VERTICAL[o] & torri_amiche ) )
      evalNode.open_column[side] |= TABLOG[o];
    else if ( u == ( VERTICAL[o] & torri_amiche & PAWNS_enemies ) )
      evalNode.colonna_semi_aperta[side] |= TABLOG[o];
    torri_amiche &= NOTTABLOG[o];
  };
}


int
evaluate_pawn ( const int side, u64 ped_friends, const u64 ped_enemies, const u64 pezzi_enemies, const int pos_re_amico ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int result = 0;

  if ( !ped_friends )
    return 0;
  /*u64 key=makeZobristKey_pawn();
     Thash *phashe = &hash_array_pawn[key % HASH_SIZE_PAWN];
     if( phashe->key == key ){
     #ifdef DEBUG_MODE
     n_cut_hash_pawn++;
     #endif
     return phashe->score;
     } */
  if ( BitCount ( ped_enemies ) == 8 )
    result -= ENEMIES_PAWNS_ALL;


  int oo;

  evalNode.isolati = 0;
  ////////spazio // 2/7 traversa
  if ( side ) {
    result += PAWN_7H * BitCount ( ped_friends & ORIZZONTAL_48 );
    result += PED_CENTRE * BitCount ( ped_friends & 0x181800 );
  }
  else {
    result += PAWN_7H * BitCount ( ped_friends & ORIZZONTAL_8 );
    result += PED_CENTRE * BitCount ( ped_friends & 0x18180000000000ULL );
  }
  while ( ped_friends ) {
    oo = BitScanForward ( ped_friends );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/Ndifensori_pezzo+1)

    evalNode.sicurezza_re[side] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_re_amico][oo] ) / ( VALUEPAWN / ( BitCount ( calcola_attaccanti ( oo, side ^ 1 ) ) + 1 ) ) );


    /////////////////////////
    //if ((FORK_PAWN_PATTERN[side][oo] & pezzi_enemies)==FORK_PAWN_PATTERN[side][oo])
    //      print();
    /////isolato //nelle colonne  adiacenti non ha PAWNS ped_friends
    if ( !( ped_friends & ISOLATED_MASK[oo] ) ) {
#ifdef DEBUG_MODE
      assert ( get_column[oo] == oo % 8 );
#endif
      result -= ISO * ( ISOLATED[get_column[oo]] );
      evalNode.isolati |= TABLOG[oo];
    }
    //////doppiato //due PAWNS friends sulla stessa colonna

    if ( NOTTABLOG[oo] & VERTICAL[oo] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( evalNode.isolati & TABLOG[oo] ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    /////////BACKWARD //un pawn che nelle colonne adiecenti non ha un amico a fianco o piu arretrato di uno

    if ( !( ped_friends & BACKWARD_MASK[side][oo] ) ) {
      result -= BACKWARD_PAWN;
      if ( evalNode.open_column[side] & TABLOG[oo] )
	result -= OPEN_FILE;
      else if ( evalNode.attaccanti[oo] )
	result -= BACKWARD_OPEN_PAWN;
    }
    /////////forchetta
    if ( BitCount ( pezzi_enemies & evalNode.attaccate[oo] ) == 2 )
      result += FORCHETTA_SCORE;

    /////////passato  //un pawn che non ha PAWNS avversari che possano impedirne la marcia verso la promozione ne sulla stessa colonna ne su quelle adiacenti

    if ( !( ped_enemies & PASSATO_MASK[side][oo] ) )
      result += PAWN_PUSH * ( PASSATO[side][oo] );

    ped_friends &= NOTTABLOG[oo];
  };
  //RecordHash_pawn(key,result);


  return result;

};


int
evaluate_bishop ( const int tipo, const u64 adiacente_re_nemico, const u64 pezzi_enemies, const int pos_re_amico ) {
  //mobilita
  //intrappolato
  //spazio
  //re nemico
  //pezzi attaccati
  //2 alfieri

  //fianchetto di fronte al re arroccato TODO
  int o, result = 0, mob;
  u64 x = chessboard[BISHOP_BLACK + tipo];
  if ( FINE_APERTURA && BitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  while ( x ) {

    o = BitScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.sicurezza_re[tipo] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_re_amico][o] ) / ( VALUEBISHOP / ( BitCount ( calcola_attaccanti ( o, tipo ^ 1 ) ) + 1 ) ) );

    /////////////////////////
    if ( tipo ) {
      if ( o & ORIZZONTAL_0 )
	result -= UNDEVELOPED;
      if ( ( o == D3 || o == E3 ) && get_piece_at ( WHITE, TABLOG[o + 8] ) == WHITE )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( o & ORIZZONTAL_7 )
	result -= UNDEVELOPED;
      if ( ( o == D6 || o == E6 ) && get_piece_at ( BLACK, TABLOG[o - 8] ) == BLACK )
	result -= BLOCK_PAWNS;
    }
    if ( evalNode.re_attaccato[o] )
      result += ( BISHOP_ATTACK );
    mob = BitCount ( evalNode.attaccate[o] );
    if ( !mob )
      result -= BISHOP_TRAPPED;
    else {
      if ( !( BIG_DIAG_LEFT[o] & case_all_bit_occupate (  ) ) )
	result += OPEN_FILE;	//diagonale aperta
      if ( !( BIG_DIAG_RIGHT[o] & case_all_bit_occupate (  ) ) )
	result += OPEN_FILE;	//diagonale aperta

      result += MOB * mob;
      result += BitCount ( pezzi_enemies & evalNode.attaccate[o] );
      if ( adiacente_re_nemico & evalNode.attaccate[o] )
	result += NEAR_XKING;
    }
    result += SPACE * ( DISTANCE_BISHOP[o] );

    x &= NOTTABLOG[o];
  };
  return result;
};

int
evaluate_queen ( const int tipo, u64 queen, const int pos_re_nemico, const u64 ped_enemies, const u64 pezzi_enemies, const u64 ALLPIECES, const int pos_re_amico ) {
  int o, mob, result = 0;
  while ( queen ) {

    o = BitScanForward ( queen );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.sicurezza_re[tipo] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_re_amico][o] ) / ( VALUEQUEEN / ( BitCount ( calcola_attaccanti ( o, tipo ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    if ( tipo ) {
      if ( ( o == D3 || o == E3 ) && get_piece_at ( WHITE, TABLOG[o + 8] ) == WHITE )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( ( o == D6 || o == E6 ) && get_piece_at ( BLACK, TABLOG[o - 8] ) == BLACK )
	result -= BLOCK_PAWNS;
    }
    if ( ( ped_enemies & VERTICAL[o] ) )
      result += HALF_OPEN_FILE_Q;
    if ( ( VERTICAL[o] & ALLPIECES ) == TABLOG[o] )
      result += OPEN_FILE_Q;
    if ( evalNode.re_attaccato[o] )
      result += ( QUEEN_ATTACK );
    if ( !( mob = BitCount ( evalNode.attaccate[o] ) ) )
      result -= QUEEN_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( pezzi_enemies & evalNode.attaccate[o] );
    }
    if ( LEFT_RIGHT[o] & chessboard[BISHOP_BLACK + tipo] )
      result += BISHOP_ON_QUEEN;
    if ( FINE_APERTURA )
      result -= DIST_XKING * ( DISTANCE[o][pos_re_nemico] );
    queen &= NOTTABLOG[o];
  };
  return result;
};

int
evaluate_knight ( const int tipo, const int pos_re_amico, const int pos_re_nemico, const u64 pezzi_enemies, const u64 ped_enemies ) {
  int mob, o, result = 0, ped_attaccanti = 0;

  u64 x = chessboard[KNIGHT_BLACK + tipo];
  while ( x ) {

    o = BitScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.sicurezza_re[tipo] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_re_amico][o] ) / ( VALUEKNIGHT / ( BitCount ( calcola_attaccanti ( o, tipo ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    /* Don't block in central pawns */
    if ( tipo ) {
      if ( o & ORIZZONTAL_0 )
	result -= UNDEVELOPED;
      if ( ( o == D3 || o == E3 ) && get_piece_at ( WHITE, TABLOG[o + 8] ) == WHITE )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( o & ORIZZONTAL_7 )
	result -= UNDEVELOPED;
      if ( ( o == D6 || o == E6 ) && get_piece_at ( BLACK, TABLOG[o - 8] ) == BLACK )
	result -= BLOCK_PAWNS;
    }
    if ( evalNode.re_attaccato[o] )
      result += ( KNIGHT_ATTACK );
    if ( !( mob = BitCount ( evalNode.attaccate[o] ) ) )
      result -= KNIGHT_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( pezzi_enemies & evalNode.attaccate[o] );
    }
    result -= DIST_XKING * ( DISTANCE[o][pos_re_amico] );
    result += SPACE * ( DISTANCE_KNIGHT[o] );
    result -= DIST_XKING * ( DISTANCE[o][pos_re_nemico] );

    ped_attaccanti += BitCount ( PAWN_CAPTURE_MASK[tipo][o] & ped_enemies );

    x &= NOTTABLOG[o];
  };

  return result + ( 8 - ped_attaccanti );
};

int
evaluate_king ( const int side, const u64 queen_nemica, const u64 PAWNS_friends, const int pos_re, const u64 pezzi_friends, const u64 pezzi_enemies ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int result, col;
  if ( FINE_APERTURA )
    result = SPACE * ( DISTANCE_RE_CLOSURE[pos_re] );
  else
    result = SPACE * ( DISTANCE_RE_OPENING[pos_re] );
  col = ( int ) TABLOG[get_column[pos_re]];
  if ( !FINE_APERTURA && ( ( evalNode.open_column[side] & col | evalNode.colonna_semi_aperta[side] & col ) || ( ( col <= 15 && col > 1 ) && ( evalNode.open_column[side] & ( col >> 1 ) | evalNode.colonna_semi_aperta[side] & ( col >> 1 ) ) ) || ( ( col != 128 ) && ( evalNode.open_column[side] & ( col << 1 ) | evalNode.colonna_semi_aperta[side] & ( col << 1 ) ) ) ) )
    result -= END_OPENING;

  /*
     If the number of enemy pieces and pawns in the friendly king's
     board quadrant is greater than the number of friendly pieces and pawns in the
     same quadrant, the side is penalised the difference multiplied by five.
     When considering enemy presence in the quadrant a queen is counted as
     three pieces. */
  int diff = BitCount ( QUADRANTS[pos_re] & pezzi_enemies ) - BitCount ( QUADRANTS[pos_re] & pezzi_friends );
  if ( diff > 0 ) {
    result -= ENEMY_NEAR_KING * diff;
    if ( QUADRANTS[pos_re] & queen_nemica )
      result -= XQUEEN_NEAR_KING;
  }
  if ( !( KING_MASK[pos_re] & PAWNS_friends ) )
    result -= PAWN_NEAR_KING;
  result += evalNode.sicurezza_re[side];
  result += evalNode.king_attak[side] / 2;
  if ( !( col = BitCount ( evalNode.attaccate[pos_re] ) ) )
    result -= KING_TRAPPED;

  else
    result += MOB * col;
  /*non muovereil re se non per l'arrocco */

  //if (!CASTLE_DONE[SIDE] && (SIDE && pos_re!=3 || !SIDE && pos_re!=59))
  //result -= BONUS_CASTLE;

  return result;
};

int
evaluate_rook ( const int tipo, const u64 ped_friends, const u64 ped_enemies, const int pos_re_nemico, const u64 all_pieces, const int pos_re_amico ) {
  //mobile su piu di 11 case
  //2 torri collegate
  //colonna aperta
  //colonna semi aperta
  //distanza re nemico
  // 2/7 traversa

  //difesa un pawn passato

  int mob, o, result = 0;

  u64 x = chessboard[TOWER_BLACK + tipo];
  if ( !x )
    return 0;
  int da = -1;
  int a = -1;
  while ( x ) {
    if ( FINE_APERTURA ) {
      if ( tipo && x & ORIZZONTAL_8 )
	result += ROOK_7TH_RANK;
      if ( !tipo && x & ORIZZONTAL_48 )
	result += ROOK_7TH_RANK;
    }
    o = BitScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.sicurezza_re[tipo] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_re_amico][o] ) / ( VALUETOWER / ( BitCount ( calcola_attaccanti ( o, tipo ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    if ( da == -1 )
      da = o;
    else
      a = o;
    mob = BitCount ( evalNode.attaccate[o] );
    if ( evalNode.re_attaccato[o] )
      result += ( ROOK_ATTACK );

    if ( !mob )
      result -= ROOK_TRAPPED;
    else {
      result += mob * MOB;
      if ( mob > 11 )
	result += BONUS_11;
    }
    if ( !( ped_friends & VERTICAL[o] ) )
      result += OPEN_FILE;
    if ( !( ped_enemies & VERTICAL[o] ) )
      result += OPEN_FILE;
    if ( FINE_APERTURA )
      result -= DIST_XKING * ( DISTANCE[o][pos_re_nemico] );

    /* Penalise if Rook is Blocked Horizontally */

    if ( ( ORIZZ_BOUND[o] & all_pieces ) != ORIZZ_BOUND[o] ) {
      //uchar bit_raw = (uchar) (shr(all_pieces ,pos_posMod8[o])) ;
      //      if(BitCount(MOVIMENTO_MASK_MOV[bit_raw][ROT45[o]])<3)
      result -= ROOK_BLOCKED;
    };
    x &= NOTTABLOG[o];
  };

  if ( a != -1 && ( !( LINK_TOWERS[da][a] & all_pieces ) ) )
    result += CONNECTED_ROOKS;
  return result;
};

int
passed_and_space ( const int tipo, u64 PAWNS, const u64 avversari ) {
  //un pawn che non ha PAWNS avversari che possano impedirne la marcia verso la promozione ne sulla stessa colonna ne su quelle adiacenti
  // 2/7 traversa

  int oo, result = 0;
  if ( tipo ) {
    result += PAWN_7H * BitCount ( PAWNS & ORIZZONTAL_48 );
    result -= BitCount ( PAWNS & 0x181800 );
  }
  else {
    result += PAWN_7H * BitCount ( PAWNS & ORIZZONTAL_8 );
    result -= BitCount ( PAWNS & 0x18180000000000ULL );
  }

  while ( PAWNS ) {
    oo = BitScanForward ( PAWNS );
    if ( !( avversari & PASSATO_MASK[tipo][oo] ) )
      result += PASSATO[tipo][oo];
    PAWNS &= NOTTABLOG[oo];
  };
  return result;
};



int
piece_attack ( const int side, u64 all, const int pos_re ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int o, result = 0;
  while ( all ) {
    o = BitScanForward ( all );
    if ( o != pos_re )
      result += see ( o, side ) / VALUEPAWN;
    all &= NOTTABLOG[o];
  };

  return result;
}

int
eval ( const int SIDE
#ifdef FP_MODE
       , const int alpha, const int beta
#endif
       , const u64 keyzobrist ) {
#ifdef DEBUG_MODE
  check_side ( SIDE );
#endif
  int mat;
#ifdef DEBUG_MODE
  eval_count++;
#endif
  int pas_spaz_black, pas_spaz_white;
  int lazyscore_white = lazy_eval_white (  );
  int lazyscore_black = lazy_eval_black (  );
#ifdef FP_MODE
  int lazyscore;
  lazyscore = lazyscore_black - lazyscore_white;
  if ( SIDE )
    lazyscore = -lazyscore;
  if ( abs ( beta ) != _INFINITE && abs ( alpha ) != _INFINITE && ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) ) {
#ifdef DEBUG_MODE
    EvalCuts++;
    //printf("\n%d %d",lazyscore,eval(SIDE,_INFINITE,_INFINITE,0));
#endif
    return lazyscore;
  }
#endif
  int result;


  if ( use_book && keyzobrist && ( ( mat = search_openbook ( keyzobrist, -1 ) ) != -1 ) ) {

    return openbook[mat].eval;
  }

  //  pattern=eval_pattern();TODO
  int castle_black = 0, castle_white = 0, mob_black, mob_white, p_black, p_white, a_black, a_white, k_black, k_white, r_black, r_white, t_black, t_white, c_black, c_white, attach_black, attach_white;
  memset ( &evalNode, 0, sizeof ( Teval ) );

  if ( !( mob_black = evaluateMobility ( BLACK ) ) ) {
    if ( SIDE )
      return _INFINITE;
    else
      return -_INFINITE;
  }
  if ( !( mob_white = evaluateMobility ( WHITE ) ) )
    if ( SIDE )
      return -_INFINITE;
    else
      return _INFINITE;

  u64 ALLPIECES = case_all_bit_occupate (  );
  int pos_re_nero = BitScanForward ( chessboard[KING_BLACK] );
  int pos_re_bianco = BitScanForward ( chessboard[KING_WHITE] );

#ifdef DEBUG_MODE
  assert ( pos_re_nero != -1 );
  assert ( pos_re_bianco != -1 );
#endif
  int attack_f7 = 0;
  if ( chessboard[KING_BLACK] == TABLOG_59 && ( ALLPIECES & 0x1C1C000000000000ULL ) == 0x1C1C000000000000ULL && attack_square ( BLACK, F7 ) )
    attack_f7 = ( ATTACK_F7_F2 );

  int attack_f2 = 0;
  if ( chessboard[KING_WHITE] == TABLOG_3 && ( ALLPIECES & 0x1C1C ) == 0x1C1C && attack_square ( WHITE, F2 ) )
    attack_f2 = ( ATTACK_F7_F2 );

  u64 adiacente_re_nero = KING_MASK[pos_re_nero];
  u64 adiacente_re_bianco = KING_MASK[pos_re_bianco];
  u64 all_pezzi_neri = square_bit_occupied ( BLACK );
  u64 all_pezzi_bianchi = square_bit_occupied ( WHITE );
  u64 ped_black = chessboard[BLACK];
  u64 ped_white = chessboard[WHITE];
  u64 queen_bianca = chessboard[QUEEN_WHITE];
  u64 queen_nera = chessboard[QUEEN_BLACK];
  u64 rook_nera = chessboard[TOWER_BLACK];
  u64 rook_bianca = chessboard[TOWER_WHITE];
  u64 pezzi_neri = get_pieces ( BLACK );
  u64 pezzi_bianchi = get_pieces ( WHITE );

  open_column ( WHITE, ALLPIECES, rook_bianca, ped_black );
  open_column ( BLACK, ALLPIECES, rook_nera, ped_white );

#ifdef DEBUG_MODE
  mob_black = mob_white = p_black = p_white = a_black = a_white = k_black = k_white = r_black = r_white = t_black = t_white = c_black = c_white = attach_black = attach_white = 0;
#endif
  pas_spaz_black = passed_and_space ( BLACK, chessboard[BLACK], chessboard[WHITE] );
  pas_spaz_white = passed_and_space ( WHITE, chessboard[WHITE], chessboard[BLACK] );

  p_black = pas_spaz_black + evaluate_pawn ( BLACK, ped_black, ped_white, pezzi_bianchi, pos_re_nero );
  p_white = pas_spaz_white + evaluate_pawn ( WHITE, ped_white, ped_black, pezzi_neri, pos_re_bianco );

  a_black = evaluate_bishop ( BLACK, adiacente_re_bianco, all_pezzi_bianchi, pos_re_nero );
  a_white = evaluate_bishop ( WHITE, adiacente_re_nero, all_pezzi_neri, pos_re_bianco );

  k_black = evaluate_king ( BLACK, queen_bianca, ped_black, pos_re_nero, all_pezzi_neri, all_pezzi_bianchi );
  k_white = evaluate_king ( WHITE, queen_nera, ped_white, pos_re_bianco, all_pezzi_bianchi, all_pezzi_neri );

  r_black = evaluate_queen ( BLACK, queen_nera, pos_re_bianco, ped_white, all_pezzi_bianchi, ALLPIECES, pos_re_nero );
  r_white = evaluate_queen ( WHITE, queen_bianca, pos_re_nero, ped_black, all_pezzi_neri, ALLPIECES, pos_re_bianco );

  t_black = evaluate_rook ( BLACK, ped_black, ped_white, pos_re_bianco, ALLPIECES, pos_re_nero );
  t_white = evaluate_rook ( WHITE, ped_white, ped_black, pos_re_nero, ALLPIECES, pos_re_bianco );

  c_black = evaluate_knight ( BLACK, pos_re_nero, pos_re_bianco, all_pezzi_bianchi, ped_white );
  c_white = evaluate_knight ( WHITE, pos_re_bianco, pos_re_nero, all_pezzi_neri, ped_black );

  attach_white = piece_attack ( BLACK, all_pezzi_neri, pos_re_nero );
  attach_black = piece_attack ( WHITE, all_pezzi_bianchi, pos_re_bianco );
  if ( CASTLE_DONE[BLACK] )
    castle_black = BONUS_CASTLE;
  if ( CASTLE_DONE[WHITE] )
    castle_white = -BONUS_CASTLE;
  result = ( lazyscore_black + attach_black + attack_f2 + mob_black + p_black + a_black + k_black + r_black + t_black + c_black + castle_black ) - ( lazyscore_white + attach_white + attack_f7 + mob_white + p_white + a_white + k_white + r_white + t_white + c_white + castle_white );

  /*muovere la queen dopo lo sviluppo dei pezzi minori
   */

  if ( SIDE )
    result = -result;

  return result;
}

#endif
