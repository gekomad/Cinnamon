#include "Eval.h"
#include "bitmap.h"
Eval::Eval ( char *iniFile, Search * s ) {
  search = s;
  structure = NULL;
  if ( s )
    structure = s->getStructure (  );
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
  BLOCK_PAWNS = 2;
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
  if ( iniFile != NULL )
    readParam ( iniFile );
}

Eval::~Eval (  ) {
}

int
Eval::evaluateMobility ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  search->setEvaluateMobilityMode ( true );
  search->incListId (  );
  search->generateCap ( STANDARD_MOVE_MASK, side );
  search->generateMoves ( STANDARD_MOVE_MASK, side );
  int listcount = search->getListCount (  );
  if ( listcount == -1 )
    listcount = 0;
  search->resetList (  );
  search->decListId (  );
  search->setEvaluateMobilityMode ( false );
  return listcount;
}

int
Eval::lazyEvalBlack (  ) {	//TODO inline
  return BitCount ( search->chessboard[0] ) * VALUEPAWN + BitCount ( search->chessboard[2] ) * VALUEROOK + BitCount ( search->chessboard[4] ) * VALUEBISHOP + BitCount ( search->chessboard[6] ) * VALUEKNIGHT + BitCount ( search->chessboard[10] ) * VALUEQUEEN;
}

int
Eval::lazyEvalWhite (  ) {
  ASSERT ( search );
  return BitCount ( search->chessboard[1] ) * VALUEPAWN + BitCount ( search->chessboard[3] ) * VALUEROOK + BitCount ( search->chessboard[5] ) * VALUEBISHOP + BitCount ( search->chessboard[7] ) * VALUEKNIGHT + BitCount ( search->chessboard[11] ) * VALUEQUEEN;
}

void
Eval::openColumn ( const int side ) {
  int o;
  u64 side_rooks = structure->rooks[side];
  structure->openColumn = 0;
  structure->semiOpenColumn[side] = 0;
  while ( side_rooks ) {
    o = BITScanForward ( side_rooks );
    if ( !( VERTICAL[o] & ( structure->pawns[WHITE] | structure->pawns[BLACK] ) ) )
      structure->openColumn |= VERTICAL[o];
    else if ( VERTICAL[o] & structure->pawns[side ^ 1] )
      structure->semiOpenColumn[side] |= VERTICAL[o];
    side_rooks &= NOTTABLOG[o];
  }
}

