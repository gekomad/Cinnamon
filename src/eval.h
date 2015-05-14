#if !defined(EVAL_H)
#define EVAL_H
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
FORCEINLINE int
evaluateMobility ( const int side ) {

#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
  ASSERT ( !evaluateMobility_mode );
#endif
  evaluateMobility_mode = 1;
  list_id++;
#ifdef DEBUG_MODE
  if ( list_id < 0 || list_id >= MAX_PLY ) {
    printf ( "\nlist_id:%d\n", list_id );
    fflush ( stdout );
    ASSERT ( 0 );
  }
#endif
  generateCap ( STANDARD, side );
  generateMoves ( STANDARD, side );
  int listcount = get_gen_list_count ( list_id );
  if ( listcount == -1 )
    listcount = 0;
  gen_list[list_id][0].score = 0;
  list_id--;
  evaluateMobility_mode = 0;
  return listcount;
}

FORCEINLINE void
open_column ( const int side, EVAL_TAG * EVAL ) {
  check_side ( side );
  int o;
  u64 u;
  u64 side_rooks = EVAL->rooks[side];
  EVAL->open_column[side] = 0;
  EVAL->semi_open_column[side] = 0;
  while ( side_rooks ) {
    o = BITScanForward ( side_rooks );
    if ( ( u = ( VERTICAL[o] & EVAL->ALLPIECES ) ) == ( VERTICAL[o] & side_rooks ) )
      EVAL->open_column[side] |= TABLOG[o];	//TABLOG[n] == pow(2,n)
    else if ( u == ( VERTICAL[o] & side_rooks & EVAL->pawns[side ^ 1] ) )
      EVAL->semi_open_column[side] |= TABLOG[o];
    side_rooks &= NOTTABLOG[o];
  }
}

FORCEINLINE int
evaluate_pawn ( const int side, EVAL_TAG * EVAL ) {
#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
#endif
  u64 ped_friends = EVAL->pawns[side];
#ifdef TEST_MODE
  EVAL->passed_pawn_score[side] = 0;
#endif
  if ( !ped_friends )
    return 0;
  int result = 0;
  EVAL->isolated[side] = 0;
  /*u64 key=makeZobristKey_pawn();
     Thash *phashe = &hash_array_pawn[key % HASH_SIZE_PAWN];
     if( phashe->key == key ){
     #ifdef DEBUG_MODE
     n_cut_hash_pawn++;
     #endif
     return phashe->score;
     } */
  if ( BitCount ( EVAL->pawns[side ^ 1] ) == 8 )
    result -= ENEMIES_PAWNS_ALL;

  //space - 2/7th
  if ( side == WHITE ) {
    result += PED_CENTRE * BitCount ( ped_friends & 0x181800 );
    result += PAWN_7H * ( BitCount ( ped_friends & 0xFF000000000000 ) );
    result += PAWN_IN_RACE * BitCount ( 0xFF00000000000000 & ( ( ( ped_friends << 8 ) ) & ( ~EVAL->all_pieces[BLACK] ) ) );
#ifdef TEST_MODE
    EVAL->race_pawn_score[side] = PAWN_IN_RACE * BitCount ( 0xFF00000000000000 & ( ( ped_friends << 8 ) & ( ~EVAL->all_pieces[BLACK] ) ) );
#endif
  }
  else {
    result += PED_CENTRE * BitCount ( ped_friends & 0x18180000000000ULL );
    result += PAWN_7H * ( BitCount ( ped_friends & 0xFF00 ) );
    result += PAWN_IN_RACE * BitCount ( 0xFF & ( ( ped_friends >> 8 ) & ( ~EVAL->all_pieces[BLACK] ) ) );
#ifdef TEST_MODE
    EVAL->race_pawn_score[side] = PAWN_IN_RACE * BitCount ( 0xFF & ( ( ( ped_friends >> 8 ) ) & ( ~EVAL->all_pieces[BLACK] ) ) );;
#endif
  }

  int oo;
  while ( ped_friends ) {
    oo = BITScanForward ( ped_friends );
    if ( EVAL->king_attacked[oo] )
      result += ( PAWN_ATTACK_KING );
    //result +=VALUEPAWN;done in lazyeval
    EVAL->king_security_distance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side]][oo] ) );
    EVAL->king_security_distance[side ^ 1] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side ^ 1]][oo] ) );
    //isolated
    if ( !( ped_friends & ISOLATED_MASK[oo] ) ) {
      ASSERT ( get_column[oo] == oo % 8 );
      result -= ISO * ( ISOLATED[get_column[oo]] );
      EVAL->isolated[side] |= TABLOG[oo];
    }
    //doubled
    if ( NOTTABLOG[oo] & VERTICAL[oo] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( EVAL->isolated[side] & TABLOG[oo] ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    //backward
    if ( !( ped_friends & BACKWARD_MASK[side][oo] ) ) {
      result -= BACKWARD_PAWN;
      if ( EVAL->open_column[side] & TABLOG[oo] )
	result -= OPEN_FILE;
    }
    //fork
    if ( BitCount ( EVAL->all_pieces[side ^ 1] & EVAL->attacked[oo] ) == 2 )
      result += FORK_SCORE;
    //passed
    if ( !( EVAL->pawns[side ^ 1] & PASSED_MASK[side][oo] ) ) {
      int a = PAWN_PUSH * ( PASSED[side][oo] );
      result += a;
#ifdef TEST_MODE
      EVAL->passed_pawn_score[side] += a;
#endif
    }
    ped_friends &= NOTTABLOG[oo];
  }
  //RecordHash_pawn(key,result);
  return result;
}

