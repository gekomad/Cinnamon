#include "maindefine.h"
#include "stdafx.h"
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
#ifndef PERFT_MODE
int
quiescence ( int alpha, const int side, int beta, u64 key ) {
  if ( !running )
    return 0;
#ifdef DEBUG_MODE
  check_side ( side );
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  if ( !side && !chessboard[KING_BLACK] || side && !chessboard[KING_WHITE] ) {
    ASSERT ( 0 );
  }
#endif
  int score = -_INFINITE;
  if ( !( num_movesq++ & 1023 ) )
    if ( !( running = still_time (  ) ) )
      return score;
  int i, xside, listcount;
  Tmove *mossa;
  ASSERT ( makeZobristKey (  ) == key );
  score = eval ( side
#ifdef FP_MODE
		 , alpha, beta
#endif
		 , key );
  if ( score >= beta ) {
    return beta;
  }
  xside = side ^ 1;
  if ( score > alpha )
    alpha = score;
  list_id++;
  ASSERT ( list_id < MAX_PLY );
  if ( generateCap ( STANDARD, side ) ) {
    gen_list[list_id--][0].score = 0;
    return _INFINITE;
  }
  listcount = get_gen_list_count ( list_id );
  if ( !listcount ) {
    --list_id;
    return score;
  }
  qsort ( gen_list[list_id] + 1, listcount, sizeof ( Tmove ), compare_move );
  for ( i = 1; i <= listcount; i++ ) {
    mossa = get_gen_list ( list_id, i );
    makemove ( mossa, &key );
    int val = -quiescence ( -beta, xside, -alpha, key );
    score = _max ( score, val );
    takeback ( mossa );
    if ( score >= beta ) {
      gen_list[list_id--][0].score = 0;
      //if ( score == _INFINITE )
      //      update_pv ( pline, &line, mossa, 0 );
      return beta;
    }
    if ( score > alpha ) {
      //update_pv ( pline, &line, mossa, 0 );
      alpha = score;
    }
  }
  gen_list[list_id--][0].score = 0;
  return alpha;
}
#endif