int
Eval::evaluatePawn ( const int side ) {
#ifdef DEBUG_MODE
  N_EVALUATION[side]++;
#endif
  u64 ped_friends = structure->pawns[side];
#ifdef TEST_MODE
  structure->passed_pawn_score[side] = 0;
#endif
  if ( !ped_friends )
    return 0;
  int result = 0;
  structure->isolated[side] = 0;
  if ( BitCount ( structure->pawns[side ^ 1] ) == 8 )
    result -= ENEMIES_PAWNS_ALL;
  //space - 2/7th
  if ( side == WHITE ) {
    if ( structure->status == OPEN )
      result += PAWN_CENTRE * BitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( BitCount ( ped_friends & 0xFF000000000000ULL ) );
      result += PAWN_IN_RACE * BitCount ( 0xFF00000000000000ULL & ( ( ( ped_friends << 8 ) ) & ( ~structure->allPiecesSide[BLACK] ) ) );
    }
#ifdef TEST_MODE
    structure->race_pawn_score[side] = PAWN_IN_RACE * BitCount ( 0xFF00000000000000ULL & ( ( ped_friends << 8 ) & ( ~structure->allPiecesSide[BLACK] ) ) );
#endif
  }
  else {
    if ( structure->status == OPEN )
      result += PAWN_CENTRE * BitCount ( ped_friends & CENTER_MASK );
    else {
      result += PAWN_7H * ( BitCount ( ped_friends & 0xFF00ULL ) );
      result += PAWN_IN_RACE * BitCount ( 0xFFULL & ( ( ped_friends >> 8 ) & ( ~structure->allPiecesSide[BLACK] ) ) );
    }
#ifdef TEST_MODE
    structure->race_pawn_score[side] = PAWN_IN_RACE * BitCount ( 0xFFULL & ( ( ( ped_friends >> 8 ) ) & ( ~structure->allPiecesSide[BLACK] ) ) );
#endif
  }
  int o;
  u64 p = ped_friends;
  while ( p ) {
    o = BITScanForward ( p );
    if ( structure->status != OPEN && structure->kingAttacked[o] ) {
      result += ATTACK_KING * 2;
      structure->attacKingCount[side ^ 1]++;
    }
    //result +=VALUEPAWN;done in lazyeval
    if ( structure->status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, side ) ) {
	result -= PINNED_PIECE;
      }
      structure->kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side]][o] ) );
      structure->kingSecurityDistance[side ^ 1] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side ^ 1]][o] ) );
    }

    //unprotected
    if ( !( ped_friends & PROTECTED_MASK[side][o] ) ) {
      result -= UNPROTECTED_PAWNS;
    };
    //isolated
    if ( !( ped_friends & ISOLATED_MASK[o] ) ) {
      result -= PAWN_ISOLATED;
      structure->isolated[side] |= TABLOG[o];
    }
    //doubled
    if ( NOTTABLOG[o] & VERTICAL[o] & ped_friends ) {
      result -= DOUBLED_PAWNS;
      if ( !( structure->isolated[side] & TABLOG[o] ) )
	result -= DOUBLED_ISOLATED_PAWNS;
    };
    //backward
    if ( !( ped_friends & BACKWARD_MASK[side][o] ) ) {
      result -= BACKWARD_PAWN;
    }
    //fork
    if ( BitCount ( structure->allPiecesSide[side ^ 1] & structure->mobility[o] ) == 2 )
      result += FORK_SCORE;
    //passed
    if ( !( structure->pawns[side ^ 1] & PASSED_MASK[side][o] ) ) {
      result += PASSED[side][o];
#ifdef TEST_MODE
      structure->passed_pawn_score[side] += PASSED[side][o];
#endif
    }
    //attack centre
    if ( structure->status == OPEN ) {
      result += ATTACK_CENTRE * BitCount ( structure->mobility[o] & CENTER_MASK );
      //result += ATTACK_CENTRE *BitCount(structure->attacked[oo] & CENTER_BOUND_MASK);

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
  u64 x = search->chessboard[BISHOP_BLACK + side];
  if ( !x )
    return 0;
  int o, result = 0, mob;
  if ( structure->status == FINAL && BitCount ( x ) > 1 )
    result += BONUS2BISHOP;
  // Check to see if the bishop is trapped at a7 or h7 with a pawn at b6 or g6 that has trapped the bishop.
  if ( side ) {
    if ( search->chessboard[PAWN_WHITE] & 0x400000ULL && search->chessboard[BISHOP_WHITE] & 0x8000ULL )
      result -= BISHOP_TRAPPED_DIAG;
    if ( search->chessboard[PAWN_WHITE] & 0x20000ULL && search->chessboard[BISHOP_WHITE] & 0x100ULL )
      result -= BISHOP_TRAPPED_DIAG;
  }
  else {
    if ( search->chessboard[PAWN_BLACK] & 0x400000000000ULL && search->chessboard[BISHOP_BLACK] & 0x80000000000000ULL )
      result -= BISHOP_TRAPPED_DIAG;
    if ( search->chessboard[PAWN_BLACK] & 0x20000000000ULL && search->chessboard[BISHOP_BLACK] & 0x1000000000000ULL )
      result -= BISHOP_TRAPPED_DIAG;
  }
  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEBISHOP;done in lazyeval
    if ( structure->status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, BISHOP_BLACK + side ) ) {
	result -= PINNED_PIECE;
      }
      structure->kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side]][o] ) );
      structure->kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side ^ 1]][o] ) );
    }
    if ( side ) {
      if ( structure->status == OPEN && ( o == C1 || o == F1 ) )
	result -= UNDEVELOPED;
    }
    else {
      if ( structure->status == OPEN && ( o == C8 || o == F8 ) )
	result -= UNDEVELOPED;
    }
    if ( structure->status != OPEN && structure->kingAttacked[o] ) {
      result += ATTACK_KING;
      structure->attacKingCount[side ^ 1]++;
    }
    if ( !( mob = BitCount ( structure->mobility[o] ) ) )
      result -= BISHOP_TRAPPED;
    else {
      if ( !( BIG_DIAG_LEFT[o] & structure->allPieces ) )
	result += OPEN_FILE;
      if ( !( BIG_DIAG_RIGHT[o] & structure->allPieces ) )
	result += OPEN_FILE;
      result += MOB * mob;
      result += BitCount ( structure->allPieces & structure->mobility[o] );
      if ( structure->status != OPEN && NEAR_MASK[structure->posKing[side]] & structure->mobility[o] ) {
	result += NEAR_xKING;
      }
    }
    //attack centre
    if ( structure->status == OPEN ) {
      result += ATTACK_CENTRE * BitCount ( structure->mobility[o] & CENTER_MASK );
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
  u64 queen = search->chessboard[QUEEN_BLACK + side];
  while ( queen ) {
    o = BITScanForward ( queen );
    //result+=VALUEQUEEN;done in lazyeval
    if ( structure->status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, QUEEN_BLACK + side ) ) {
	result -= PINNED_PIECE * 3;
      }
      structure->kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side]][o] ) );
      structure->kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side ^ 1]][o] ) );
    }
    if ( ( structure->pawns[side ^ 1] & VERTICAL[o] ) )
      result += HALF_OPEN_FILE_Q;
    if ( ( VERTICAL[o] & structure->allPieces ) == TABLOG[o] )
      result += OPEN_FILE_Q;
    if ( structure->kingAttacked[o] ) {
      result += ATTACK_KING;
      structure->attacKingCount[side ^ 1]++;
    }
    mob = BitCount ( structure->mobility[o] );
    if ( structure->status != OPEN && !mob )
      result -= QUEEN_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( structure->allPiecesSide[side ^ 1] & structure->mobility[o] );
    }
    if ( LEFT_RIGHT[o] & search->chessboard[BISHOP_BLACK + side] )
      result += BISHOP_ON_QUEEN;
    //attack centre
    if ( structure->status == OPEN ) {
      result += ATTACK_CENTRE * BitCount ( structure->mobility[o] & CENTER_MASK );
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
  x = search->chessboard[KNIGHT_BLACK + side];
  while ( x ) {
    o = BITScanForward ( x );
    //result+=VALUEKNIGHT;//done in lazyeval
    if ( structure->status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, KNIGHT_BLACK + side ) ) {
	result -= PINNED_PIECE;
      }
      structure->kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side]][o] ) );
      structure->kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side ^ 1]][o] ) );
    }
    /* Don't block in central pawns */
    if ( side ) {
      if ( structure->status == OPEN && ( o == B1 || o == G1 ) )
	result -= UNDEVELOPED;
    }
    else {
      if ( structure->status == OPEN && ( o == B8 || o == G8 ) )
	result -= UNDEVELOPED;
    }
    if ( structure->status != OPEN && structure->kingAttacked[o] ) {
      result += ATTACK_KING;
      structure->attacKingCount[side ^ 1]++;
    }
    if ( !( mob = BitCount ( structure->mobility[o] ) ) )
      result -= KNIGHT_TRAPPED;
    else {
      result += MOB * mob;
      result += BitCount ( structure->allPiecesSide[side ^ 1] & structure->mobility[o] );
    }
    //attack centre
    if ( structure->status == OPEN ) {
      result += ATTACK_CENTRE * BitCount ( structure->mobility[o] & CENTER_MASK );
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
  if ( !( x = search->chessboard[ROOK_BLACK + side] ) )
    return 0;
  if ( structure->status == MIDDLE ) {
    if ( !side && ( o = BitCount ( x & ORIZZONTAL_8 ) ) )
      result += ROOK_7TH_RANK * o;
    if ( side && ( o = BitCount ( x & ORIZZONTAL_48 ) ) )
      result += o * ROOK_7TH_RANK;
  }
  while ( x ) {
    o = BITScanForward ( x );

    if ( rook1 == -1 )
      rook1 = o;
    else
      rook2 = o;
    //result +=VALUEROOK;done in lazyeval
    if ( structure->status != OPEN ) {
      //pinned
      if ( isPinned ( side, o, ROOK_BLACK + side ) ) {
	result -= PINNED_PIECE * 2;
      }
      structure->kingSecurityDistance[side] += FRIEND_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side]][o] ) );
      structure->kingSecurityDistance[side] -= ENEMY_NEAR_KING * ( ( INV_DISTANCE[structure->posKing[side ^ 1]][o] ) );
    }
    if ( structure->status != OPEN && structure->kingAttacked[o] ) {
      result += ATTACK_KING;
      structure->attacKingCount[side ^ 1]++;
    }
    if ( !( mob = BitCount ( structure->mobility[o] ) ) )
      result -= ROOK_TRAPPED;
    else {
      result += mob * MOB;
      if ( mob > 11 )
	result += BONUS_11;
    }
    if ( !( structure->pawns[side] & VERTICAL[o] ) )
      result += OPEN_FILE;
    if ( !( structure->pawns[side ^ 1] & VERTICAL[o] ) )
      result += OPEN_FILE;
    /* Penalise if Rook is Blocked Horizontally */
    if ( structure->status != OPEN && ( ( ORIZZ_BOUND[o] & structure->allPieces ) == ORIZZ_BOUND[o] ) ) {
      result -= ROOK_BLOCKED;
    };
    x &= NOTTABLOG[o];
  };
  if ( rook1 != -1 && rook2 != -1 )
    if ( ( !( LINK_ROOKS[rook1][rook2] & structure->allPieces ) ) ) {
      result += CONNECTED_ROOKS;
    }
  return result;
}