FORCEINLINE int
evaluate_bishop ( const int side, EVAL_TAG * EVAL ) {
  //mobilita
  //intrappolato
  //spazio
  //re nemico
  //pieces attaccati
  //2 alfieri
  //fianchetto di fronte al re arroccato TODO
#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
#endif
  u64 x = chessboard[BISHOP_BLACK + side];
  if ( !x )
    return 0;
  int o, result = 0, mob;
  if ( EVAL->END_OPEN && BitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  // Check to see if the bishop is trapped at a7 or h7 with a pawn at b6 or g6 that has trapped the bishop.
  if ( side ) {
    if ( chessboard[PAWN_WHITE] & 0x400000 && chessboard[BISHOP_WHITE] & 0x8000 )
      result -= BISHOP_TRAPPED_DIAG;
    if ( chessboard[PAWN_WHITE] & 0x20000 && chessboard[BISHOP_WHITE] & 0x100 )
      result -= BISHOP_TRAPPED_DIAG;
  }
  else {
    if ( chessboard[PAWN_BLACK] & 0x400000000000ULL && chessboard[BISHOP_BLACK] & 0x80000000000000ULL )
      result -= BISHOP_TRAPPED_DIAG;
    if ( chessboard[PAWN_BLACK] & 0x20000000000ULL && chessboard[BISHOP_BLACK] & 0x1000000000000ULL )
      result -= BISHOP_TRAPPED_DIAG;
  }

  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEBISHOP;done in lazyeval
    EVAL->king_security_distance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side]][o] ) );
    EVAL->king_security_distance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side ^ 1]][o] ) );
    if ( side ) {
      if ( o == C1 || o == F1 )
	result -= UNDEVELOPED;	//TODO solo in apertura
      if ( ( o == D3 || o == E3 ) && chessboard[WHITE] & TABLOG[o - 8] )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( o == C8 || o == F8 )
	result -= UNDEVELOPED;	//TODO solo in apertura
      if ( ( o == D6 || o == E6 ) && chessboard[BLACK] & TABLOG[o + 8] )
	result -= BLOCK_PAWNS;
    }
    if ( EVAL->king_attacked[o] )
      result += ( BISHOP_ATTACK_KING );
    mob = BitCount ( EVAL->attacked[o] );
    if ( !mob )
      result -= BISHOP_TRAPPED;
    else {
      if ( !( BIG_DIAG_LEFT[o] & square_all_bit_occupied (  ) ) )
	result += OPEN_FILE;	//diagonale aperta
      if ( !( BIG_DIAG_RIGHT[o] & square_all_bit_occupied (  ) ) )
	result += OPEN_FILE;	//diagonale aperta
      result += MOB * mob;
      result += BitCount ( EVAL->all_pieces[side ^ 1] & EVAL->attacked[o] );
      if ( NEAR_MASK[EVAL->pos_king[side]] & EVAL->attacked[o] ) {
	result += NEAR_XKING;
      }
    }
    result += SPACE * ( DISTANCE_BISHOP[o] );
    x &= NOTTABLOG[o];
  };
  return result;
}

