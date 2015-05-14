#include "Eval.h"
#include "bitmap.h"
Eval::Eval (  ) {
  ATTACK_KING = 30;
  PINNED_PIECE = 10;
  ATTACK_CENTRE = 5;
  OPEN_FILE_Q = 3;
  FORK_SCORE = 9;
  BONUS2BISHOP = 18;
  MOB = 2;
  KING_TRAPPED = 24;
  KNIGHT_TRAPPED = 14;
  BISHOP_TRAPPED = 9;
  ROOK_TRAPPED = 10;
  QUEEN_TRAPPED = 25;
  CONNECTED_ROOKS = 7;
  ROOK_BLOCKED = 13;
  ROOK_7TH_RANK = 10;
  OPEN_FILE = 10;
  UNDEVELOPED = 9;
  HALF_OPEN_FILE_Q = 3;
  DOUBLED_PAWNS = 5;
  PAWN_IN_RACE = 114;
  PAWN_7H = 32;
  PAWN_CENTRE = 15;
  FRIEND_NEAR_KING = 1;
  ENEMY_NEAR_KING = 2;
  PAWN_ISOLATED = 3;
  SPACE = 1;
  END_OPENING = 6;
  PAWN_NEAR_KING = 2;
  NEAR_xKING = 2;
  BONUS_11 = 1;
  BISHOP_ON_QUEEN = 2;
  ENEMIES_PAWNS_ALL = 8;
  DOUBLED_ISOLATED_PAWNS = 14;
  BACKWARD_PAWN = 2;
  BACKWARD_OPEN_PAWN = 4;
  BISHOP_TRAPPED_DIAG = 35;
  ATTACK_F7_F2 = 11;
  UNPROTECTED_PAWNS = 5;
#ifdef DEBUG_MODE
  N_EVALUATION[0] = 0;
  N_EVALUATION[1] = 0;
#endif

}

Eval::~Eval (  ) {
}


int
Eval::lazyEvalBlack (  ) {
  return bitCount ( chessboard[0] ) * VALUEPAWN + bitCount ( chessboard[2] ) * VALUEROOK + bitCount ( chessboard[4] ) * VALUEBISHOP + bitCount ( chessboard[6] ) * VALUEKNIGHT + bitCount ( chessboard[10] ) * VALUEQUEEN;
}

int
Eval::lazyEvalWhite (  ) {

  return bitCount ( chessboard[1] ) * VALUEPAWN + bitCount ( chessboard[3] ) * VALUEROOK + bitCount ( chessboard[5] ) * VALUEBISHOP + bitCount ( chessboard[7] ) * VALUEKNIGHT + bitCount ( chessboard[11] ) * VALUEQUEEN;
}

void
Eval::openColumn ( const int side ) {
  int o;
  u64 side_rooks = structure.rooks[side];
  structure.openColumn = 0;
  structure.semiOpenColumn[side] = 0;
  while ( side_rooks ) {
    o = BITScanForward ( side_rooks );
    if ( !( VERTICAL[o] & ( structure.pawns[WHITE] | structure.pawns[BLACK] ) ) )
      structure.openColumn |= VERTICAL[o];
    else if ( VERTICAL[o] & structure.pawns[side ^ 1] )
      structure.semiOpenColumn[side] |= VERTICAL[o];
    side_rooks &= NOTTABLOG[o];
  }
}

int
Eval::evaluatePawn ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  u64 ped_friends = structure.pawns[side];
#ifdef TUNE_CRAFTY_MODE
  structure.passed_pawn_score[side] = 0;
#endif
  if ( !ped_friends )
    return 0;
  int result = 0;
  structure.isolated[side] = 0;
  if ( bitCount ( structure.pawns[side ^ 1] ) == 8 )
    result -= ENEMIES_PAWNS_ALL;
  //space - 2/7th
  if ( side == WHITE ) {
    if ( structure.status == OPEN )
      result += PAWN_CENTRE * bitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( bitCount ( ped_friends & 0xFF000000000000ULL ) );
      result += PAWN_IN_RACE * bitCount ( 0xFF00000000000000ULL & ( ( ( ped_friends << 8 ) ) & ( ~structure.allPiecesSide[BLACK] ) ) );
    }
