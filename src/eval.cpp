/*
Copyright (C) 2008-2010
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

#include "openbook.h"
int
evaluateMobility ( const int type ) {
  evaluateMobility_mode = 1;
  list_id++;
#ifdef DEBUG_MODE
  assert ( list_id < MAX_PLY );
#endif
  generateCap ( STANDARD, type );
  generateMoves ( STANDARD, type );
  int listcount = gen_list[list_id][0].score;
  if ( listcount == -1 )
    listcount = 0;
  gen_list[list_id][0].score = 0;
  list_id--;
  evaluateMobility_mode = 0;
  return listcount;
};

void
open_column ( const int side, const u64 ALLPIECES, u64 friends_rooks, const u64 PAWNS_enemies ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int o;
  u64 u;
  while ( friends_rooks ) {
    o = BITScanForward ( friends_rooks );
    if ( ( u = ( VERTICAL[o] & ALLPIECES ) ) == ( VERTICAL[o] & friends_rooks ) )
      evalNode.open_column[side] |= TABLOG[o];	//TABLOG[n] == pow(2,n)
    else if ( u == ( VERTICAL[o] & friends_rooks & PAWNS_enemies ) )
      evalNode.semi_open_column[side] |= TABLOG[o];
    friends_rooks &= NOTTABLOG[o];
  };
}


int
evaluate_pawn ( const int side, u64 ped_friends, const u64 ped_enemies, const u64 pieces_enemies, const int pos_friend_king ) {
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

  evalNode.isolated = 0;
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
    oo = BITScanForward ( ped_friends );

    evalNode.king_security[side] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_friend_king][oo] ) );	// / ( VALUEPAWN / ( BitCount ( calcola_attaccanti ( oo, side ^ 1 ) ) + 1 ) ) );


    /////////////////////////
    //if ((FORK_PAWN_PATTERN[side][oo] & pieces_enemies)==FORK_PAWN_PATTERN[side][oo])
    //      print();
    /////isolato //nelle colonne  adiacenti non ha PAWNS ped_friends
    if ( !( ped_friends & ISOLATED_MASK[oo] ) ) {
#ifdef DEBUG_MODE
      assert ( get_column[oo] == oo % 8 );
#endif
      result -= ISO * ( ISOLATED[get_column[oo]] );
      evalNode.isolated |= TABLOG[oo];
    }
    //////doppiato //due PAWNS friends sulla stessa colonna

    if ( NOTTABLOG[oo] & VERTICAL[oo] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( evalNode.isolated & TABLOG[oo] ) )
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
    if ( BitCount ( pieces_enemies & evalNode.attaccate[oo] ) == 2 )
      result += FORK_SCORE;

    /////////passato  //un pawn che non ha PAWNS enemies che possano impedirne la marcia verso la promozione ne sulla stessa colonna ne su quelle adiacenti

    if ( !( ped_enemies & PASSED_MASK[side][oo] ) )
      result += PAWN_PUSH * ( PASSED[side][oo] );

    ped_friends &= NOTTABLOG[oo];
  };
  //RecordHash_pawn(key,result);


  return result;

};


int
evaluate_bishop ( const int type, const u64 near_friend_king, const u64 pieces_enemies, const int pos_friend_king ) {
  //mobilita
  //intrappolato
  //spazio
  //re nemico
  //pieces attaccati
  //2 alfieri

  //fianchetto di fronte al re arroccato TODO
  int o, result = 0, mob;
  u64 x = chessboard[BISHOP_BLACK + type];
  if ( END_OPEN && BitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  while ( x ) {
    o = BITScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.king_security[type] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_friend_king][o] ) );	// / ( VALUEBISHOP / ( BitCount ( calcola_attaccanti ( o, type ^ 1 ) ) + 1 ) ) );

    /////////////////////////
    if ( type ) {
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
    if ( evalNode.king_attacked[o] )
      result += ( BISHOP_ATTACK );
    mob = BitCount ( evalNode.attaccate[o] );
    if ( !mob )
      result -= BISHOP_TRAPPED;
    else {
      if ( !( BIG_DIAG_LEFT[o] & square_all_bit_occupied (  ) ) )
	result += OPEN_FILE;	//diagonale aperta
      if ( !( BIG_DIAG_RIGHT[o] & square_all_bit_occupied (  ) ) )
	result += OPEN_FILE;	//diagonale aperta

      result += MOB * mob;
      result += BitCount ( pieces_enemies & evalNode.attaccate[o] );
      if ( near_friend_king & evalNode.attaccate[o] )
	result += NEAR_XKING;
    }
    result += SPACE * ( DISTANCE_BISHOP[o] );

    x &= NOTTABLOG[o];
  };
  return result;
};

int
evaluate_queen ( const int type, u64 queen, const int pos_enemy_king, const u64 ped_enemies, const u64 pieces_enemies, const u64 ALLPIECES, const int pos_friend_king ) {
  int o, mob, result = 0;
  while ( queen ) {

    o = BITScanForward ( queen );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.king_security[type] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_friend_king][o] ) );	/// ( VALUEQUEEN / ( BitCount ( calcola_attaccanti ( o, type ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    if ( type ) {
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
    if ( evalNode.king_attacked[o] )
      result += ( QUEEN_ATTACK );
    if ( !( mob = BitCount ( evalNode.attaccate[o] ) ) )
      result -= QUEEN_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( pieces_enemies & evalNode.attaccate[o] );
    }
    if ( LEFT_RIGHT[o] & chessboard[BISHOP_BLACK + type] )
      result += BISHOP_ON_QUEEN;
    if ( END_OPEN )
      result -= DIST_XKING * ( DISTANCE[o][pos_enemy_king] );
    queen &= NOTTABLOG[o];
  };
  return result;
};

int
evaluate_knight ( const int type, const int pos_friend_king, const int pos_enemy_king, const u64 pieces_enemies, const u64 ped_enemies ) {
  int mob, o, result = 0, ped_attaccanti = 0;

  u64 x = chessboard[KNIGHT_BLACK + type];
  while ( x ) {

    o = BITScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.king_security[type] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_friend_king][o] ) );	// / ( VALUEKNIGHT / ( BitCount ( calcola_attaccanti ( o, type ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    /* Don't block in central pawns */
    if ( type ) {
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
    if ( evalNode.king_attacked[o] )
      result += ( KNIGHT_ATTACK );
    if ( !( mob = BitCount ( evalNode.attaccate[o] ) ) )
      result -= KNIGHT_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( pieces_enemies & evalNode.attaccate[o] );
    }
    result -= DIST_XKING * ( DISTANCE[o][pos_friend_king] );
    result += SPACE * ( DISTANCE_KNIGHT[o] );
    result -= DIST_XKING * ( DISTANCE[o][pos_enemy_king] );

    ped_attaccanti += BitCount ( PAWN_CAPTURE_MASK[type][o] & ped_enemies );

    x &= NOTTABLOG[o];
  };

  return result + ( 8 - ped_attaccanti );
};