FORCEINLINE int
evaluate_queen ( const int side, EVAL_TAG * EVAL ) {
#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
#endif
  int o, mob, result = 0;
  u64 queen = chessboard[QUEEN_BLACK + side];
  while ( queen ) {
    o = BITScanForward ( queen );
    //result+=VALUEQUEEN;done in lazyeval
    EVAL->king_security_distance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side]][o] ) );
    EVAL->king_security_distance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side ^ 1]][o] ) );
    if ( side ) {
      if ( ( o == D3 || o == E3 ) && chessboard[WHITE] & TABLOG[o + 8] )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( ( o == D6 || o == E6 ) && chessboard[BLACK] & TABLOG[o - 8] )
	result -= BLOCK_PAWNS;
    }
    if ( ( EVAL->pawns[side ^ 1] & VERTICAL[o] ) )
      result += HALF_OPEN_FILE_Q;
    if ( ( VERTICAL[o] & EVAL->ALLPIECES ) == TABLOG[o] )
      result += OPEN_FILE_Q;
    if ( EVAL->king_attacked[o] )
      result += ( QUEEN_ATTACK_KING );
    if ( !( mob = BitCount ( EVAL->attacked[o] ) ) )
      result -= QUEEN_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( EVAL->all_pieces[side ^ 1] & EVAL->attacked[o] );
    }
    if ( LEFT_RIGHT[o] & Chessboard ( BISHOP_BLACK + side ) )
      result += BISHOP_ON_QUEEN;
    //if (END_OPEN)
    //result -= DIST_XKING * (DISTANCE[o][EVAL->pos_king[side ^ 1]]);
    queen &= NOTTABLOG[o];
  };
  return result;
}

FORCEINLINE int
evaluate_knight ( const int side, EVAL_TAG * EVAL ) {
#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
#endif
  int mob, o, result = 0, ped_attaccanti = 0;
  u64 x;
  x = Chessboard ( KNIGHT_BLACK + side );
  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEKNIGHT;//done in lazyeval
    EVAL->king_security_distance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side]][o] ) );
    EVAL->king_security_distance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side ^ 1]][o] ) );
    /* Don't block in central pawns */
    if ( side ) {
      if ( o & ORIZZONTAL_0 )
	result -= UNDEVELOPED;	//if (EVAL->END_OPEN TODO
      if ( ( o == D3 || o == E3 ) && chessboard[WHITE] & TABLOG[o + 8] )
	result -= BLOCK_PAWNS;
    }
    else {
      if ( o & ORIZZONTAL_7 )
	result -= UNDEVELOPED;	//if (EVAL->END_OPEN TODO
      if ( ( o == D6 || o == E6 ) && chessboard[BLACK] & TABLOG[o - 8] )
	result -= BLOCK_PAWNS;
    }
    if ( EVAL->king_attacked[o] )
      result += ( KNIGHT_ATTACK_KING );
    if ( !( mob = BitCount ( EVAL->attacked[o] ) ) )
      result -= KNIGHT_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( EVAL->all_pieces[side ^ 1] & EVAL->attacked[o] );
    }
    //result -= DIST_XKING * (DISTANCE[o][EVAL->pos_king[side]]);
    result += SPACE * ( DISTANCE_KNIGHT[o] );
    //result -= DIST_XKING * (DISTANCE[o][EVAL->pos_king[side ^ 1]]);
    ped_attaccanti += BitCount ( PAWN_CAPTURE_MASK[side][o] & EVAL->pawns[side ^ 1] );
    x &= NOTTABLOG[o];
  };
  result += ( 8 - ped_attaccanti );
  return result;
}