#ifdef TUNE_CRAFTY_MODE
    structure.race_pawn_score[side] = PAWN_IN_RACE * bitCount ( 0xFF00000000000000ULL & ( ( ped_friends << 8 ) & ( ~structure.allPiecesSide[BLACK] ) ) );
#endif
  }
  else {
    if ( structure.status == OPEN )
      result += PAWN_CENTRE * bitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( bitCount ( ped_friends & 0xFF00ULL ) );
      result += PAWN_IN_RACE * bitCount ( 0xFFULL & ( ( ped_friends >> 8 ) & ( ~structure.allPiecesSide[BLACK] ) ) );
    }
#ifdef TUNE_CRAFTY_MODE
    structure.race_pawn_score[side] = PAWN_IN_RACE * bitCount ( 0xFFULL & ( ( ( ped_friends >> 8 ) ) & ( ~structure.allPiecesSide[BLACK] ) ) );
#endif
  }
  int o;
  u64 p = ped_friends;
  while ( p ) {
    o = BITScanForward ( p );
    if ( structure.status != OPEN && structure.kingAttacked[o] ) {
      result += ATTACK_KING * 2;
      structure.attacKingCount[side ^ 1]++;
    }
    //result +=VALUEPAWN;done in lazyeval
    if ( structure.status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, side ) ) {
	result -= PINNED_PIECE;
      }
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side]][o] ) );
      structure.kingSecurityDistance[side ^ 1] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side ^ 1]][o] ) );
    }

    //unprotected
    if ( !( ped_friends & PROTECTED_MASK[side][o] ) ) {
      result -= UNPROTECTED_PAWNS;
    };
    //isolated
    if ( !( ped_friends & ISOLATED_MASK[o] ) ) {
      result -= PAWN_ISOLATED;
      structure.isolated[side] |= TABLOG[o];
    }
    //doubled
    if ( NOTTABLOG[o] & VERTICAL[o] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( structure.isolated[side] & TABLOG[o] ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    //backward
    if ( !( ped_friends & BACKWARD_MASK[side][o] ) ) {
      result -= BACKWARD_PAWN;
    }
    //fork
    if ( bitCount ( structure.allPiecesSide[side ^ 1] & structure.mobility[o] ) == 2 )
      result += FORK_SCORE;
    //passed
    if ( !( structure.pawns[side ^ 1] & PASSED_MASK[side][o] ) ) {
      result += PASSED[side][o];
#ifdef TUNE_CRAFTY_MODE
      structure.passed_pawn_score[side] += PASSED[side][o];
#endif
    }
    //attack centre
    if ( structure.status == OPEN ) {
      result += ATTACK_CENTRE * bitCount ( structure.mobility[o] & CENTER_MASK );
    }
    p &= NOTTABLOG[o];
  }
  return result;
}

int
Eval::evaluateBishop ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  u64 x = chessboard[BISHOP_BLACK + side];
  if ( !x )
    return 0;
  int o, result = 0, mob;
  if ( structure.status == FINAL && bitCount ( x ) > 1 )
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
    if ( structure.status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, BISHOP_BLACK + side ) ) {
	result -= PINNED_PIECE;
      }
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side]][o] ) );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side ^ 1]][o] ) );
    }
    if ( side ) {
      if ( structure.status == OPEN && ( o == C1 || o == F1 ) )
	result -= UNDEVELOPED;
    }
    else {
      if ( structure.status == OPEN && ( o == C8 || o == F8 ) )
	result -= UNDEVELOPED;
    }
    if ( structure.status != OPEN && structure.kingAttacked[o] ) {
      result += ATTACK_KING;
      structure.attacKingCount[side ^ 1]++;
    }
    if ( !( mob = bitCount ( structure.mobility[o] ) ) )
      result -= BISHOP_TRAPPED;
    else {
      if ( !( BIG_DIAG_LEFT[o] & structure.allPieces ) )
	result += OPEN_FILE;
      if ( !( BIG_DIAG_RIGHT[o] & structure.allPieces ) )
	result += OPEN_FILE;
      result += MOB * mob;
      result += bitCount ( structure.allPieces & structure.mobility[o] );
      if ( structure.status != OPEN && NEAR_MASK[structure.posKing[side]] & structure.mobility[o] ) {
	result += NEAR_xKING;
      }
    }
    //attack centre
    if ( structure.status == OPEN ) {
      result += ATTACK_CENTRE * bitCount ( structure.mobility[o] & CENTER_MASK );
    }
    x &= NOTTABLOG[o];
  };
  return result;
}