int
evaluate_king ( const int side, const u64 enemy_queen, const u64 PAWNS_friends, const int pos_king, const u64 pieces_friends, const u64 pieces_enemies ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  int result, col;
  if ( END_OPEN )
    result = SPACE * ( DISTANCE_RE_CLOSURE[pos_king] );
  else
    result = SPACE * ( DISTANCE_RE_OPENING[pos_king] );
  col = ( int ) TABLOG[get_column[pos_king]];
  if ( !END_OPEN && ( ( evalNode.open_column[side] & col | evalNode.semi_open_column[side] & col ) || ( ( col <= 15 && col > 1 ) && ( evalNode.open_column[side] & ( col >> 1 ) | evalNode.semi_open_column[side] & ( col >> 1 ) ) ) || ( ( col != 128 ) && ( evalNode.open_column[side] & ( col << 1 ) | evalNode.semi_open_column[side] & ( col << 1 ) ) ) ) )
    result -= END_OPENING;

  /*
     If the number of enemy pieces and pawns in the friendly king's
     board quadrant is greater than the number of friendly pieces and pawns in the
     same quadrant, the side is penalised the difference multiplied by five.
     When considering enemy presence in the quadrant a queen is counted as
     three pieces. */
  int diff = BitCount ( QUADRANTS[pos_king] & pieces_enemies ) - BitCount ( QUADRANTS[pos_king] & pieces_friends );
  if ( diff > 0 ) {
    result -= ENEMY_NEAR_KING * diff;
    if ( QUADRANTS[pos_king] & enemy_queen )
      result -= XQUEEN_NEAR_KING;
  }

  if ( !( KING_MASK[pos_king] & PAWNS_friends ) )
    result -= PAWN_NEAR_KING;
  result += evalNode.king_security[side];
  result += evalNode.king_attak[side] / 2;
  if ( !( col = BitCount ( evalNode.attaccate[pos_king] ) ) )
    result -= KING_TRAPPED;
  else
    result += MOB * col;
  /*non muovereil re se non per l'arrocco */
  //if (!CASTLE_DONE[SIDE] && (SIDE && pos_king!=3 || !SIDE && pos_king!=59))
  //result -= BONUS_CASTLE;
  return result;
};