int
ael ( const int SIDE, int depth
#ifndef PERFT_MODE
      , int alpha, int beta, LINE * pline, u64 key, int PVnode
#endif
   ) {
  if ( !running )
    return 0;
#ifdef DEBUG_MODE
  ASSERT ( chessboard[KING_BLACK] );
  ASSERT ( chessboard[KING_WHITE] );
  check_side ( SIDE );
  if ( !SIDE && !chessboard[KING_BLACK] || SIDE && !chessboard[KING_WHITE] )
    ASSERT ( 0 );
#endif
#ifndef PERFT_MODE
  int n_pieces_x_side = n_pieces ( SIDE ^ 1 );
  int n_pieces_side = n_pieces ( SIDE );
  int score = -_INFINITE;
  int is_incheck_side = in_check ( SIDE );
  if ( ( is_incheck_side ) /*|| make_extension() */  )
    depth++;
  //else if (check_draw(n_pieces_side + BitCount(chessboard[PAWN_BLACK + SIDE]), n_pieces_x_side + BitCount(chessboard[PAWN_BLACK + (SIDE ^ 1)]), SIDE))
  //      return 0;

#endif
  if ( depth == 0 ) {
#ifdef PERFT_MODE
    ++n_perft;
    return 0;
#else
    score = quiescence ( alpha, SIDE, beta, key );
#ifdef HASH_MODE
    //RecordHash (mply, hashfEXACT, SIDE, key, score);
#endif
    pline->cmove = 0;
    return score;
#endif
  };
  ++num_moves;
#ifdef TEST_MODE
  num_moves_test++;
#endif

#ifdef FP_MODE
  int fprune, mat_balance;
  int fscore, Fmax = 0;
#endif
#ifndef PERFT_MODE
  LINE line;
  main_depth = depth;
  line.cmove = 0;
  int val;
  if ( !( num_moves & 1023 ) )
    running = still_time (  );
  if ( !running )
    return score;
  //********* null move ***********
#ifdef NULL_MODE
  if (  /*!PVnode && */ !is_incheck_side && null_ok ( depth, SIDE, n_pieces_side ) ) {
    null_sem = 1;
    int null_score = -ael ( SIDE ^ 1, depth - R_adpt ( SIDE, depth ) - 1, -beta, -beta + 1, &line, key, PVnode );
    null_sem = 0;
    if ( null_score >= beta ) {
#ifdef DEBUG_MODE
      ++null_move_cut;
#endif
#ifdef HASH_MODE
      //RecordHash (depth-nullreduction, ....);
#endif
      //return beta;
      return null_score;
    }
  }
#endif
#endif
  ///******* null move end ********
#ifdef FP_MODE
  int is_incheck_xside = in_check ( SIDE ^ 1 );
	/**************Futility Pruning****************/
	/**************Futility Pruning razor at pre-pre-frontier*****/
  fprune = 0;

  ASSERT ( score == -_INFINITE );
  if ( !PVnode && depth <= 3 && !is_incheck_side ) {
    mat_balance = lazy_eval ( SIDE );
    if ( depth == 3 && n_pieces_x_side > 4 && ( mat_balance + RAZOR_MARGIN ) <= alpha ) {
#ifdef DEBUG_MODE
      n_cut_razor++;
#endif
      depth--;
    }
    else
      //**************Futility Pruning at pre-frontier*****
    if ( depth == 2 && ( fscore = mat_balance + EXT_FUTILY_MARGIN ) <= alpha ) {
      fprune = 1;
      score = Fmax = fscore;
    }
    else
      //**************Futility Pruning at frontier*****
    if ( depth == 1 && ( fscore = mat_balance + FUTIL_MARGIN ) <= alpha ) {
      fprune = 1;
      score = Fmax = fscore;
    }
  }
	/************ end Futility Pruning*************/
#endif
  //************* hash ****************
#ifdef HASH_MODE
  if ( !PVnode ) {
    Thash *phashe = &hash_array[SIDE][key % HASH_SIZE];
    if ( phashe->depth >= depth && phashe->key == key ) {

      ASSERT ( phashe->depth > 0 && phashe->depth <= 32 );
      ASSERT ( phashe->flags >= 0 && phashe->flags < 3 );
      ASSERT ( phashe->key != 0 );

      if ( phashe->flags == hashfEXACT ) {
#ifdef DEBUG_MODE
	n_cut_hash++;
#endif
	return phashe->score;
      }
      if ( phashe->flags == hashfBETA ) {
	if ( phashe->score >= beta ) {
#ifdef DEBUG_MODE
	  n_cut_hash++;
#endif
	  return phashe->score;
	}
	if ( phashe->score > alpha )
	  alpha = phashe->score;
	if ( phashe->score > score )
	  score = phashe->score;
      }
      else if ( phashe->flags == hashfALPHA ) {
	if ( phashe->score <= alpha ) {
#ifdef DEBUG_MODE
	  n_cut_hash++;
#endif
	  return phashe->score;
	}
	if ( phashe->score < beta )
	  beta = phashe->score;
      }
    }
  }
  char hashf = hashfALPHA;
#endif
  //********** end hash ***************
  int ii, listcount;
  Tmove *mossa;
#ifndef PERFT_MODE
  int bestmove;
#endif
  list_id++;
  ASSERT ( list_id < MAX_PLY );
  ASSERT ( KING_BLACK + SIDE >= 0 && KING_BLACK + SIDE < 12 );
  ASSERT ( KING_BLACK + change_side ( SIDE ) >= 0 && KING_BLACK + change_side ( SIDE ) < 12 );
  Friend_king[SIDE] = BITScanForward ( chessboard[KING_BLACK + SIDE] );
  Friend_king[change_side ( SIDE )] = BITScanForward ( chessboard[KING_BLACK + change_side ( SIDE )] );
  if ( generateCap ( STANDARD, SIDE ) ) {
    gen_list[list_id--][0].score = 0;
    return _INFINITE - ( main_ply - depth + 1 );
  }
  generateMoves ( STANDARD, SIDE );
  listcount = get_gen_list_count ( list_id );
  if ( !listcount ) {
    --list_id;
#ifdef HASH_MODE
    RecordHash ( ( char ) ( mply - depth ), hashfEXACT, SIDE, key, -_INFINITE );
#endif
    return -_INFINITE + ( main_ply - depth + 1 );
  }
#ifndef PERFT_MODE
  qsort ( gen_list[list_id] + 1, listcount, sizeof ( Tmove ), compare_move );
#endif

#ifdef PERFT_MODE
  listcount_n++;
#endif
  int check_alpha = 0;
  for ( ii = 1; ii <= listcount; ii++ ) {
#ifndef PERFT_MODE
    if ( ii == 1 )
      PVnode = 1;
#endif
    mossa = get_gen_list ( list_id, ii );
#ifdef DEBUG_MODE
    if ( mossa->type != CASTLE ) {
      assert ( mossa->from >= 0 && mossa->from < 64 );
      if ( get_piece_at ( SIDE, tablog ( mossa->from ) ) == 12 ) {
	print ( mossa->board );
	printf ( "\ncount: %d ERROR side%d mossa->from %s mossa->capture%d mossa->promotion_piece%d mossa->score%d mossa->side%d mossa->to %s mossa->type%d", ii, get_piece_at ( SIDE ^ 1, tablog ( mossa->from ) ), decodeBoardinv ( mossa->from, mossa->side ), mossa->capture, mossa->promotion_piece, mossa->score, mossa->side, decodeBoardinv ( mossa->to, mossa->side ), mossa->type );
	fflush ( stdout );
	assert ( 0 );
      }
    }

#endif
#ifdef PERFT_MODE
    u64 key;
#endif
    makemove ( mossa, &key );
#ifndef PERFT_MODE
    ASSERT ( stack_move1.next < MAX_PLY );
    ASSERT ( stack_move1.next >= 0 );
#endif

#ifdef FP_MODE

    int alpha_pre = alpha;
    if ( fprune && check_alpha && mossa->type != PROMOTION	/*&& mossa->capture == 12 */
	 && Fmax + PIECES_VALUE[mossa->capture] <= alpha && !in_check ( SIDE ) ) {
#ifdef DEBUG_MODE
      n_cut_fp++;
      ASSERT ( stack_move1.next > 0 );
#endif
      takeback ( mossa );
      continue;
    }
#endif

#ifdef PERFT_MODE
    ael ( change_side ( SIDE ), depth - 1 );
    takeback ( mossa );
#else
    ASSERT ( makeZobristKey (  ) == key );
    int do_mws = ( score > -_INFINITE );
    int lwb = _max ( alpha, score );
    int upb = ( do_mws ? ( lwb + 1 ) : beta );
    val = -ael ( change_side ( SIDE ), depth - 1, -upb, -lwb, &line, key, PVnode );
    if ( do_mws && ( lwb < val ) && ( val < beta ) ) {
      val = -ael ( change_side ( SIDE ), depth - 1, -beta, -val + 1, &line, key, PVnode );
    }
    score = _max ( score, val );
    takeback ( mossa );
    ASSERT ( stack_move1.next >= 0 );
    mossa->score = score;
    if ( score >= beta ) {
      gen_list[list_id--][0].score = 0;
#ifdef DEBUG_MODE
      ASSERT ( mossa->score == score );
      n_cut++;
      beta_efficency1 += 1 / ii;
#endif
#ifdef HASH_MODE
      RecordHash ( ( char ) depth, hashfBETA, SIDE, key, beta );
#endif
      ///// mate
      if ( score == _INFINITE ) {	//TODO
	update_pv ( pline, &line, mossa, depth );
      }
      /////
      return score;
    }
    PVnode = 0;
    if ( score > alpha ) {
      PVnode = 1;
      alpha = score;
      bestmove = mossa->score;
      check_alpha = 1;
#ifdef HASH_MODE
      hashf = hashfEXACT;
#endif
      update_pv ( pline, &line, mossa, depth );
    }

#endif
  }
  gen_list[list_id--][0].score = 0;
#ifdef HASH_MODE
  RecordHash ( ( char ) depth, hashf, SIDE, key, score );
#endif
#ifdef PERFT_MODE
  return 0;
#else
  return score;
#endif

}
