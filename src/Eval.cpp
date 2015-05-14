#include "Eval.h"

Eval::Eval (  ) {
}

Eval::~Eval (  ) {
}

void
Eval::openColumn ( int side ) {
  int o;
  u64 side_rooks = structure.rooks[side];
  structure.openColumn = 0;
  structure.semiOpenColumn[side] = 0;
  while ( side_rooks ) {
    o = BITScanForward ( side_rooks );
    if ( !( FILE_[o] & ( structure.pawns[WHITE] | structure.pawns[BLACK] ) ) )
      structure.openColumn |= FILE_[o];
    else if ( FILE_[o] & structure.pawns[side ^ 1] )
      structure.semiOpenColumn[side] |= FILE_[o];
    side_rooks &= NOTPOW2[o];
  }
}

template < int side, _Tstatus status > int
Eval::evaluatePawn (  ) {
  INC ( N_EVALUATION[side] );
  u64 ped_friends = structure.pawns[side];
  if ( !ped_friends )
    return 0;
  int result = 0;
  structure.isolated[side] = 0;
  if ( bitCount ( structure.pawns[side ^ 1] ) == 8 )
    result -= ENEMIES_PAWNS_ALL;
  result += ATTACK_KING * bitCount ( ped_friends & structure.kingAttackers[side ^ 1] );
  //space - 2/7th
  if ( side == WHITE ) {
    if ( status == OPEN )
      result += PAWN_CENTER * bitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( bitCount ( ped_friends & 0xFF000000000000ULL ) );
      result += PAWN_IN_RACE * bitCount ( 0xFF00000000000000ULL & ( ( ( ped_friends << 8 ) ) & ( ~structure.allPiecesSide[BLACK] ) ) );
    }
  }
  else {
    if ( status == OPEN )
      result += PAWN_CENTER * bitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( bitCount ( ped_friends & 0xFF00ULL ) );
      result += PAWN_IN_RACE * bitCount ( 0xFFULL & ( ( ped_friends >> 8 ) & ( ~structure.allPiecesSide[BLACK] ) ) );
    }
  }
  int o;
  u64 p = ped_friends;
  while ( p ) {
    o = BITScanForward ( p );
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, side))
      //  result -= PINNED_PIECE;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side ^ 1] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );

      /* if ( structure.kingAttacked[o]) {
         result += ATTACK_KING * 2;
         structure.attacKingCount[side ^ 1]++;
         } */
    }				/*else
				   //attack center
				   if (status == OPEN) {
				   result += ATTACK_CENTER * bitCount(structure.mobility[o] & CENTER_MASK);
				   } */
    //result +=VALUEPAWN;done in lazyeval


    //unprotected
    if ( !( ped_friends & PAWN_PROTECTED_MASK[side][o] ) ) {
      result -= UNPROTECTED_PAWNS;
    };
    //isolated
    if ( !( ped_friends & PAWN_ISOLATED_MASK[o] ) ) {
      result -= PAWN_ISOLATED;
      structure.isolated[side] |= POW2[o];
    }
    //doubled
    if ( NOTPOW2[o] & FILE_[o] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( structure.isolated[side] & POW2[o] ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    //backward
    if ( !( ped_friends & PAWN_BACKWARD_MASK[side][o] ) ) {
      result -= BACKWARD_PAWN;
    }
    //fork
    /*        if (bitCount(structure.allPiecesSide[side ^ 1] & structure.mobility[o]) == 2)
       result += FORK_SCORE; */
    //passed
    if ( !( structure.pawns[side ^ 1] & PAWN_PASSED_MASK[side][o] ) ) {
      result += PAWN_PASSED[side][o];
    }

    p &= NOTPOW2[o];
  }
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateBishop (  ) {
  INC ( N_EVALUATION[side] );
  u64 x = chessboard[BISHOP_BLACK + side];
  if ( !x )
    return 0;
  int o, result = 0;
  if ( status == END && bitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  // Check to see if the bishop is trapped at a7 or h7 with a pawn at b6 or g6 that has trapped the bishop.
  if ( side ) {
    if ( chessboard[PAWN_WHITE] & 0x400000ULL && chessboard[BISHOP_WHITE] & 0x8000ULL )
      result -= BISHOP_TRAPPED_DIAG;
    if ( chessboard[PAWN_WHITE] & 0x20000ULL && chessboard[BISHOP_WHITE] & 0x100ULL )
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
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, BISHOP_BLACK + side))
      //  result -= PINNED_PIECE;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );
      /*            if ( structure.kingAttacked[o]) {
         result += ATTACK_KING;
         structure.attacKingCount[side ^ 1]++;
         } */
    }
    else
      //attack center
    if ( status == OPEN ) {
//                result += ATTACK_CENTER * bitCount(structure.mobility[o] & CENTER_MASK);
      if ( side ) {
	if ( o == C1 || o == F1 )
	  result -= UNDEVELOPED;
      }
      else {
	if ( o == C8 || o == F8 )
	  result -= UNDEVELOPED;
      }
    }
//        if (!(mob = bitCount(structure.mobility[o])))
    //     result -= BISHOP_TRAPPED;
    else {
      if ( BIG_DIAG_LEFT & POW2[o] && !( LEFT_DIAG[o] & structure.allPieces ) )
	result += OPEN_FILE;
      if ( BIG_DIAG_RIGHT & POW2[o] && !( RIGHT_DIAG[o] & structure.allPieces ) )
	result += OPEN_FILE;
      /*            result += MOB * mob;
         result += bitCount(structure.allPieces & structure.mobility[o]);
         if (status != OPEN && NEAR_MASK1[structure.posKing[side]] & structure.mobility[o]) {
         result += NEAR_xKING;
         } */
    }
    x &= NOTPOW2[o];
  };
  return result;
}