int
evaluate_rook ( const int type, const u64 ped_friends, const u64 ped_enemies, const int pos_enemy_king, const u64 all_pieces, const int pos_friend_king ) {
  //mobile su piu di 11 case
  //2 torri collegate
  //colonna aperta
  //colonna semi aperta
  //distanza re nemico
  // 2/7 traversa

  //difesa un pawn passato

  int mob, o, result = 0;

  u64 x = chessboard[ROOK_BLACK + type];
  if ( !x )
    return 0;
  int from = -1;
  int to = -1;
  while ( x ) {

    if ( END_OPEN ) {
      if ( type && x & ORIZZONTAL_8 )
	result += ROOK_7TH_RANK;
      if ( !type && x & ORIZZONTAL_48 )
	result += ROOK_7TH_RANK;
    }
    o = BITScanForward ( x );
    ///  vicinanza_al_proprio_re(pezzo) / (valore(pezzo)/N+1)

    evalNode.king_security[type] += KING_PROXIMITY * ( ( 8 - DISTANCE[pos_friend_king][o] ) );	/// ( VALUEROOK / ( BitCount ( calcola_attaccanti ( o, type ^ 1 ) ) + 1 ) ) );
    /////////////////////////
    if ( from == -1 )
      from = o;
    else
      to = o;
    mob = BitCount ( evalNode.attaccate[o] );
    if ( evalNode.king_attacked[o] )
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
    if ( END_OPEN )
      result -= DIST_XKING * ( DISTANCE[o][pos_enemy_king] );

    /* Penalise if Rook is Blocked Horizontally */

    if ( ( ORIZZ_BOUND[o] & all_pieces ) != ORIZZ_BOUND[o] ) {
      //uchar bit_raw = (uchar) (shr(all_pieces ,pos_posMod8[o])) ;
      //      if(BitCount(MOVIMENTO_MASK_MOV[bit_raw][ROT45[o]])<3)
      result -= ROOK_BLOCKED;
    };
    x &= NOTTABLOG[o];
  };

  if ( to != -1 && ( !( LINK_ROOKS[from][to] & all_pieces ) ) )
    result += CONNECTED_ROOKS;
  return result;
};

int
passed_and_space ( const int type, u64 PAWNS, const u64 enemies ) {
  //un pawn che non ha PAWNS enemies che possano impedirne la marcia verso la promozione ne sulla stessa colonna ne su quelle adiacenti
  // 2/7 traversa

  int oo, result = 0;
  if ( type ) {
    result += PAWN_7H * BitCount ( PAWNS & ORIZZONTAL_48 );
    result -= BitCount ( PAWNS & 0x181800 );
  }
  else {
    result += PAWN_7H * BitCount ( PAWNS & ORIZZONTAL_8 );
    result -= BitCount ( PAWNS & 0x18180000000000ULL );
  }

  while ( PAWNS ) {
    oo = BITScanForward ( PAWNS );
    if ( !( enemies & PASSED_MASK[type][oo] ) )
      result += PASSED[type][oo];
    PAWNS &= NOTTABLOG[oo];
  };
  return result;
};


/*
int
piece_attack ( const int side, u64 all, const int pos_king ) {
#ifdef DEBUG_MODE
  check_side ( side );
#endif
  return 0;
  int o, result = 0;
  while ( all ) {
    o = BITScanForward ( all );
    if ( o != pos_king )
      result += See ( o, side ) / VALUEPAWN;
    all &= NOTTABLOG[o];
  };

  return result;
}
*/
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

  int castle_black = 0, castle_white = 0, mob_black, mob_white, p_black, p_white, to_black, to_white, k_black, k_white, r_black, r_white, t_black, t_white, c_black, c_white;
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

  u64 ALLPIECES = square_all_bit_occupied (  );
  int pos_black_king = BITScanForward ( chessboard[KING_BLACK] );
  int pos_white_king = BITScanForward ( chessboard[KING_WHITE] );

#ifdef DEBUG_MODE
  assert ( pos_black_king != -1 );
  assert ( pos_white_king != -1 );