int
Eval::evaluateKing ( const int side ) {
#ifdef DEBUG_MODE
  if ( N_EVALUATION[side] != 6 )
    myassert ( 0 );
#endif
  int result = 0;

  u64 pos_king = structure->posKing[side];
  if ( structure->status == FINAL )
    result = SPACE * ( DISTANCE_KING_CLOSURE[pos_king] );
  else
    result = SPACE * ( DISTANCE_KING_OPENING[pos_king] );

  u64 tablog_king = TABLOG[pos_king];
  if ( structure->status != OPEN ) {

    if ( ( structure->openColumn & tablog_king ) || ( structure->semiOpenColumn[side ^ 1] & tablog_king ) ) {
      result -= END_OPENING;
      if ( BitCount ( ORIZZONTAL[pos_king] ) < 4 )
	result -= END_OPENING;
    }
  }
  int y;
  ASSERT ( pos_king < 64 );
  if ( !( NEAR_MASK[pos_king] & structure->pawns[side] ) )
    result -= PAWN_NEAR_KING;
  //result += structure->king_attak[side] / 2;
  if ( structure->status != OPEN ) {
    if ( !( y = BitCount ( structure->mobility[pos_king] ) ) )
      result -= KING_TRAPPED;
    else
      result += MOB * y;
  }
  result += structure->kingSecurityDistance[side];
  return result;
}