int
Eval::evaluateQueen ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  int o, mob, result = 0;
  u64 queen = chessboard[QUEEN_BLACK + side];
  while ( queen ) {
    o = BITScanForward ( queen );
    //result+=VALUEQUEEN;done in lazyeval
    if ( structure.status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, QUEEN_BLACK + side ) ) {
	result -= PINNED_PIECE * 3;
      }
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side]][o] ) );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side ^ 1]][o] ) );
    }
    if ( ( structure.pawns[side ^ 1] & VERTICAL[o] ) )
      result += HALF_OPEN_FILE_Q;
    if ( ( VERTICAL[o] & structure.allPieces ) == TABLOG[o] )
      result += OPEN_FILE_Q;
    if ( structure.kingAttacked[o] ) {
      result += ATTACK_KING;
      structure.attacKingCount[side ^ 1]++;
    }
    mob = bitCount ( structure.mobility[o] );
    if ( structure.status != OPEN && !mob )
      result -= QUEEN_TRAPPED;
    else {
      result += MOB * mob;
      result += bitCount ( structure.allPiecesSide[side ^ 1] & structure.mobility[o] );
    }
    if ( LEFT_RIGHT[o] & chessboard[BISHOP_BLACK + side] )
      result += BISHOP_ON_QUEEN;
    //attack centre
    if ( structure.status == OPEN ) {
      result += ATTACK_CENTRE * bitCount ( structure.mobility[o] & CENTER_MASK );
    }
    queen &= NOTTABLOG[o];
  };
  return result;
}

int
Eval::evaluateKnight ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  int mob, o, result = 0;
  u64 x;
  x = chessboard[KNIGHT_BLACK + side];
  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEKNIGHT;//done in lazyeval
    if ( structure.status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, KNIGHT_BLACK + side ) ) {
	result -= PINNED_PIECE;
      }
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side]][o] ) );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side ^ 1]][o] ) );
    }
    /* Don't block in central pawns */
    if ( side ) {
      if ( structure.status == OPEN && ( o == B1 || o == G1 ) )
	result -= UNDEVELOPED;
    }
    else {
      if ( structure.status == OPEN && ( o == B8 || o == G8 ) )
	result -= UNDEVELOPED;
    }
    if ( structure.status != OPEN && structure.kingAttacked[o] ) {
      result += ATTACK_KING;
      structure.attacKingCount[side ^ 1]++;
    }
    if ( !( mob = bitCount ( structure.mobility[o] ) ) )
      result -= KNIGHT_TRAPPED;
    else {
      result += MOB * mob;
      result += bitCount ( structure.allPiecesSide[side ^ 1] & structure.mobility[o] );
    }
    //attack centre
    if ( structure.status == OPEN ) {
      result += ATTACK_CENTRE * bitCount ( structure.mobility[o] & CENTER_MASK );
    }
    x &= NOTTABLOG[o];
  };
  return result;
}