#endif
  int attack_f7 = 0;
  if ( chessboard[KING_BLACK] == TABLOG_59 && ( ALLPIECES & 0x1C1C000000000000ULL ) == 0x1C1C000000000000ULL && attack_square ( BLACK, F7 ) )
    attack_f7 = ( ATTACK_F7_F2 );

  int attack_f2 = 0;
  if ( chessboard[KING_WHITE] == TABLOG_3 && ( ALLPIECES & 0x1C1C ) == 0x1C1C && attack_square ( WHITE, F2 ) )
    attack_f2 = ( ATTACK_F7_F2 );

  u64 near_black_king = KING_MASK[pos_black_king];
  u64 near_white_king = KING_MASK[pos_white_king];
  u64 all_pieces_black = square_bit_occupied ( BLACK );
  u64 all_pieces_white = square_bit_occupied ( WHITE );
  u64 ped_black = chessboard[BLACK];
  u64 ped_white = chessboard[WHITE];
  u64 white_queen = chessboard[QUEEN_WHITE];
  u64 black_queen = chessboard[QUEEN_BLACK];
  u64 black_rook = chessboard[ROOK_BLACK];
  u64 white_rook = chessboard[ROOK_WHITE];
  u64 black_pieces = get_pieces ( BLACK );
  u64 white_pieces = get_pieces ( WHITE );

  open_column ( WHITE, ALLPIECES, white_rook, ped_black );
  open_column ( BLACK, ALLPIECES, black_rook, ped_white );

#ifdef DEBUG_MODE
  mob_black = mob_white = p_black = p_white = to_black = to_white = k_black = k_white = r_black = r_white = t_black = t_white = c_black = c_white = 0;
#endif
  pas_spaz_black = passed_and_space ( BLACK, chessboard[BLACK], chessboard[WHITE] );
  pas_spaz_white = passed_and_space ( WHITE, chessboard[WHITE], chessboard[BLACK] );

  p_black = evaluate_pawn ( BLACK, ped_black, ped_white, white_pieces, pos_black_king );
  p_white = evaluate_pawn ( WHITE, ped_white, ped_black, black_pieces, pos_white_king );

  to_black = evaluate_bishop ( BLACK, near_white_king, all_pieces_white, pos_black_king );
  to_white = evaluate_bishop ( WHITE, near_black_king, all_pieces_black, pos_white_king );

  k_black = evaluate_king ( BLACK, white_queen, ped_black, pos_black_king, all_pieces_black, all_pieces_white );
  k_white = evaluate_king ( WHITE, black_queen, ped_white, pos_white_king, all_pieces_white, all_pieces_black );

  r_black = evaluate_queen ( BLACK, black_queen, pos_white_king, ped_white, all_pieces_white, ALLPIECES, pos_black_king );
  r_white = evaluate_queen ( WHITE, white_queen, pos_black_king, ped_black, all_pieces_black, ALLPIECES, pos_white_king );

  t_black = evaluate_rook ( BLACK, ped_black, ped_white, pos_white_king, ALLPIECES, pos_black_king );
  t_white = evaluate_rook ( WHITE, ped_white, ped_black, pos_black_king, ALLPIECES, pos_white_king );

  c_black = evaluate_knight ( BLACK, pos_black_king, pos_white_king, all_pieces_white, ped_white );
  c_white = evaluate_knight ( WHITE, pos_white_king, pos_black_king, all_pieces_black, ped_black );

  if ( CASTLE_DONE[BLACK] )
    castle_black = BONUS_CASTLE;
  else if ( CASTLE_NOT_POSSIBLE[BLACK] )
    castle_black -= BONUS_CASTLE;

  if ( CASTLE_DONE[WHITE] )
    castle_white = BONUS_CASTLE;
  else if ( CASTLE_NOT_POSSIBLE[WHITE] )
    castle_white -= BONUS_CASTLE;

  result = ( pas_spaz_black + lazyscore_black + attack_f2 + mob_black + p_black + to_black + k_black + r_black + t_black + c_black + castle_black ) - ( pas_spaz_white + lazyscore_white + attack_f7 + mob_white + p_white + to_white + k_white + r_white + t_white + c_white + castle_white );

  /// printf ("\n%d %d %d %d %d |%d| %d %d %d |%d| %d %d\n", pas_spaz_black,lazyscore_black ,attack_f2 , mob_black , p_black ,to_black ,k_black ,r_black , t_black ,c_black , castle_black );
  //printf ("\n%d %d %d %d %d |%d| %d %d %d |%d| %d %d\n",  pas_spaz_white,lazyscore_white ,attack_f7 , mob_white , p_white , to_white, k_white , r_white , t_white, c_white , castle_white );



  /*TODO muovere la regina dopo lo sviluppo dei pieces minori
   */

  if ( SIDE )
    result = -result;

  return result;
}

#endif