template < _Tstatus status > int
Eval::evaluateQueen ( int side ) {
  INC ( N_EVALUATION[side] );
  int o, result = 0;
  u64 queen = chessboard[QUEEN_BLACK + side];
  while ( queen ) {
    o = BITScanForward ( queen );
    //result+=VALUEQUEEN;done in lazyeval
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, QUEEN_BLACK + side))
      //  result -= PINNED_PIECE * 3;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );
    }				//else //attack center
    //       if (status == OPEN) {
    //         result += ATTACK_CENTER * bitCount(structure.mobility[o] & CENTER_MASK);
    //   }
    if ( ( structure.pawns[side ^ 1] & FILE_[o] ) )
      result += HALF_OPEN_FILE_Q;
    if ( ( FILE_[o] & structure.allPieces ) == POW2[o] )
      result += OPEN_FILE_Q;
    /*        if (structure.kingAttacked[o]) {
       result += ATTACK_KING;
       structure.attacKingCount[side ^ 1]++;
       }
       mob = bitCount(structure.mobility[o]);
       if (status != OPEN && !mob)
       result -= QUEEN_TRAPPED;
       else {
       result += MOB * mob;
       result += bitCount(structure.allPiecesSide[side ^ 1] & structure.mobility[o]);
       } */
    if ( LEFT_RIGHT_DIAG[o] & chessboard[BISHOP_BLACK + side] )
      result += BISHOP_ON_QUEEN;

    queen &= NOTPOW2[o];
  };
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateKnight (  ) {
  INC ( N_EVALUATION[side] );
  int o, result = 0;

  u64 x = chessboard[KNIGHT_BLACK + side];
  if ( status == OPEN ) {
    side ? result -= bitCount ( x & 0x42ULL ) * UNDEVELOPED : result -= bitCount ( x & 0x4200000000000000ULL ) * UNDEVELOPED;
  }
  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEKNIGHT;//done in lazyeval
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, KNIGHT_BLACK + side))
      //  result -= PINNED_PIECE;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );
      /*            if (structure.kingAttacked[o]) {
         result += ATTACK_KING;
         structure.attacKingCount[side ^ 1]++;
         } */
    }				/*else
				   //attack center
				   if (status == OPEN)
				   result += ATTACK_CENTER * bitCount(structure.mobility[o] & CENTER_MASK);

				   if (!(mob = bitCount(structure.mobility[o])))
				   result -= KNIGHT_TRAPPED;
				   else {
				   result += MOB * mob;
				   result += bitCount(structure.allPiecesSide[side ^ 1] & structure.mobility[o]);
				   } */

    x &= NOTPOW2[o];
  };
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateRook (  ) {
  INC ( N_EVALUATION[side] );
  int firstRook = -1, secondRook = -1, o, result = 0;
  u64 x;
  if ( !( x = chessboard[ROOK_BLACK + side] ) )
    return 0;
  if ( status == MIDDLE ) {
    if ( !side && ( o = bitCount ( x & RANK_1 ) ) )
      result += ROOK_7TH_RANK * o;
    if ( side && ( o = bitCount ( x & RANK_6 ) ) )
      result += o * ROOK_7TH_RANK;
  }
  while ( x ) {
    o = BITScanForward ( x );

    if ( firstRook == -1 )
      firstRook = o;
    else
      secondRook = o;
    //result +=VALUEROOK;done in lazyeval
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, ROOK_BLACK + side))
      //  result -= PINNED_PIECE * 2;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );

      /*            if (structure.kingAttacked[o]) {
         result += ATTACK_KING;
         structure.attacKingCount[side ^ 1]++;
         } */
      /* Penalise if Rook is Blocked Horizontally */
      if ( ( RANK_BOUND[o] & structure.allPieces ) == RANK_BOUND[o] ) {
	result -= ROOK_BLOCKED;
      };
    }

    /*        if (!(mob = bitCount(structure.mobility[o])))
       result -= ROOK_TRAPPED;
       else {
       result += mob * MOB;
       if (mob > 11)
       result += BONUS_11;
       } */
    if ( !( structure.pawns[side] & FILE_[o] ) )
      result += OPEN_FILE;
    if ( !( structure.pawns[side ^ 1] & FILE_[o] ) )
      result += OPEN_FILE;

    x &= NOTPOW2[o];
  };
  if ( firstRook != -1 && secondRook != -1 )
    if ( ( !( _bits::LINK_ROOKS[firstRook][secondRook] & structure.allPieces ) ) ) {
      result += CONNECTED_ROOKS;
    }
  return result;
}