int
Eval::evaluateRook ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  int rook1 = -1, rook2 = -1, mob, o, result = 0;
  u64 x;
  if ( !( x = chessboard[ROOK_BLACK + side] ) )
    return 0;
  if ( structure.status == MIDDLE ) {
    if ( !side && ( o = bitCount ( x & ORIZZONTAL_8 ) ) )
      result += ROOK_7TH_RANK * o;
    if ( side && ( o = bitCount ( x & ORIZZONTAL_48 ) ) )
      result += o * ROOK_7TH_RANK;
  }
  while ( x ) {
    o = BITScanForward ( x );

    if ( rook1 == -1 )
      rook1 = o;
    else
      rook2 = o;
    //result +=VALUEROOK;done in lazyeval
    if ( structure.status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, ROOK_BLACK + side ) ) {
	result -= PINNED_PIECE * 2;
      }
      structure.kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side]][o] ) );
      structure.kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure.posKing[side ^ 1]][o] ) );
    }
    if ( structure.status != OPEN && structure.kingAttacked[o] ) {
      result += ATTACK_KING;
      structure.attacKingCount[side ^ 1]++;
    }
    if ( !( mob = bitCount ( structure.mobility[o] ) ) )
      result -= ROOK_TRAPPED;
    else {
      result += mob * MOB;
      if ( mob > 11 )
	result += BONUS_11;
    }
    if ( !( structure.pawns[side] & VERTICAL[o] ) )
      result += OPEN_FILE;
    if ( !( structure.pawns[side ^ 1] & VERTICAL[o] ) )
      result += OPEN_FILE;
    /* Penalise if Rook is Blocked Horizontally */
    if ( structure.status != OPEN && ( ( ORIZZ_BOUND[o] & structure.allPieces ) == ORIZZ_BOUND[o] ) ) {
      result -= ROOK_BLOCKED;
    };
    x &= NOTTABLOG[o];
  };
  if ( rook1 != -1 && rook2 != -1 )
    if ( ( !( LINK_ROOKS[rook1][rook2] & structure.allPieces ) ) ) {
      result += CONNECTED_ROOKS;
    }
  return result;
}

int
Eval::evaluateKing ( const int side ) {
#ifdef DEBUG_MODE
  if ( N_EVALUATION[side] != 6 )
    ASSERT ( 0 );
#endif
  int result = 0;

  u64 pos_king = structure.posKing[side];
  if ( structure.status == FINAL )
    result = SPACE * ( DISTANCE_KING_CLOSURE[pos_king] );
  else
    result = SPACE * ( DISTANCE_KING_OPENING[pos_king] );

  u64 tablog_king = TABLOG[pos_king];
  if ( structure.status != OPEN ) {

    if ( ( structure.openColumn & tablog_king ) || ( structure.semiOpenColumn[side ^ 1] & tablog_king ) ) {
      result -= END_OPENING;
      if ( bitCount ( ORIZZONTAL[pos_king] ) < 4 )
	result -= END_OPENING;
    }
  }
  int y;
  ASSERT ( pos_king < 64 );
  if ( !( NEAR_MASK[pos_king] & structure.pawns[side] ) )
    result -= PAWN_NEAR_KING;
  if ( structure.status != OPEN ) {
    if ( !( y = bitCount ( structure.mobility[pos_king] ) ) )
      result -= KING_TRAPPED;
    else
      result += MOB * y;
  }
  result += structure.kingSecurityDistance[side];
  return result;
}

#ifdef TUNE_CRAFTY_MODE
int
Eval::getScore ( const int side, int *material_score, int *pawns_score, int *passed_pawns_score, int *knights_score, int *bishop_score, int *rooks_score, int *queens_score, int *kings_score, int *development_score, int *pawn_race_score, int *total_score )
#else
int
Eval::getScore ( const int side, const int alpha, const int beta )
#endif
{
  int lazyscore_white = lazyEvalWhite (  );
  int lazyscore_black = lazyEvalBlack (  );
#ifndef TUNE_CRAFTY_MODE
#ifndef NO_FP_MODE
  int lazyscore = lazyscore_black - lazyscore_white;
  if ( side )
    lazyscore = -lazyscore;
  if ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) {
#ifdef DEBUG_MODE
    LazyEvalCuts++;
#endif
    if ( side )			//white
      return lazyscore -= 5;	//5 bonus for the side on move.

    else
      return lazyscore += 5;
  }
#endif
#endif
#ifdef DEBUG_MODE
  N_EVALUATION[0] = N_EVALUATION[1] = 0;