FORCEINLINE int
evaluate_king ( const int side, EVAL_TAG * EVAL ) {
#ifdef DEBUG_MODE
  check_side ( side );
  if ( N_EVALUATION[side] != 6 )
    myassert ( 0, "evaluate_king always the last" );
#endif
  int result, col;
  u64 pos_king = EVAL->pos_king[side];
  if ( EVAL->END_OPEN )
    result = SPACE * ( DISTANCE_KING_CLOSURE[pos_king] );
  else
    result = SPACE * ( DISTANCE_KING_OPENING[pos_king] );
  col = ( int ) TABLOG[get_column[pos_king]];
  if ( !EVAL->END_OPEN && ( ( ( EVAL->open_column[side] ) & ( col | EVAL->semi_open_column[side] & col ) ) || ( ( col <= 15 && col > 1 ) && ( ( EVAL->open_column[side] & ( col >> 1 ) ) | ( EVAL->semi_open_column[side] & ( col >> 1 ) ) ) ) || ( ( col != 128 ) && ( ( EVAL->open_column[side] & ( col << 1 ) ) | ( EVAL->semi_open_column[side] & ( col << 1 ) ) ) ) ) )
    result -= END_OPENING;
  /*
     If the number of enemy pieces and pawns in the friendly king's
     board quadrant is greater than the number of friendly pieces and pawns in the
     same quadrant, the side is penalised the difference multiplied by five.
     When considering enemy presence in the quadrant a queen is counted as
     three pieces. */

  //result -= PIECES_NEAR_KING *  BitCount(QUADRANTS[pos_king] & EVAL->all_pieces[side ^ 1]);
  //result += PIECES_NEAR_KING *  BitCount(QUADRANTS[pos_king] & EVAL->all_pieces[side ]);

  ASSERT ( pos_king >= 0 && pos_king < 64 );
  ASSERT ( pos_king >= 0 && pos_king < 64 );

  //if (QUADRANTS[pos_king] & EVAL->queens[side ^ 1])
  //result -= XQUEEN_NEAR_KING;
  if ( !( NEAR_MASK[pos_king] & EVAL->pawns[side] ) )
    result -= PAWN_NEAR_KING;
  //result += EVAL->king_security_distance[side];
  //result += EVAL->king_attak[side] / 2;
  if ( !( col = BitCount ( EVAL->attacked[pos_king] ) ) )
    result -= KING_TRAPPED;
  else
    result += MOB * col;
  result += EVAL->king_security_distance[side];
  /*non muovere il re se non per l'arrocco */
  //if (!CASTLE_DONE[SIDE] && (SIDE && pos_king!=3 || !SIDE && pos_king!=59))
  //result -= BONUS_CASTLE;
  return result;
}

FORCEINLINE int
evaluate_rook ( const int side, EVAL_TAG * EVAL ) {
  /*int evaluate_rook(const int type, const u64 ped_friends, const u64 ped_enemies,
     const int pos_enemy_king, const u64 all_pieces,
     const int pos_friend_king) { */
  //mobile su piu di 11 case
  //2 torri collegate
  //colonna aperta
  //colonna semi aperta
  //distanza re nemico
  // 2/7 traversa
  //difesa un pawn passato
#ifdef DEBUG_MODE
  check_side ( side );
  N_EVALUATION[side]++;
#endif
  int mob, o, result = 0;
  u64 x;
  x = Chessboard ( ROOK_BLACK + side );
  if ( !x )
    return 0;
  int from = -1;
  int to = -1;
  while ( x ) {
    if ( EVAL->END_OPEN ) {
      if ( !side && x & ORIZZONTAL_8 )
	result += ROOK_7TH_RANK;
      if ( side && x & ORIZZONTAL_48 )
	result += ROOK_7TH_RANK;
    }
    o = BITScanForward ( x );
    //result +=VALUEROOK;done in lazyeval
    EVAL->king_security_distance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side]][o] ) );
    EVAL->king_security_distance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[EVAL->pos_king[side ^ 1]][o] ) );
    if ( from == -1 )
      from = o;
    else
      to = o;
    mob = BitCount ( EVAL->attacked[o] );
    if ( EVAL->king_attacked[o] )
      result += ( ROOK_ATTACK_KING );
    if ( !mob )
      result -= ROOK_TRAPPED;
    else {
      result += mob * MOB;
      if ( mob > 11 )
	result += BONUS_11;
    }
    if ( !( EVAL->pawns[side] & VERTICAL[o] ) )
      result += OPEN_FILE;
    if ( !( EVAL->pawns[side ^ 1] & VERTICAL[o] ) )
      result += OPEN_FILE;
    //if (END_OPEN)
    //result -= DIST_XKING * (DISTANCE[o][EVAL->pos_king[side ^ 1]]);
    /* Penalise if Rook is Blocked Horizontally */
    if ( ORIZZ_BOUND[o] & EVAL->ALLPIECES ) {
      result -= ROOK_BLOCKED;
    };
    x &= NOTTABLOG[o];
  };