template < _Tstatus status > int
Eval::evaluateKing ( int side ) {
  ASSERT ( N_EVALUATION[side] == 5 );
  int result = 0;
  u64 pos_king = structure.posKing[side];
  if ( status == END )
    result = SPACE * ( DISTANCE_KING_ENDING[pos_king] );
  else
    result = SPACE * ( DISTANCE_KING_OPENING[pos_king] );

  u64 POW2_king = POW2[pos_king];
  if ( status != OPEN ) {
    if ( ( structure.openColumn & POW2_king ) || ( structure.semiOpenColumn[side ^ 1] & POW2_king ) ) {
      result -= END_OPENING;
      if ( bitCount ( RANK[pos_king] ) < 4 )
	result -= END_OPENING;
    }
    /*int mob;
       if (!(mob = bitCount(structure.mobility[pos_king])))
       result -= KING_TRAPPED;
       else
       result += MOB * mob; */
  }
  ASSERT ( pos_king < 64 );
  if ( !( NEAR_MASK1[pos_king] & structure.pawns[side] ) )
    result -= PAWN_NEAR_KING;
  result += structure.kingSecurityDistance[side];
  return result;
}

int
Eval::getScore ( const int side, const int alpha, const int beta ) {
  int lazyscore_white = lazyEvalSide < WHITE > (  );
  int lazyscore_black = lazyEvalSide < BLACK > (  );
#ifndef NO_FP_MODE
  int lazyscore = lazyscore_black - lazyscore_white;
  if ( side )
    lazyscore = -lazyscore;
  if ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) {
    INC ( LazyEvalCuts );
    //return lazyscore; TODO
    if ( side ) {		//white
      return lazyscore -= 5;	//5 bonus for the side on move.
    }
    else {
      return lazyscore += 5;
    }
  }
#endif
#ifdef DEBUG_MODE
  N_EVALUATION[WHITE] = N_EVALUATION[BLACK] = 0;
#endif
  //int mob_black=0;int mob_white=0;
//    memset(structure.mobility, 0, sizeof(structure.mobility));
  memset ( structure.kingSecurityDistance, 0, sizeof ( structure.kingSecurityDistance ) );