#endif
  int mob_black, mob_white;
  memset ( structure.mobility, 0, sizeof ( structure.mobility ) );
  memset ( structure.kingSecurityDistance, 0, sizeof ( structure.kingSecurityDistance ) );
  memset ( structure.kingAttacked, 0, sizeof ( structure.kingAttacked ) );
  memset ( structure.attacKingCount, 0, sizeof ( structure.attacKingCount ) );
#ifdef TUNE_CRAFTY_MODE
  memset ( structure.race_pawn_score, 0, sizeof ( structure.race_pawn_score ) );
#endif
  int npieces = getNpieces ( WHITE ) + getNpieces ( BLACK );
  if ( npieces < 6 )
    structure.status = FINAL;
  else if ( npieces < 13 )
    structure.status = MIDDLE;
  else
    structure.status = OPEN;
  structure.allPiecesSide[BLACK] = getBitBoard ( BLACK );
  structure.allPiecesSide[WHITE] = getBitBoard ( WHITE );
  structure.allPieces = structure.allPiecesSide[BLACK] | structure.allPiecesSide[WHITE];
  structure.posKing[BLACK] = ( unsigned short ) BITScanForward ( chessboard[KING_BLACK] );
  structure.posKing[WHITE] = ( unsigned short ) BITScanForward ( chessboard[KING_WHITE] );
#ifdef DEBUG_MODE
  N_EVALUATION[BLACK]++;
#endif
  if ( !( mob_black = evaluateMobility ( BLACK ) ) ) {
    return ( side ? _INFINITE : -_INFINITE );
  }
#ifdef DEBUG_MODE
  N_EVALUATION[WHITE]++;
#endif
  if ( !( mob_white = evaluateMobility ( WHITE ) ) ) {
    return ( side ? -_INFINITE : _INFINITE );
  }

  structure.pawns[BLACK] = chessboard[BLACK];
  structure.pawns[WHITE] = chessboard[WHITE];
  structure.queens[WHITE] = chessboard[QUEEN_WHITE];
  structure.queens[BLACK] = chessboard[QUEEN_BLACK];
  structure.rooks[BLACK] = chessboard[ROOK_BLACK];
  structure.rooks[WHITE] = chessboard[ROOK_WHITE];
  openColumn ( WHITE );
  openColumn ( BLACK );

  int pawns_score_black = evaluatePawn ( BLACK );
  int pawns_score_white = evaluatePawn ( WHITE );

  int bishop_score_black = evaluateBishop ( BLACK );
  int bishop_score_white = evaluateBishop ( WHITE );

  int queens_score_black = evaluateQueen ( BLACK );
  int queens_score_white = evaluateQueen ( WHITE );

  int rooks_score_black = evaluateRook ( BLACK );
  int rooks_score_white = evaluateRook ( WHITE );

  int knights_score_black = evaluateKnight ( BLACK );
  int knights_score_white = evaluateKnight ( WHITE );

  int kings_score_black = evaluateKing ( BLACK );
  int kings_score_white = evaluateKing ( WHITE );

  int bonus_attack_black_king = 0;
  int bonus_attack_white_king = 0;
  if ( structure.status != OPEN ) {
    bonus_attack_black_king = BONUS_ATTACK_KING[bitCount ( structure.attacKingCount[WHITE] )];
    bonus_attack_white_king = BONUS_ATTACK_KING[bitCount ( structure.attacKingCount[BLACK] )];
  }
  int result = ( bonus_attack_black_king + lazyscore_black + mob_black + pawns_score_black + knights_score_black + bishop_score_black + rooks_score_black + queens_score_black + kings_score_black )
    - ( bonus_attack_white_king + lazyscore_white + mob_white + pawns_score_white + knights_score_white + bishop_score_white + rooks_score_white + queens_score_white + kings_score_white );
  if ( side )			//white
    result -= 5;		//5 bonus for the side on move.
  else
    result += 5;

  /*cout << "black: " << bonus_attack_black_king << " " << lazyscore_black << " " << mob_black << " " << pawns_score_black << " "
     << knights_score_black << " " << bishop_score_black << " " << rooks_score_black << " " << queens_score_black << " " << kings_score_black
     << " " << castle_score[BLACK] << endl;
     cout << "white: " << bonus_attack_white_king << " " << lazyscore_white << " " << mob_white << " " << pawns_score_white << " "
     << knights_score_white << " " << bishop_score_white << " " << rooks_score_white << " " << queens_score_white << " " << kings_score_white
     << " " << castle_score[WHITE] << endl; */