#ifdef DEBUG_MODE
  if ( to != -1 )
    ASSERT ( from >= 0 && from < 64 && to >= 0 && to < 64 );
#endif
  if ( to != -1 && ( !( LINK_ROOKS[from][to] & EVAL->ALLPIECES ) ) )
    result += CONNECTED_ROOKS;
  return result;
}

FORCEINLINE int
eval ( const int SIDE
#ifdef FP_MODE
       , const int alpha, const int beta
#endif
       , const u64 keyzobrist
#ifdef TEST_MODE
       , int *material_score, int *pawns_score, int *passed_pawns_score, int *knights_score, int *bishop_score, int *rooks_score, int *queens_score, int *kings_score, int *development_score, int *pawn_race_score, int *total_score
#endif
   ) {
  int lazyscore_white = lazy_eval_white (  );
  int lazyscore_black = lazy_eval_black (  );
#ifdef FP_MODE
  if ( abs ( beta ) < _INFINITE - 100000 && abs ( alpha ) < _INFINITE - 100000 ) {
    int lazyscore = lazyscore_black - lazyscore_white;
    if ( SIDE )
      lazyscore = -lazyscore;
    if ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) {
#ifdef DEBUG_MODE
      LazyEvalCuts++;
#endif
      if ( SIDE )		//white
	return lazyscore -= 5;	//5 bonus for the side on move.
      else
	return lazyscore += 5;
    }
  }
#endif
#ifdef DEBUG_MODE
  check_side ( SIDE );
  eval_count++;
  N_EVALUATION[0] = N_EVALUATION[1] = 0;
#endif
  int mob_black, mob_white;
  if ( use_book && keyzobrist && ( ( mob_black = search_openbook ( keyzobrist, SIDE ) ) != -1 ) ) {
    return openbook[mob_black].eval;
  }
  memset ( EVAL.attacked, 0, sizeof ( EVAL.attacked ) );
  memset ( EVAL.king_security_distance, 0, sizeof ( EVAL.king_security_distance ) );
  memset ( EVAL.king_attacked, 0, sizeof ( EVAL.king_attacked ) );
  memset ( EVAL.attackers, 0, sizeof ( EVAL.attackers ) );
#ifdef TEST_MODE
  memset ( EVAL.race_pawn_score, 0, sizeof ( EVAL.race_pawn_score ) );