#ifdef TEST_MODE
int
Eval::score ( const int side
#ifdef FP_MODE
	      , const int alpha, const int beta
#endif
   ) {
  int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11;
  return score ( side
#ifdef FP_MODE
		 , alpha, beta
#endif
		 , &a1, &a2, &a3, &a4, &a5, &a6, &a7, &a8, &a9, &a10, &a11 );
}
#endif
int
Eval::score ( const int side
#ifdef FP_MODE
	      , const int alpha, const int beta
#endif
#ifdef TEST_MODE
	      , int *material_score, int *pawns_score, int *passed_pawns_score, int *knights_score, int *bishop_score, int *rooks_score, int *queens_score, int *kings_score, int *development_score, int *pawn_race_score, int *total_score
#endif
   ) {

  int lazyscore_white = lazyEvalWhite (  );
  int lazyscore_black = lazyEvalBlack (  );
#ifdef FP_MODE
  int lazyscore = lazyscore_black - lazyscore_white;
  if ( side )
    lazyscore = -lazyscore;
  if ( lazyscore > ( beta + FUTIL_MARGIN ) || lazyscore < ( alpha - FUTIL_MARGIN ) ) {
#ifdef DEBUG_MODE
    search->LazyEvalCuts++;
#endif
    if ( side )			//white
      return lazyscore -= 5;	//5 bonus for the side on move.

    else
      return lazyscore += 5;
  }
#endif
#ifdef DEBUG_MODE
  N_EVALUATION[0] = N_EVALUATION[1] = 0;
#endif
  int mob_black, mob_white;
  memset ( structure->mobility, 0, sizeof ( structure->mobility ) );
  memset ( structure->kingSecurityDistance, 0, sizeof ( structure->kingSecurityDistance ) );
  memset ( structure->kingAttacked, 0, sizeof ( structure->kingAttacked ) );
  memset ( structure->attacKingCount, 0, sizeof ( structure->attacKingCount ) );
#ifdef TEST_MODE
  memset ( structure->race_pawn_score, 0, sizeof ( structure->race_pawn_score ) );
#endif
  int npieces = search->n_pieces ( WHITE ) + search->n_pieces ( BLACK );
  if ( npieces < 6 )
    structure->status = FINAL;
  else if ( npieces < 13 )
    structure->status = MIDDLE;
  else
    structure->status = OPEN;
  structure->allPieces = search->squareAllBitOccupied (  );
  structure->posKing[BLACK] = ( unsigned short ) BITScanForward ( search->chessboard[KING_BLACK] );
  structure->posKing[WHITE] = ( unsigned short ) BITScanForward ( search->chessboard[KING_WHITE] );

  if ( !( mob_black = evaluateMobility ( BLACK ) ) ) {
    if ( side )
      return _INFINITE;
    else
      return -_INFINITE;
  }
  if ( !( mob_white = evaluateMobility ( WHITE ) ) ) {
    if ( side )
      return -_INFINITE;
    else
      return _INFINITE;
  }
  ASSERT ( structure->posKing[0] != -1 );
  ASSERT ( structure->posKing[1] != -1 );

  structure->allPiecesSide[BLACK] = search->squareBitOccupied ( BLACK );
  structure->allPiecesSide[WHITE] = search->squareBitOccupied ( WHITE );
  structure->pawns[BLACK] = search->chessboard[BLACK];
  structure->pawns[WHITE] = search->chessboard[WHITE];
  structure->queens[WHITE] = search->chessboard[QUEEN_WHITE];
  structure->queens[BLACK] = search->chessboard[QUEEN_BLACK];
  structure->rooks[BLACK] = search->chessboard[ROOK_BLACK];
  structure->rooks[WHITE] = search->chessboard[ROOK_WHITE];
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
  int castle_score[2];
  castle_score[BLACK] = castle_score[WHITE] = 0;
  int bonus_attack_black_king = 0;
  int bonus_attack_white_king = 0;
  if ( structure->status != OPEN ) {
    bonus_attack_black_king = BONUS_ATTACK_KING[BitCount ( structure->attacKingCount[WHITE] )];
    bonus_attack_white_king = BONUS_ATTACK_KING[BitCount ( structure->attacKingCount[BLACK] )];
  }
  int result = ( bonus_attack_black_king + lazyscore_black + mob_black + pawns_score_black + knights_score_black + bishop_score_black + rooks_score_black + queens_score_black + kings_score_black + castle_score[BLACK] )
    - ( bonus_attack_white_king + lazyscore_white + mob_white + pawns_score_white + knights_score_white + bishop_score_white + rooks_score_white + queens_score_white + kings_score_white + castle_score[WHITE] );
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
#ifdef TEST_MODE
  *material_score = lazyscore_white - lazyscore_black;
  *pawns_score = pawns_score_white - pawns_score_black;
  *passed_pawns_score = structure->passed_pawn_score[WHITE] - structure->passed_pawn_score[BLACK];
  *knights_score = knights_score_white - knights_score_black;
  *bishop_score = bishop_score_white - bishop_score_black;
  *rooks_score = rooks_score_white - rooks_score_black;
  *queens_score = queens_score_white - queens_score_black;
  *kings_score = kings_score_white - kings_score_black;
  *development_score = -9999999;
  *pawn_race_score = structure->race_pawn_score[WHITE] - structure->race_pawn_score[BLACK];
  *total_score = -result;
#endif
  if ( side )
    result = -result;
  return result;
}