//    memset(structure.kingAttacked,0,  sizeof(structure.kingAttacked));
//    memset(structure.attacKingCount, 0, sizeof(structure.attacKingCount));

  int npieces = getNpiecesNoPawnNoKing < WHITE > (  ) + getNpiecesNoPawnNoKing < BLACK > (  );
  _Tstatus status;
  if ( npieces < 4 )
    status = END;
  else if ( npieces < 11 )
    status = MIDDLE;
  else
    status = OPEN;
  structure.allPiecesSide[BLACK] = getBitBoard < BLACK > (  );
  structure.allPiecesSide[WHITE] = getBitBoard < WHITE > (  );

  structure.allPieces = structure.allPiecesSide[BLACK] | structure.allPiecesSide[WHITE];
  structure.posKing[BLACK] = ( unsigned short ) BITScanForward ( chessboard[KING_BLACK] );
  structure.posKing[WHITE] = ( unsigned short ) BITScanForward ( chessboard[KING_WHITE] );
  structure.kingAttackers[WHITE] = getKingAttackers ( WHITE );
  // if(structure.kingAttackers[WHITE]){display();cout<<structure.kingAttackers[WHITE]<<endl;assert(0);};
  structure.kingAttackers[BLACK] = getKingAttackers ( BLACK );
  /*INC(N_EVALUATION[BLACK]);
     if (!(mob_black = evaluateMobility<BLACK>())) {
     return ( side? _INFINITE:  -_INFINITE);
     }
     INC(N_EVALUATION[WHITE]);
     if (!(mob_white = evaluateMobility<WHITE>())) {
     return (side?  -_INFINITE:  _INFINITE);
     }
   */
  structure.pawns[BLACK] = chessboard[BLACK];
  structure.pawns[WHITE] = chessboard[WHITE];
  structure.queens[WHITE] = chessboard[QUEEN_WHITE];
  structure.queens[BLACK] = chessboard[QUEEN_BLACK];
  structure.rooks[BLACK] = chessboard[ROOK_BLACK];
  structure.rooks[WHITE] = chessboard[ROOK_WHITE];
  openColumn ( WHITE );
  openColumn ( BLACK );

  int pawns_score_black;
  int pawns_score_white;
  int bishop_score_black;
  int bishop_score_white;
  int queens_score_black;
  int queens_score_white;
  int rooks_score_black;
  int rooks_score_white;
  int knights_score_black;
  int knights_score_white;
  int kings_score_black;
  int kings_score_white;
  int bonus_attack_black_king = 0;
  int bonus_attack_white_king = 0;
  if ( status == OPEN ) {
    pawns_score_black = evaluatePawn < BLACK, OPEN > (  );
    pawns_score_white = evaluatePawn < WHITE, OPEN > (  );

    bishop_score_black = evaluateBishop < BLACK, OPEN > (  );
    bishop_score_white = evaluateBishop < WHITE, OPEN > (  );

    queens_score_black = evaluateQueen < OPEN > ( BLACK );
    queens_score_white = evaluateQueen < OPEN > ( WHITE );

    rooks_score_black = evaluateRook < BLACK, OPEN > (  );
    rooks_score_white = evaluateRook < WHITE, OPEN > (  );

    knights_score_black = evaluateKnight < BLACK, OPEN > (  );
    knights_score_white = evaluateKnight < WHITE, OPEN > (  );

    kings_score_black = evaluateKing < OPEN > ( BLACK );
    kings_score_white = evaluateKing < OPEN > ( WHITE );
  }
  else {
    bonus_attack_black_king = BONUS_ATTACK_KING[bitCount ( structure.kingAttackers[WHITE] )];
    bonus_attack_white_king = BONUS_ATTACK_KING[bitCount ( structure.kingAttackers[BLACK] )];
    if ( status == END ) {
      pawns_score_black = evaluatePawn < BLACK, END > (  );
      pawns_score_white = evaluatePawn < WHITE, END > (  );

      bishop_score_black = evaluateBishop < BLACK, END > (  );
      bishop_score_white = evaluateBishop < WHITE, END > (  );

      queens_score_black = evaluateQueen < END > ( BLACK );
      queens_score_white = evaluateQueen < END > ( WHITE );

      rooks_score_black = evaluateRook < BLACK, END > (  );
      rooks_score_white = evaluateRook < WHITE, END > (  );

      knights_score_black = evaluateKnight < BLACK, END > (  );
      knights_score_white = evaluateKnight < WHITE, END > (  );

      kings_score_black = evaluateKing < END > ( BLACK );
      kings_score_white = evaluateKing < END > ( WHITE );
    }
    else {
      pawns_score_black = evaluatePawn < BLACK, MIDDLE > (  );
      pawns_score_white = evaluatePawn < WHITE, MIDDLE > (  );

      bishop_score_black = evaluateBishop < BLACK, MIDDLE > (  );
      bishop_score_white = evaluateBishop < WHITE, MIDDLE > (  );

      queens_score_black = evaluateQueen < MIDDLE > ( BLACK );
      queens_score_white = evaluateQueen < MIDDLE > ( WHITE );

      rooks_score_black = evaluateRook < BLACK, MIDDLE > (  );
      rooks_score_white = evaluateRook < WHITE, MIDDLE > (  );

      knights_score_black = evaluateKnight < BLACK, MIDDLE > (  );
      knights_score_white = evaluateKnight < WHITE, MIDDLE > (  );

      kings_score_black = evaluateKing < MIDDLE > ( BLACK );
      kings_score_white = evaluateKing < MIDDLE > ( WHITE );
    }
  }

  int attack_king_white = ATTACK_KING * bitCount ( structure.kingAttackers[BLACK] );
  int attack_king_black = ATTACK_KING * bitCount ( structure.kingAttackers[WHITE] );

  int result = ( attack_king_black + bonus_attack_black_king + lazyscore_black + pawns_score_black + knights_score_black + bishop_score_black + rooks_score_black + queens_score_black + kings_score_black )
    - ( attack_king_white + bonus_attack_white_king + lazyscore_white + pawns_score_white + knights_score_white + bishop_score_white + rooks_score_white + queens_score_white + kings_score_white );
  if ( side )			//white
    result -= 5;		//5 bonus for the side on move.
  else
    result += 5;

  if ( side )
    result = -result;

  return result;
}