#endif
  EVAL.END_OPEN = ( n_pieces ( 0 ) < 6 || n_pieces ( 1 ) < 6 );
  EVAL.ALLPIECES = square_all_bit_occupied (  );
  EVAL.pos_king[BLACK] = ( unsigned short ) BITScanForward ( chessboard[KING_BLACK] );
  EVAL.pos_king[WHITE] = ( unsigned short ) BITScanForward ( chessboard[KING_WHITE] );

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

  ASSERT ( EVAL.pos_king[0] != -1 );
  ASSERT ( EVAL.pos_king[1] != -1 );

  EVAL.all_pieces[BLACK] = square_bit_occupied ( BLACK );
  EVAL.all_pieces[WHITE] = square_bit_occupied ( WHITE );
  EVAL.pawns[BLACK] = chessboard[BLACK];
  EVAL.pawns[WHITE] = chessboard[WHITE];
  EVAL.queens[WHITE] = chessboard[QUEEN_WHITE];
  EVAL.queens[BLACK] = chessboard[QUEEN_BLACK];
  EVAL.rooks[BLACK] = chessboard[ROOK_BLACK];
  EVAL.rooks[WHITE] = chessboard[ROOK_WHITE];
  open_column ( WHITE, &EVAL );
  open_column ( BLACK, &EVAL );
  int pawns_score_black = evaluate_pawn ( BLACK, &EVAL );
  int pawns_score_white = evaluate_pawn ( WHITE, &EVAL );
  int bishop_score_black = evaluate_bishop ( BLACK, &EVAL );
  int bishop_score_white = evaluate_bishop ( WHITE, &EVAL );
  int queens_score_black = evaluate_queen ( BLACK, &EVAL );
  int queens_score_white = evaluate_queen ( WHITE, &EVAL );
  int rooks_score_black = evaluate_rook ( BLACK, &EVAL );
  int rooks_score_white = evaluate_rook ( WHITE, &EVAL );
  int knights_score_black = evaluate_knight ( BLACK, &EVAL );
  int knights_score_white = evaluate_knight ( WHITE, &EVAL );
  int kings_score_black = evaluate_king ( BLACK, &EVAL );
  int kings_score_white = evaluate_king ( WHITE, &EVAL );
  int castle_score[2];
  castle_score[BLACK] = castle_score[WHITE] = 0;
  if ( CASTLE_DONE[BLACK] )
    castle_score[BLACK] = BONUS_CASTLE;
  else if ( CASTLE_NOT_POSSIBLE[BLACK] )
    castle_score[BLACK] -= BONUS_CASTLE;
  if ( CASTLE_DONE[WHITE] )
    castle_score[WHITE] = BONUS_CASTLE;
  else if ( CASTLE_NOT_POSSIBLE[WHITE] )
    castle_score[WHITE] -= BONUS_CASTLE;
  int result = ( lazyscore_black + mob_black + pawns_score_black + knights_score_black + bishop_score_black + rooks_score_black + queens_score_black + kings_score_black + castle_score[BLACK] ) - ( lazyscore_white + mob_white + pawns_score_white + knights_score_white + bishop_score_white + rooks_score_white + queens_score_white + kings_score_white + castle_score[WHITE] );

  //printf ("\nblack: %d %d %d %d %d |%d| %d %d %d |%d| %d \n", pas_spaz_black,lazyscore_black ,attack_f2 , mob_black , p_black ,to_black ,k_black ,r_black , t_black ,c_black , castle_black );
  //printf ("\nwhite: %d %d %d %d %d |%d| %d %d %d |%d| %d \n",  pas_spaz_white,lazyscore_white ,attack_f7 , mob_white , p_white , to_white, k_white , r_white , t_white, c_white , castle_white );

  if ( SIDE )			//white
    result -= 5;		//5 bonus for the side on move.
  else
    result += 5;

#ifdef TEST_MODE
  *material_score = lazyscore_white - lazyscore_black;
  *pawns_score = pawns_score_white - pawns_score_black;
  *passed_pawns_score = EVAL.passed_pawn_score[WHITE] - EVAL.passed_pawn_score[BLACK];
  *knights_score = knights_score_white - knights_score_black;
  *bishop_score = bishop_score_white - bishop_score_black;
  *rooks_score = rooks_score_white - rooks_score_black;
  *queens_score = queens_score_white - queens_score_black;
  *kings_score = kings_score_white - kings_score_black;
  *development_score = -9999999;
  *pawn_race_score = EVAL.race_pawn_score[WHITE] - EVAL.race_pawn_score[BLACK];
  *total_score = -result;
#endif
  if ( SIDE )
    result = -result;
  return result;
}

#ifdef TEST_MODE
FORCEINLINE int
eval ( const int SIDE
#ifdef FP_MODE
       , const int alpha, const int beta
#endif
       , const u64 keyzobrist ) {
  int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
  return eval ( SIDE
#ifdef FP_MODE
		, alpha, beta
#endif
		, keyzobrist, &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11 );
}
#endif
#endif
#endif