void
Eval::setRandomParam (  ) {
  const int percent = 95;
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
  x = rand (  ) % BLOCK_PAWNS * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BLOCK_PAWNS = BLOCK_PAWNS + x;
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
  x = rand (  ) % BONUS_CASTLE * percent / 100;
  x = ( rand (  ) % 2 ) == 0 ? x : -x;
  BONUS_CASTLE = BONUS_CASTLE + x;
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
Eval::writeParam ( char *param_file, int cd_param, bool append ) {
  FILE *fil;
  if ( append )
    fil = fopen ( param_file, "a" );
  else
    fil = fopen ( param_file, "w" );
  fprintf ( fil, "------------------------- param%d.txt -------------------------\n", cd_param );
  fprintf ( fil, "ATTACK_KING=%d\n", ATTACK_KING );
  fprintf ( fil, "OPEN_FILE_Q=%d\n", OPEN_FILE_Q );
  fprintf ( fil, "FORK_SCORE=%d\n", FORK_SCORE );
  fprintf ( fil, "BONUS2BISHOP=%d\n", BONUS2BISHOP );
  fprintf ( fil, "MOB=%d\n", MOB );
  fprintf ( fil, "KING_TRAPPED=%d\n", KING_TRAPPED );
  fprintf ( fil, "KNIGHT_TRAPPED=%d\n", KNIGHT_TRAPPED );
  fprintf ( fil, "BISHOP_TRAPPED=%d\n", BISHOP_TRAPPED );
  fprintf ( fil, "ROOK_TRAPPED=%d\n", ROOK_TRAPPED );
  fprintf ( fil, "QUEEN_TRAPPED=%d\n", QUEEN_TRAPPED );
  fprintf ( fil, "CONNECTED_ROOKS=%d\n", CONNECTED_ROOKS );
  fprintf ( fil, "ROOK_BLOCKED=%d\n", ROOK_BLOCKED );
  fprintf ( fil, "ROOK_7TH_RANK=%d\n", ROOK_7TH_RANK );
  fprintf ( fil, "OPEN_FILE=%d\n", OPEN_FILE );
  fprintf ( fil, "BLOCK_PAWNS=%d\n", BLOCK_PAWNS );
  fprintf ( fil, "UNDEVELOPED=%d\n", UNDEVELOPED );
  fprintf ( fil, "HALF_OPEN_FILE_Q=%d\n", HALF_OPEN_FILE_Q );
  fprintf ( fil, "DOUBLED_PAWNS=%d\n", DOUBLED_PAWNS );
  fprintf ( fil, "PAWN_IN_RACE=%d\n", PAWN_IN_RACE );
  fprintf ( fil, "PAWN_7H=%d\n", PAWN_7H );
  fprintf ( fil, "PAWN_CENTRE=%d\n", PAWN_CENTRE );
  fprintf ( fil, "FRIEND_NEAR_KING=%d\n", FRIEND_NEAR_KING );
  fprintf ( fil, "ENEMY_NEAR_KING=%d\n", ENEMY_NEAR_KING );
  fprintf ( fil, "PAWN_ISOLATED=%d\n", PAWN_ISOLATED );
  fprintf ( fil, "SPACE=%d\n", SPACE );
  fprintf ( fil, "END_OPENING=%d\n", END_OPENING );
  fprintf ( fil, "PAWN_NEAR_KING=%d\n", PAWN_NEAR_KING );
  fprintf ( fil, "BONUS_CASTLE=%d\n", BONUS_CASTLE );
  fprintf ( fil, "NEAR_xKING=%d\n", NEAR_xKING );
  fprintf ( fil, "BONUS_11=%d\n", BONUS_11 );
  fprintf ( fil, "BISHOP_ON_QUEEN=%d\n", BISHOP_ON_QUEEN );
  fprintf ( fil, "ENEMIES_PAWNS_ALL=%d\n", ENEMIES_PAWNS_ALL );
  fprintf ( fil, "DOUBLED_ISOLATED_PAWNS=%d\n", DOUBLED_ISOLATED_PAWNS );
  fprintf ( fil, "BACKWARD_PAWN=%d\n", BACKWARD_PAWN );
  fprintf ( fil, "BACKWARD_OPEN_PAWN=%d\n", BACKWARD_OPEN_PAWN );
  fprintf ( fil, "BISHOP_TRAPPED_DIAG=%d\n", BISHOP_TRAPPED_DIAG );
  fprintf ( fil, "ATTACK_F7_F2=%d\n", ATTACK_F7_F2 );
  fprintf ( fil, "UNPROTECTED_PAWNS=%d\n", UNPROTECTED_PAWNS );
  fclose ( fil );
}

int
Eval::getValue ( char *s ) {
  char *i = strstr ( s, "=" );
  if ( !i ) {
    cout << "\n.ini error";
    myassert ( 0 );
  }
  int y = atoi ( i + 1 );
  return y;
}

void
Eval::readParam ( char *param_file ) {

  if ( !param_file )
    return;

  cout << "\nread " << param_file << ".." << flush;
  FILE *stream = fopen ( param_file, "r" );
  if ( !stream ) {
    cout << "\nerror file not found";
    myassert ( 0 );
  }
  char line[100];
  char *dummy;
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ATTACK_KING" ) == line )
    ATTACK_KING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "OPEN_FILE_Q" ) == line )
    OPEN_FILE_Q = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "FORK_SCORE" ) == line )
    FORK_SCORE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BONUS2BISHOP" ) == line )
    BONUS2BISHOP = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "MOB" ) == line )
    MOB = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "KING_TRAPPED" ) == line )
    KING_TRAPPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "KNIGHT_TRAPPED" ) == line )
    KNIGHT_TRAPPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BISHOP_TRAPPED" ) == line )
    BISHOP_TRAPPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ROOK_TRAPPED" ) == line )
    ROOK_TRAPPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "QUEEN_TRAPPED" ) == line )
    QUEEN_TRAPPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "CONNECTED_ROOKS" ) == line )
    CONNECTED_ROOKS = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ROOK_BLOCKED" ) == line )
    ROOK_BLOCKED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ROOK_7TH_RANK" ) == line )
    ROOK_7TH_RANK = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "OPEN_FILE" ) == line )
    OPEN_FILE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BLOCK_PAWNS" ) == line )
    BLOCK_PAWNS = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "UNDEVELOPED" ) == line )
    UNDEVELOPED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "HALF_OPEN_FILE_Q" ) == line )
    HALF_OPEN_FILE_Q = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "DOUBLED_PAWNS" ) == line )
    DOUBLED_PAWNS = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "PAWN_IN_RACE" ) == line )
    PAWN_IN_RACE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "PAWN_7H" ) == line )
    PAWN_7H = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "PAWN_CENTRE" ) == line )
    PAWN_CENTRE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "FRIEND_NEAR_KING" ) == line )
    FRIEND_NEAR_KING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ENEMY_NEAR_KING" ) == line )
    ENEMY_NEAR_KING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "PAWN_ISOLATED" ) == line )
    PAWN_ISOLATED = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "SPACE" ) == line )
    SPACE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "END_OPENING" ) == line )
    END_OPENING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "PAWN_NEAR_KING" ) == line )
    PAWN_NEAR_KING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BONUS_CASTLE" ) == line )
    BONUS_CASTLE = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "NEAR_xKING" ) == line )
    NEAR_xKING = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BONUS_11" ) == line )
    BONUS_11 = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BISHOP_ON_QUEEN" ) == line )
    BISHOP_ON_QUEEN = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ENEMIES_PAWNS_ALL" ) == line )
    ENEMIES_PAWNS_ALL = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "DOUBLED_ISOLATED_PAWNS" ) == line )
    DOUBLED_ISOLATED_PAWNS = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BACKWARD_PAWN" ) == line )
    BACKWARD_PAWN = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BACKWARD_OPEN_PAWN" ) == line )
    BACKWARD_OPEN_PAWN = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "BISHOP_TRAPPED_DIAG" ) == line )
    BISHOP_TRAPPED_DIAG = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "ATTACK_F7_F2" ) == line )
    ATTACK_F7_F2 = getValue ( line );
  dummy = fgets ( line, sizeof ( line ), stream );
  if ( strstr ( line, "UNPROTECTED_PAWNS" ) == line )
    UNPROTECTED_PAWNS = getValue ( line );
  fclose ( stream );
  cout << "ok\n";
}