#ifdef TUNE_CRAFTY_MODE
  *material_score = lazyscore_white - lazyscore_black;
  *pawns_score = pawns_score_white - pawns_score_black;
  *passed_pawns_score = structure.passed_pawn_score[WHITE] - structure.passed_pawn_score[BLACK];
  *knights_score = knights_score_white - knights_score_black;
  *bishop_score = bishop_score_white - bishop_score_black;
  *rooks_score = rooks_score_white - rooks_score_black;
  *queens_score = queens_score_white - queens_score_black;
  *kings_score = kings_score_white - kings_score_black;
  *development_score = -9999999;
  *pawn_race_score = structure.race_pawn_score[WHITE] - structure.race_pawn_score[BLACK];
  *total_score = -result;
#endif
  if ( side )
    result = -result;
  return result;
}

void
Eval::setRandomParam (  ) {
  const int percent = 30;
  int x;
/////PAWN//////
  x = rand (  ) % ENEMIES_PAWNS_ALL * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ENEMIES_PAWNS_ALL = ENEMIES_PAWNS_ALL + x;
  x = rand (  ) % DOUBLED_ISOLATED_PAWNS * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  DOUBLED_ISOLATED_PAWNS = DOUBLED_ISOLATED_PAWNS + x;
  x = rand (  ) % BACKWARD_PAWN * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BACKWARD_PAWN = BACKWARD_PAWN + x;
  x = rand (  ) % BACKWARD_OPEN_PAWN * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BACKWARD_OPEN_PAWN = BACKWARD_OPEN_PAWN + x;
  x = rand (  ) % UNPROTECTED_PAWNS * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  UNPROTECTED_PAWNS = UNPROTECTED_PAWNS + x;
  x = rand (  ) % DOUBLED_PAWNS * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  DOUBLED_PAWNS = DOUBLED_PAWNS + x;
  x = rand (  ) % PAWN_IN_RACE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  PAWN_IN_RACE = PAWN_IN_RACE + x;
  x = rand (  ) % PAWN_7H * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  PAWN_7H = PAWN_7H + x;
  x = rand (  ) % PAWN_NEAR_KING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  PAWN_NEAR_KING = PAWN_NEAR_KING + x;
  x = rand (  ) % PAWN_CENTRE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  PAWN_CENTRE = PAWN_CENTRE + x;

/////////END PAWN///////////

  x = rand (  ) % UNDEVELOPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  UNDEVELOPED = UNDEVELOPED + x;
  x = rand (  ) % ATTACK_KING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ATTACK_KING = ATTACK_KING + x;
  x = rand (  ) % OPEN_FILE_Q * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  OPEN_FILE_Q = OPEN_FILE_Q + x;
  x = rand (  ) % FORK_SCORE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  FORK_SCORE = FORK_SCORE + x;
  x = rand (  ) % BONUS2BISHOP * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BONUS2BISHOP = BONUS2BISHOP + x;
  x = rand (  ) % MOB * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  MOB = MOB + x;
  x = rand (  ) % KING_TRAPPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  KING_TRAPPED = KING_TRAPPED + x;
  x = rand (  ) % KNIGHT_TRAPPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  KNIGHT_TRAPPED = KNIGHT_TRAPPED + x;
  x = rand (  ) % BISHOP_TRAPPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BISHOP_TRAPPED = BISHOP_TRAPPED + x;
  x = rand (  ) % ROOK_TRAPPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ROOK_TRAPPED = ROOK_TRAPPED + x;
  x = rand (  ) % QUEEN_TRAPPED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  QUEEN_TRAPPED = QUEEN_TRAPPED + x;
  x = rand (  ) % CONNECTED_ROOKS * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  CONNECTED_ROOKS = CONNECTED_ROOKS + x;
  x = rand (  ) % ROOK_BLOCKED * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ROOK_BLOCKED = ROOK_BLOCKED + x;
  x = rand (  ) % ROOK_7TH_RANK * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ROOK_7TH_RANK = ROOK_7TH_RANK + x;
  x = rand (  ) % OPEN_FILE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  OPEN_FILE = OPEN_FILE + x;
  x = rand (  ) % HALF_OPEN_FILE_Q * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  HALF_OPEN_FILE_Q = HALF_OPEN_FILE_Q + x;
  x = rand (  ) % FRIEND_NEAR_KING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  FRIEND_NEAR_KING = FRIEND_NEAR_KING + x;
  x = rand (  ) % ENEMY_NEAR_KING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ENEMY_NEAR_KING = ENEMY_NEAR_KING + x;
  x = rand (  ) % SPACE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  SPACE = SPACE + x;
  x = rand (  ) % END_OPENING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  END_OPENING = END_OPENING + x;
  x = rand (  ) % NEAR_xKING * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  NEAR_xKING = NEAR_xKING + x;
  x = rand (  ) % BONUS_11 * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BONUS_11 = BONUS_11 + x;
  x = rand (  ) % BISHOP_ON_QUEEN * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BISHOP_ON_QUEEN = BISHOP_ON_QUEEN + x;
  x = rand (  ) % BISHOP_TRAPPED_DIAG * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BISHOP_TRAPPED_DIAG = BISHOP_TRAPPED_DIAG + x;
  x = rand (  ) % ATTACK_F7_F2 * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  ATTACK_F7_F2 = ATTACK_F7_F2 + x;
}

