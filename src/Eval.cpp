#include "Eval.h"

Eval::Eval (  ) {
}

Eval::~Eval (  ) {
}

void
Eval::openColumn ( int side ) {
  u64 side_rooks = structure.rooks[side];
  structure.openColumn = 0;
  structure.semiOpenColumn[side] = 0;
  while ( side_rooks ) {
    int o = BITScanForward ( side_rooks );
    if ( !( FILE_[o] & ( structure.pawns[WHITE] | structure.pawns[BLACK] ) ) )
      structure.openColumn |= FILE_[o];
    else if ( FILE_[o] & structure.pawns[side ^ 1] )
      structure.semiOpenColumn[side] |= FILE_[o];
    side_rooks &= NOTPOW2[o];
  }
}

template < int side, _Tstatus status > int
Eval::evaluatePawn (  ) {
  INC ( evaluationCount[side] );
  u64 ped_friends = structure.pawns[side];
  if ( !ped_friends )
    return -NO_PAWNS;

  structure.isolated[side] = 0;
  //ASSERT(getMobilityPawns<side>(  enpassantPosition,  ped_friends, side==WHITE?structure.allPiecesSide[BLACK]:structure.allPiecesSide[WHITE],~structure.allPiecesSide[BLACK]|~structure.allPiecesSide[WHITE])<(int)(sizeof(MOB_PAWNS)/sizeof(int)));
  int result = MOB_PAWNS[getMobilityPawns ( side, enpassantPosition, ped_friends, side == WHITE ? structure.allPiecesSide[BLACK] : structure.allPiecesSide[WHITE], ~structure.allPiecesSide[BLACK] | ~structure.allPiecesSide[WHITE] )];

  if ( bitCount ( structure.pawns[side ^ 1] ) == 8 )
    result -= ENEMIES_PAWNS_ALL;
  result += ATTACK_KING * bitCount ( ped_friends & structure.kingAttackers[side ^ 1] );

  //space

  if ( status == OPEN )
    result += PAWN_CENTER * bitCount ( ped_friends & CENTER_MASK );
  else {
    result += PAWN_7H * ( bitCount ( ped_friends & PAWNS_7_2[side] ) );

  }

  u64 p = ped_friends;
  while ( p ) {
    int o = BITScanForward ( p );
    u64 pos = POW2[o];
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, side))
      //  result -= PINNED_PIECE;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & pos ? 1 : 0 );
      structure.kingSecurityDistance[side ^ 1] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & pos ? 1 : 0 );
      ///  pawn in race
      if ( PAWNS_7_2[side] & pos ) {
	if ( ( ( shiftForward < side, 8 > ( pos ) & ( ~structure.allPieces ) ) | ( structure.allPiecesSide[side ^ 1] & ( shiftForward < side, 9 > ( pos ) | shiftForward < side, 7 > ( pos ) ) ) )
	     & PAWNS_8_1[side] ) {
	  result += PAWN_IN_RACE;
	}
      }
    }
    /// blocked
    result -= ( !( PAWN_FORK_MASK[side][o] & structure.allPiecesSide[side ^ 1] ) ) && ( structure.allPieces & ( shiftForward < side, 8 > ( pos ) ) ) ? PAWN_BLOCKED : 0;

    //result +=VALUEPAWN;done in lazyeval

    /// unprotected
    if ( !( ped_friends & PAWN_PROTECTED_MASK[side][o] ) ) {
      result -= UNPROTECTED_PAWNS;
    };
    //isolated
    if ( !( ped_friends & PAWN_ISOLATED_MASK[o] ) ) {
      result -= PAWN_ISOLATED;
      structure.isolated[side] |= pos;
    }
    /// doubled
    if ( NOTPOW2[o] & FILE_[o] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( structure.isolated[side] & pos ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    /// backward
    if ( !( ped_friends & PAWN_BACKWARD_MASK[side][o] ) ) {
      result -= BACKWARD_PAWN;
    }
    /// fork
    /*        if (bitCount(structure.allPiecesSide[side ^ 1] & structure.mobility[o]) == 2)
       result += FORK_SCORE; */
    /// passed
    if ( !( structure.pawns[side ^ 1] & PAWN_PASSED_MASK[side][o] ) ) {
      result += PAWN_PASSED[side][o];
    }

    p &= NOTPOW2[o];
  }
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateBishop ( const u64 enemiesPawns, u64 enemies, u64 friends ) {
  INC ( evaluationCount[side] );
  u64 x = chessboard[BISHOP_BLACK + side];
  if ( !x )
    return 0;
  int result = 0;
  if ( status == END && bitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  // Check to see if the bishop is trapped at a7 or h7 with a pawn at b6 or g6 that has trapped the bishop.
  if ( side ) {
    if ( ( A6bit & x && B5bit & enemiesPawns ) || ( H6bit & x && G5bit & enemiesPawns ) )
      result -= BISHOP_TRAPPED;
  }
  else {
    if ( ( A3bit & x && B4bit & enemiesPawns ) || ( H3bit & x && G4bit & enemiesPawns ) )
      result -= BISHOP_TRAPPED;
  }
  while ( x ) {
    int o = BITScanForward ( x );
    //result+=VALUEBISHOP;done in lazyeval

    ASSERT ( getMobilityBishop ( o, enemies, friends ) < ( int ) ( sizeof ( MOB_BISHOP ) / sizeof ( int ) ) );
    result += MOB_BISHOP[getMobilityBishop ( o, enemies, friends )];
    if ( status != OPEN ) {
      //pinned
      //if (isPinned(side, o, BISHOP_BLACK + side))
      //  result -= PINNED_PIECE;

      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[o] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[o] ? 1 : 0 );
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
    else {
      if ( BIG_DIAG_LEFT & POW2[o] && !( LEFT_DIAG[o] & structure.allPieces ) )
	result += OPEN_FILE;
      if ( BIG_DIAG_RIGHT & POW2[o] && !( RIGHT_DIAG[o] & structure.allPieces ) )
	result += OPEN_FILE;
    }
    x &= NOTPOW2[o];
  };
  return result;
}

template < _Tstatus status > int
Eval::evaluateQueen ( int side, u64 enemies, u64 friends ) {
  INC ( evaluationCount[side] );
  int result = 0;
  u64 queen = chessboard[QUEEN_BLACK + side];
  while ( queen ) {
    int o = BITScanForward ( queen );
    //result+=VALUEQUEEN;done in lazyeval
    ASSERT ( getMobilityQueen ( o, enemies, friends ) < ( int ) ( sizeof ( MOB_QUEEN[status] ) / sizeof ( int ) ) );
    result += MOB_QUEEN[status][getMobilityQueen ( o, enemies, friends )];
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
    if ( LEFT_RIGHT_DIAG[o] & chessboard[BISHOP_BLACK + side] )
      result += BISHOP_ON_QUEEN;
    queen &= NOTPOW2[o];
  };
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateKnight ( const u64 enemiesPawns, const u64 squares ) {
  INC ( evaluationCount[side] );
  int result = 0;

  u64 x = chessboard[KNIGHT_BLACK + side];
  if ( status == OPEN ) {
    side ? result -= bitCount ( x & 0x42ULL ) * UNDEVELOPED : result -= bitCount ( x & 0x4200000000000000ULL ) * UNDEVELOPED;
  }
  if ( side == WHITE ) {
    if ( ( A7bit & x ) && ( B7bit & enemiesPawns ) && ( C6A6bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( H7bit & x ) && ( G7bit & enemiesPawns ) && ( F6H6bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( A8bit & x ) && ( A7C7bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( H8bit & x ) && ( H7G7bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
  }
  else {
    if ( ( A2bit & x ) && ( B2bit & enemiesPawns ) && ( C3A3bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( H2bit & x ) && ( G2bit & enemiesPawns ) && ( F3H3bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( A1bit & x ) && ( A2C2bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
    if ( ( H1bit & x ) && ( H2G2bit & enemiesPawns ) )
      result -= KNIGHT_TRAPPED;
  }
  while ( x ) {
    int pos = BITScanForward ( x );
    //result+=VALUEKNIGHT;//done in lazyeval
    if ( status != OPEN ) {
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( NEAR_MASK2[structure.posKing[side]] & POW2[pos] ? 1 : 0 );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( NEAR_MASK2[structure.posKing[side ^ 1]] & POW2[pos] ? 1 : 0 );
    }
    //mobility
    ASSERT ( _bits::bitCount ( squares & KNIGHT_MASK[pos] ) < ( int ) ( sizeof ( MOB_KNIGHT ) / sizeof ( int ) ) );
    result += MOB_KNIGHT[_bits::bitCount ( squares & KNIGHT_MASK[pos] )];

    x &= NOTPOW2[pos];
  };
  return result;
}

template < int side, _Tstatus status > int
Eval::evaluateRook ( const u64 king, u64 enemies, u64 friends ) {
  INC ( evaluationCount[side] );
  int o, result = 0;
  u64 x = chessboard[ROOK_BLACK + side];
  if ( !x )
    return 0;
  if ( status == MIDDLE ) {
    if ( !side && ( o = bitCount ( x & RANK_1 ) ) )
      result += ROOK_7TH_RANK * o;
    if ( side && ( o = bitCount ( x & RANK_6 ) ) )
      result += o * ROOK_7TH_RANK;
  }


  if ( side == WHITE ) {
    if ( ( ( F1G1bit & king ) && ( H1H2G1bit & x ) ) || ( ( C1B1bit & king ) && ( A1A2B1bit & x ) ) )
      result -= ROOK_TRAPPED;

  }
  else {
    if ( ( ( F8G8bit & king ) && ( H8H7G8bit & x ) ) || ( ( C8B8bit & king ) && ( A8A7B8bit & x ) ) )
      result -= ROOK_TRAPPED;
  }
  int firstRook = -1;
  int secondRook = -1;
  while ( x ) {
    o = BITScanForward ( x );
    //mobility
    ASSERT ( getMobilityRook ( o, enemies, friends ) < ( int ) ( sizeof ( MOB_ROOK[status] ) / sizeof ( int ) ) );
    result += MOB_ROOK[status][getMobilityRook ( o, enemies, friends )];
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
      /* Penalise if Rook is Blocked Horizontally */
      if ( ( RANK_BOUND[o] & structure.allPieces ) == RANK_BOUND[o] ) {
	result -= ROOK_BLOCKED;
      };
    }
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
Eval::evaluateKing ( int side, u64 squares ) {
  ASSERT ( evaluationCount[side] == 5 );
  int result = 0;
  u64 pos_king = structure.posKing[side];
  if ( status == END )
    result = DISTANCE_KING_ENDING[pos_king];
  else
    result = DISTANCE_KING_OPENING[pos_king];

  u64 POW2_king = POW2[pos_king];
  //mobility
  ASSERT ( _bits::bitCount ( squares & NEAR_MASK1[pos_king] ) < ( int ) ( sizeof ( MOB_KING[status] ) / sizeof ( int ) ) );
  result += MOB_KING[status][_bits::bitCount ( squares & NEAR_MASK1[pos_king] )];
  if ( status != OPEN ) {
    if ( ( structure.openColumn & POW2_king ) || ( structure.semiOpenColumn[side ^ 1] & POW2_king ) ) {
      result -= END_OPENING;
      if ( bitCount ( RANK[pos_king] ) < 4 )
	result -= END_OPENING;
    }
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
  int lazyscore = lazyscore_black - lazyscore_white;
  if ( side )
    lazyscore = -lazyscore;
  if ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) {
    INC ( LazyEvalCuts );
    return lazyscore;
  }
#ifdef DEBUG_MODE
  evaluationCount[WHITE] = evaluationCount[BLACK] = 0;
#endif
  memset ( structure.kingSecurityDistance, 0, sizeof ( structure.kingSecurityDistance ) );
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
  structure.kingAttackers[WHITE] = getKingAttackers ( BLACK, structure.allPieces, structure.posKing[WHITE] );
  structure.kingAttackers[BLACK] = getKingAttackers ( WHITE, structure.allPieces, structure.posKing[BLACK] );

  structure.pawns[BLACK] = chessboard[BLACK];
  structure.pawns[WHITE] = chessboard[WHITE];
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

    bishop_score_black = evaluateBishop < BLACK, OPEN > ( structure.pawns[WHITE], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
    bishop_score_white = evaluateBishop < WHITE, OPEN > ( structure.pawns[BLACK], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

    queens_score_black = evaluateQueen < OPEN > ( BLACK, structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
    queens_score_white = evaluateQueen < OPEN > ( WHITE, structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

    rooks_score_black = evaluateRook < BLACK, OPEN > ( structure.posKing[BLACK], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
    rooks_score_white = evaluateRook < WHITE, OPEN > ( structure.posKing[WHITE], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

    knights_score_black = evaluateKnight < BLACK, OPEN > ( structure.pawns[WHITE], ~structure.allPiecesSide[BLACK] );
    knights_score_white = evaluateKnight < WHITE, OPEN > ( structure.pawns[BLACK], ~structure.allPiecesSide[WHITE] );

    kings_score_black = evaluateKing < OPEN > ( BLACK, ~structure.allPiecesSide[BLACK] );
    kings_score_white = evaluateKing < OPEN > ( WHITE, ~structure.allPiecesSide[WHITE] );
  }
  else {
    bonus_attack_black_king = BONUS_ATTACK_KING[bitCount ( structure.kingAttackers[WHITE] )];
    bonus_attack_white_king = BONUS_ATTACK_KING[bitCount ( structure.kingAttackers[BLACK] )];
    if ( status == END ) {
      pawns_score_black = evaluatePawn < BLACK, END > (  );
      pawns_score_white = evaluatePawn < WHITE, END > (  );

      bishop_score_black = evaluateBishop < BLACK, END > ( structure.pawns[WHITE], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      bishop_score_white = evaluateBishop < WHITE, END > ( structure.pawns[BLACK], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      queens_score_black = evaluateQueen < END > ( BLACK, structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      queens_score_white = evaluateQueen < END > ( WHITE, structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      rooks_score_black = evaluateRook < BLACK, END > ( structure.posKing[BLACK], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      rooks_score_white = evaluateRook < WHITE, END > ( structure.posKing[WHITE], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      knights_score_black = evaluateKnight < BLACK, END > ( structure.pawns[WHITE], ~structure.allPiecesSide[BLACK] );
      knights_score_white = evaluateKnight < WHITE, END > ( structure.pawns[BLACK], ~structure.allPiecesSide[WHITE] );

      kings_score_black = evaluateKing < END > ( BLACK, ~structure.allPiecesSide[BLACK] );
      kings_score_white = evaluateKing < END > ( WHITE, ~structure.allPiecesSide[WHITE] );
    }
    else {
      pawns_score_black = evaluatePawn < BLACK, MIDDLE > (  );
      pawns_score_white = evaluatePawn < WHITE, MIDDLE > (  );

      bishop_score_black = evaluateBishop < BLACK, MIDDLE > ( structure.pawns[WHITE], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      bishop_score_white = evaluateBishop < WHITE, MIDDLE > ( structure.pawns[BLACK], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      queens_score_black = evaluateQueen < MIDDLE > ( BLACK, structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      queens_score_white = evaluateQueen < MIDDLE > ( WHITE, structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      rooks_score_black = evaluateRook < BLACK, MIDDLE > ( structure.posKing[BLACK], structure.allPiecesSide[WHITE], structure.allPiecesSide[BLACK] );
      rooks_score_white = evaluateRook < WHITE, MIDDLE > ( structure.posKing[WHITE], structure.allPiecesSide[BLACK], structure.allPiecesSide[WHITE] );

      knights_score_black = evaluateKnight < BLACK, MIDDLE > ( structure.pawns[WHITE], ~structure.allPiecesSide[BLACK] );

      knights_score_white = evaluateKnight < WHITE, MIDDLE > ( structure.pawns[BLACK], ~structure.allPiecesSide[WHITE] );

      kings_score_black = evaluateKing < MIDDLE > ( BLACK, ~structure.allPiecesSide[BLACK] );
      kings_score_white = evaluateKing < MIDDLE > ( WHITE, ~structure.allPiecesSide[WHITE] );
    }
  }
  ASSERT ( getMobilityCastle ( WHITE, structure.allPieces ) < ( int ) ( sizeof ( MOB_CASTLE[status] ) / sizeof ( int ) ) );
  ASSERT ( getMobilityCastle ( BLACK, structure.allPieces ) < ( int ) ( sizeof ( MOB_CASTLE[status] ) / sizeof ( int ) ) );
  int mobWhite = MOB_CASTLE[status][getMobilityCastle ( WHITE, structure.allPieces )];
  int mobBlack = MOB_CASTLE[status][getMobilityCastle ( BLACK, structure.allPieces )];

  int attack_king_white = ATTACK_KING * bitCount ( structure.kingAttackers[BLACK] );
  int attack_king_black = ATTACK_KING * bitCount ( structure.kingAttackers[WHITE] );

  int result = ( mobBlack + attack_king_black + bonus_attack_black_king + lazyscore_black + pawns_score_black + knights_score_black + bishop_score_black + rooks_score_black + queens_score_black + kings_score_black )
    - ( mobWhite + attack_king_white + bonus_attack_white_king + lazyscore_white + pawns_score_white + knights_score_white + bishop_score_white + rooks_score_white + queens_score_white + kings_score_white );

  result += side ? -5 : 5;	//bonus for the side on move.
  if ( side )
    result = -result;

  return result;
}