int
Eval::isPinned ( const int side, const char Position, const char piece ) {
  ASSERT ( structure->status != OPEN );
  int xside = side ^ 1;
  u64 king = search->chessboard[KING_BLACK + side];
  int result = -1;
  if ( king & ORIZZONTAL[Position] ) {
    char Position_mod_8 = ROT45[Position];
    int Position_Position_mod_8 = pos_posMod8[Position];
    search->chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[( uchar ) ( ( ( ( structure->allPieces & ORIZZONTAL[Position] ) >> Position_Position_mod_8 ) ) )][Position_mod_8]
	 & ( ( search->chessboard[ROOK_BLACK + xside] >> Position_Position_mod_8 ) & 255 | ( search->chessboard[QUEEN_BLACK + xside] >> Position_Position_mod_8 ) & 255 ) ) {
      result = 1;
    }
  }
  else if ( king & VERTICAL[Position] ) {
    search->chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[rotateBoard90 ( ( structure->allPieces & VERTICAL[Position] ) & VERTICAL[Position] )][ROT45ROT_90_MASK[Position]]
	 & ( ( rotateBoard90 ( search->chessboard[ROOK_BLACK + xside]
			       & VERTICAL[Position] ) | rotateBoard90 ( search->chessboard[QUEEN_BLACK + xside] & VERTICAL[Position] ) ) ) ) {
      result = 1;
    }
  }
  else if ( king & LEFT[Position] ) {
    char Position_mod_8 = ROT45[Position];
    search->chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[search->rotateBoardLeft45 ( ( structure->allPieces & LEFT[Position] ), Position )][Position_mod_8]
	 & ( search->rotateBoardLeft45 ( search->chessboard[BISHOP_BLACK + ( xside )], Position )
	     | search->rotateBoardLeft45 ( search->chessboard[QUEEN_BLACK + xside], Position ) ) ) {
      result = 1;
    }
  }
  else if ( king & RIGHT[Position] ) {
    char Position_mod_8 = ROT45[Position];
    search->chessboard[piece] &= NOTTABLOG[Position];
    result = 0;
    if ( MASK_CAPT_MOV[search->rotateBoardRight45 ( ( structure->allPieces & RIGHT[Position] ), Position ) | TABLOG[Position_mod_8]][Position_mod_8]
	 & ( search->rotateBoardRight45 ( search->chessboard[BISHOP_BLACK + ( xside )], Position )
	     | search->rotateBoardRight45 ( search->chessboard[QUEEN_BLACK + ( xside )], Position ) ) ) {
      result = 1;
    }
  }
  if ( result == -1 )
    return 0;
  search->chessboard[piece] |= TABLOG[Position];
  return result;
}