void
Eval::writeParam ( string param_file, int cd_param, bool append ) {
  ofstream fil ( param_file, append ? ios::app : ios::out );
  fil << "#------------------------- param " << cd_param << " -------------------------" << endl;
  fil << "ATTACK_KING=" << ATTACK_KING << ";" << endl;
  fil << "OPEN_FILE_Q=" << OPEN_FILE_Q << ";" << endl;
  fil << "FORK_SCORE=" << FORK_SCORE << ";" << endl;
  fil << "BONUS2BISHOP=" << BONUS2BISHOP << ";" << endl;
  fil << "MOB=" << MOB << ";" << endl;
  fil << "KING_TRAPPED=" << KING_TRAPPED << ";" << endl;
  fil << "KNIGHT_TRAPPED=" << KNIGHT_TRAPPED << ";" << endl;
  fil << "BISHOP_TRAPPED=" << BISHOP_TRAPPED << ";" << endl;
  fil << "ROOK_TRAPPED=" << ROOK_TRAPPED << ";" << endl;
  fil << "QUEEN_TRAPPED=" << QUEEN_TRAPPED << ";" << endl;
  fil << "CONNECTED_ROOKS=" << CONNECTED_ROOKS << ";" << endl;
  fil << "ROOK_BLOCKED=" << ROOK_BLOCKED << ";" << endl;
  fil << "ROOK_7TH_RANK=" << ROOK_7TH_RANK << ";" << endl;
  fil << "OPEN_FILE=" << OPEN_FILE << ";" << endl;
  fil << "UNDEVELOPED=" << UNDEVELOPED << ";" << endl;
  fil << "HALF_OPEN_FILE_Q=" << HALF_OPEN_FILE_Q << ";" << endl;
  fil << "DOUBLED_PAWNS=" << DOUBLED_PAWNS << ";" << endl;
  fil << "PAWN_IN_RACE=" << PAWN_IN_RACE << ";" << endl;
  fil << "PAWN_7H=" << PAWN_7H << ";" << endl;
  fil << "PAWN_CENTRE=" << PAWN_CENTRE << ";" << endl;
  fil << "FRIEND_NEAR_KING=" << FRIEND_NEAR_KING << ";" << endl;
  fil << "ENEMY_NEAR_KING=" << ENEMY_NEAR_KING << ";" << endl;
  fil << "PAWN_ISOLATED=" << PAWN_ISOLATED << ";" << endl;
  fil << "SPACE=" << SPACE << ";" << endl;
  fil << "END_OPENING=" << END_OPENING << ";" << endl;
  fil << "PAWN_NEAR_KING=" << PAWN_NEAR_KING << ";" << endl;
  fil << "NEAR_xKING=" << NEAR_xKING << ";" << endl;
  fil << "BONUS_11=" << BONUS_11 << ";" << endl;
  fil << "BISHOP_ON_QUEEN=" << BISHOP_ON_QUEEN << ";" << endl;
  fil << "ENEMIES_PAWNS_ALL=" << ENEMIES_PAWNS_ALL << ";" << endl;
  fil << "DOUBLED_ISOLATED_PAWNS=" << DOUBLED_ISOLATED_PAWNS << ";" << endl;
  fil << "BACKWARD_PAWN=" << BACKWARD_PAWN << ";" << endl;
  fil << "BACKWARD_OPEN_PAWN=" << BACKWARD_OPEN_PAWN << ";" << endl;
  fil << "BISHOP_TRAPPED_DIAG=" << BISHOP_TRAPPED_DIAG << ";" << endl;
  fil << "ATTACK_F7_F2=" << ATTACK_F7_F2 << ";" << endl;
  fil << "UNPROTECTED_PAWNS=" << UNPROTECTED_PAWNS << ";" << endl;
  fil.close (  );
}

