#ifndef GENMOVES_H_
#define GENMOVES_H_
#include "maindefine.h"
#include "bitmap.h"
#include "utils.h"
#include "ChessBoard.h"

class GenMoves:public ChessBoard {
public:
  GenMoves (  );
  void setPerft ( bool b );
  //void resetRepetitionCount();
  void setKillerHeuristic ( int from, int to, int value );
  void incKillerHeuristic ( int from, int to, int value );
  void clearKillerHeuristic (  );
   virtual ~ GenMoves (  );
  void init (  );
  bool generateCap ( const int side, u64, u64, u64 * key );
  void generateMoves ( const int side, u64 );

  u64 getTotMoves (  );
  bool attackSquare ( const int side, const uchar Position );
  void initKillerHeuristic (  );
#ifdef DEBUG_MODE
  int n_cut, null_move_cut, n_cut_fp, n_cut_razor;
  double beta_efficency_tot, beta_efficency;
#endif
  int getListCount (  ) {
    return gen_list[list_id][0].score;
  } void resetList (  ) {
    gen_list[list_id][0].score = 0;
  }
  void decListId (  ) {
    list_id--;
  }
  void incListId (  ) {
    list_id++;
  }

  /*
     void popStackMove1() {
     ASSERT(repetitionMapCount);
     repetitionMapCount--;
     if(repetitionMapCount>1 && !repetitionMap[repetitionMapCount-1].key) {
     ASSERT(repetitionMapCount);
     repetitionMapCount--;
     }
     }

     void pushStackMove(u64 key,bool reset) {
     ASSERT(repetitionMapCount<sizeof(repetitionMap)/sizeof(u64)-1);
     repetitionMap[repetitionMapCount].reset= reset;
     repetitionMap[repetitionMapCount++].key= key;
     }
   */
  void takeback ( _Tmove * move, u64 * key, const u64 oldkey ) {
    *key = oldkey;
    enpassantPosition = -1;
    int side, pieceFrom, posTo, posFrom, movecapture;
    side = move->side;
    RIGHT_CASTLE = move->type & 0 b11110000;
    if ( ( move->type & 0 b00000011 ) == STANDARD_MOVE_MASK || ( move->type & 0 b00000011 ) == ENPASSANT_MOVE_MASK ) {
      posTo = move->to;
      posFrom = move->from;
      movecapture = move->capturedPiece;
      ASSERT ( posFrom >= 0 && posFrom < 64 );
      ASSERT ( posTo >= 0 && posTo < 64 );
      pieceFrom = move->pieceFrom;
      chessboard[pieceFrom] = ( chessboard[pieceFrom] & NOTTABLOG[posTo] ) | TABLOG[posFrom];
      if ( movecapture != SQUARE_FREE ) {
	if ( ( ( move->type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) ) {
	  chessboard[movecapture] |= TABLOG[posTo];
	}
	else {
	  ASSERT ( movecapture == ( side ^ 1 ) );
	  if ( side ) {
	    chessboard[movecapture] |= TABLOG[posTo - 8];
	  }
	  else {
	    chessboard[movecapture] |= TABLOG[posTo + 8];
	  }
	}
      }

    }
    else if ( ( move->type & 0 b00000011 ) == PROMOTION_MOVE_MASK ) {
      posTo = move->to;
      posFrom = move->from;
      movecapture = move->capturedPiece;
      ASSERT ( posTo >= 0 );
      chessboard[side] = chessboard[side] | TABLOG[posFrom];
      chessboard[move->promotionPiece] = chessboard[move->promotionPiece] & NOTTABLOG[posTo];
      if ( movecapture != SQUARE_FREE ) {
	chessboard[movecapture] |= TABLOG[posTo];
      }
    }
    else if ( move->type & 0 b00001100 ) {	//castle
      unPerformCastle ( move->side, move->type );
    }
  }

  void makemove ( _Tmove * move, u64 * key ) {
    ASSERT ( move );
    ASSERT ( bitCount ( chessboard[KING_WHITE] ) == 1 && bitCount ( chessboard[KING_BLACK] ) == 1 );
    int pieceFrom, posTo, posFrom, movecapture;
    int side = move->side;
    uchar RIGHT_CASTLE_old = RIGHT_CASTLE;
    if ( !( move->type & 0 b00001100 ) ) {	//no castle
      posTo = move->to;
      posFrom = move->from;
      ASSERT ( posTo >= 0 );
      movecapture = move->capturedPiece;
      ASSERT ( posFrom >= 0 && posFrom < 64 );
      ASSERT ( posTo >= 0 && posTo < 64 );
      pieceFrom = move->pieceFrom;
      if ( ( move->type & 0 b00000011 ) == PROMOTION_MOVE_MASK ) {
	chessboard[pieceFrom] = chessboard[pieceFrom] & NOTTABLOG[posFrom];
	updateZobristKey ( key, pieceFrom, posFrom );
	chessboard[move->promotionPiece] |= TABLOG[posTo];
	updateZobristKey ( key, move->promotionPiece, posTo );

      }
      else {
	chessboard[pieceFrom] = ( chessboard[pieceFrom] | TABLOG[posTo] ) & NOTTABLOG[posFrom];
	updateZobristKey ( key, pieceFrom, posFrom );
	updateZobristKey ( key, pieceFrom, posTo );
      }

      if ( movecapture != SQUARE_FREE ) {
	if ( ( move->type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) {
	  chessboard[movecapture] &= NOTTABLOG[posTo];
	  updateZobristKey ( key, movecapture, posTo );
	}
	else {
	  ASSERT ( movecapture == ( side ^ 1 ) );
	  if ( side ) {
	    chessboard[movecapture] &= NOTTABLOG[posTo - 8];
	    updateZobristKey ( key, movecapture, posTo - 8 );
	  }
	  else {
	    chessboard[movecapture] &= NOTTABLOG[posTo + 8];
	    updateZobristKey ( key, movecapture, posTo + 8 );

	  }
	}
      }

      //lost castle right

      switch ( pieceFrom ) {
      case KING_WHITE:{
	RIGHT_CASTLE &= 0 b11001111;
      }
	break;
      case KING_BLACK:{
	RIGHT_CASTLE &= 0 b00111111;
      }
	break;

      case ROOK_WHITE:
	if ( posFrom == 0 ) {
	  RIGHT_CASTLE &= 0 b11101111;
	}
	else if ( posFrom == 7 ) {
	  RIGHT_CASTLE &= 0 b11011111;
	}
	break;
      case ROOK_BLACK:
	if ( posFrom == 56 ) {
	  RIGHT_CASTLE &= 0 b10111111;
	}
	else if ( posFrom == 63 ) {
	  RIGHT_CASTLE &= 0 b01111111;
	}
	break;
	//en passant
      case PAWN_WHITE:
	if ( ( RANK_1 & TABLOG[posFrom] ) && ( RANK_3 & TABLOG[posTo] ) ) {
	  enpassantPosition = posTo;
	  updateZobristKey ( key, 13, enpassantPosition );
	}
	break;

      case PAWN_BLACK:
	if ( ( RANK_6 & TABLOG[posFrom] ) && ( RANK_4 & TABLOG[posTo] ) ) {
	  enpassantPosition = posTo;
	  updateZobristKey ( key, 13, enpassantPosition );
	}
	break;
      default:
	;
      }
    }
    else if ( move->type & 0 b00001100 ) {	//castle
#ifdef DEBUG_MODE
      ASSERT ( side == 0 || side == 1 );
      ASSERT ( move->side == side );
#endif
      performCastle ( side, move->type, key );
      if ( side == WHITE ) {
	RIGHT_CASTLE &= 0 b11001111;
      }
      else {
	RIGHT_CASTLE &= 0 b00111111;
      }
    }
    uchar x2 = RIGHT_CASTLE_old ^ RIGHT_CASTLE;
    int position;
    while ( x2 ) {
      position = BITScanForward ( x2 );
      updateZobristKey ( key, 14, position );
      x2 &= NOTTABLOG[position];
    }
  }

protected:
  bool perftMode;
  int currentPly;
  int evaluateMobility ( const int side );
  u64 numMoves, numMovesq;
  //_Trep repetitionMap[200];
  int killerHeuristic[64][64];
  bool evaluateMobilityMode;
  int list_id;
  _Tmove **gen_list;
  int getNextMove ( _Tmove * );
  bool inCheck ( const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, const int side, int promotionPiece );
  void performCastle ( const int side, const uchar type, u64 * key );
  void unPerformCastle ( const int side, const uchar type );
  void tryAllCastle ( const int side, const u64 ALLPIECES );
  bool performKingShiftCapture ( const int piece, const u64 enemies, const int side );
  bool performKnightShiftCapture ( const int piece, const u64 enemies, const int side );
  bool performBishopCapture ( const int piece, const u64 enemies, const int side, const u64 ALLPIECES );
  bool performRookQueenCapture ( const int piece, const u64 enemies, const int side, const u64 ALLPIECES );
  bool performPawnCapture ( const u64 enemies, const int side, u64 * key );
  void performPawnShift ( const int side, const u64 XALLPIECES );
  void performBishopShift ( const int piece, const int side, const u64 ALLPIECES );
  void performRookQueenShift ( const int piece, const int side, const u64 ALLPIECES );
  void checkJumpPawn ( const u64 sc, const int side, const u64 XALLPIECES );
  _Tmove *getList ( int i ) {
    return &gen_list[list_id][i];
  }

  void setEvaluateMobilityMode ( bool b ) {
    evaluateMobilityMode = b;
  }
  bool pushmove ( const uchar type, const int from, const int to, const int side, int promotionPiece, int pieceFrom ) {
    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );
    if ( !perftMode ) {
      if ( evaluateMobilityMode ) {
	if ( !( type & 0 b00001100 ) ) {	//no castle
	  ASSERT ( from >= 0 && to >= 0 );
	  structure.mobility[from] |= TABLOG[to];
	  if ( ( TABLOG[to] & chessboard[KING_BLACK + ( side ^ 1 )] ) )
	    structure.kingAttacked[from] = 1;
	  ASSERT ( list_id < MAX_PLY && list_id >= 0 );
	  ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
	  gen_list[list_id][0].score++;
	}
	return false;
      }
    }
    int piece_captured = -1;
    bool res = false;
    if ( ( ( type & 0 b00000011 ) != ENPASSANT_MOVE_MASK ) && !( type & 0 b00001100 ) ) {
      piece_captured = getPieceAt ( ( side ^ 1 ), TABLOG[to] );
      if ( piece_captured == KING_BLACK + ( side ^ 1 ) )
	res = true;
    }
    else if ( !( type & 0 b00001100 ) )	//no castle
      piece_captured = side ^ 1;


    if ( perftMode && !( type & 0 b00001100 ) ) {	//no castle
      if ( inCheck ( from, to, type, pieceFrom, piece_captured, side, promotionPiece ) )
	return false;
    }

    _Tmove *mos;
#ifdef DEBUG_MODE
    if ( !( list_id >= 0 && list_id < MAX_PLY ) ) {
      cout << "list_id: " << list_id << endl << flush;
      ASSERT ( 0 );
    }
#endif
    ASSERT ( list_id < MAX_PLY );
    ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
    mos = &gen_list[list_id][++gen_list[list_id][0].score];
    mos->type = RIGHT_CASTLE | type;
    mos->side = ( char ) side;
    mos->used = false;
    mos->capturedPiece = piece_captured;
    if ( type & 0 b00000011 ) {
      mos->from = ( uchar ) from;
      mos->to = ( uchar ) to;
      mos->pieceFrom = pieceFrom;
      mos->promotionPiece = ( char ) promotionPiece;
      if ( !perftMode ) {
	if ( res == true ) {
	  mos->score = _INFINITE;
	}
	else {
	  mos->score = 0;
	  mos->score += killerHeuristic[from][to];
	  mos->score += ( PIECES_VALUE[piece_captured] >= PIECES_VALUE[pieceFrom] ) ? ( PIECES_VALUE[piece_captured] - PIECES_VALUE[pieceFrom] ) * 2 : PIECES_VALUE[piece_captured];
	  ASSERT ( pieceFrom >= 0 && pieceFrom < 12 && to >= 0 && to < 64 && from >= 0 && from < 64 );
	}
      }
    }

    else if ( type & 0 b00001100 ) {	//castle
      ASSERT ( RIGHT_CASTLE );
      mos->score = 100;
    }

    ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
    return res;
  }
};
#endif