int
Eval::isPinned ( const int side, const uchar Position, const uchar piece ) {
  ASSERT ( structure.status != OPEN );
  int xside = side ^ 1;
  u64 king = chessboard[KING_BLACK + side];
  int result = -1;
  uchar Position_mod_8 = ROT45[Position];
  if ( king & ORIZZONTAL[Position] ) {
    int Position_Position_mod_8 = pos_posMod8[Position];
    chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[( ( ( ( structure.allPieces & ORIZZONTAL[Position] ) >> Position_Position_mod_8 ) ) )][Position_mod_8]
	 & ( ( ( chessboard[ROOK_BLACK + xside] >> Position_Position_mod_8 ) & 255 )
	     | ( ( chessboard[QUEEN_BLACK + xside] >> Position_Position_mod_8 ) & 255 ) ) ) {
      result = 1;
    }
  }
  else if ( king & VERTICAL[Position] ) {
    chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[rotateBoard90 ( ( structure.allPieces & VERTICAL[Position] ) & VERTICAL[Position] )][ROT45ROT_90_MASK[Position]]
	 & ( ( rotateBoard90 ( chessboard[ROOK_BLACK + xside]
			       & VERTICAL[Position] ) | rotateBoard90 ( chessboard[QUEEN_BLACK + xside] & VERTICAL[Position] ) ) ) ) {
      result = 1;
    }
  }
  else if ( king & LEFT[Position] ) {
    chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[rotateBoardLeft45 ( ( structure.allPieces & LEFT[Position] ), Position )][Position_mod_8]
	 & ( rotateBoardLeft45 ( chessboard[BISHOP_BLACK + ( xside )], Position )
	     | rotateBoardLeft45 ( chessboard[QUEEN_BLACK + xside], Position ) ) ) {
      result = 1;
    }
  }
  else if ( king & RIGHT[Position] ) {
    chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[rotateBoardRight45 ( ( structure.allPieces & RIGHT[Position] ), Position ) | TABLOG[Position_mod_8]][Position_mod_8]
	 & ( rotateBoardRight45 ( chessboard[BISHOP_BLACK + ( xside )], Position )
	     | rotateBoardRight45 ( chessboard[QUEEN_BLACK + ( xside )], Position ) ) ) {
      result = 1;
    }
  }
  if ( result == -1 )
    return 0;
  chessboard[piece] |= TABLOG[Position];
  return result;
}
