#ifndef GENMOVES_H_
#define GENMOVES_H_
#include "maindefine.h"
#include "bitmap.h"
#include "Bits.h"
#include "ChessBoard.h"

class GenMoves:public ChessBoard {
public:
  GenMoves ( char * );
   virtual ~ GenMoves (  );
  int generateCap (  );
  void init (  );
  void generateMoves (  );
  int generateCap ( const uchar tipomove, const int side );
  void generateMoves ( const uchar tipomove, const int side );
  uchar rotateBoardLeft45 ( const u64 ss, const int pos );
  uchar rotateBoardRight45 ( const u64 ss, const int pos );
  u64 getTonMoves (  );
  int attackSquare ( const int side, const int Position );
  void initKillerHeuristic (  );
  void setKillerHeuristic ( int from, int to, int value );
#ifdef DEBUG_MODE
  int beta_efficency_tot, beta_efficency_tot_count, n_cut, null_move_cut, n_cut_fp, n_cut_razor;
  double beta_efficency;
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
#ifdef DEBUG_MODE
#ifndef PERFT_MODE
    for ( int i = 0; i < MAX_MOVE; i++ )
      gen_list[list_id][i].nextBoard = 0;
#endif
#endif
  }
  Tmove *getList ( int i ) {
    return &gen_list[list_id][i];
  }

  void makemove ( Tmove * mossa ) {
    ASSERT ( mossa );
    int piece_from, mossaa, mossada, mossacapture;
    int SIDE = mossa->side;
    int XSIDE = SIDE ^ 1;
    if ( mossa->type & 0x3 ) {
      ASSERT ( !( mossa->type & 0xC ) );
      mossaa = mossa->to;
      mossada = mossa->from;
      ASSERT ( mossaa >= 0 );
      if ( ( ( mossa->type & 0x3 ) != ENPASSANT_MOVE_MASK ) )
	mossacapture = get_piece_at ( XSIDE, TABLOG[mossaa] );
      else
	mossacapture = XSIDE;
      mossa->capture = ( char ) mossacapture;
      ASSERT ( mossada >= 0 && mossada < 64 );
      ASSERT ( mossaa >= 0 && mossaa < 64 );
      piece_from = get_piece_at ( SIDE, TABLOG[mossada] );
      if ( ( mossa->type & 0x3 ) == PROMOTION_MOVE_MASK ) {
	chessboard[piece_from] = chessboard[piece_from] & NOTTABLOG[mossada];
	chessboard[mossa->promotion_piece] |= TABLOG[mossaa];
      }
      else {
	chessboard[piece_from] = ( chessboard[piece_from] | TABLOG[mossaa] ) & NOTTABLOG[mossada];
      }

      if ( mossacapture != SQUARE_FREE ) {
	if ( ( mossa->type & 0x3 ) != ENPASSANT_MOVE_MASK ) {
	  chessboard[mossacapture] &= NOTTABLOG[mossaa];
	}
	else {
	  ASSERT ( mossacapture == ( SIDE ^ 1 ) );
	  if ( SIDE ) {
	    chessboard[mossacapture] &= NOTTABLOG[mossaa - 8];
	  }
	  else {
	    chessboard[mossacapture] &= NOTTABLOG[mossaa + 8];
	  }
	}
      }

      //lost castle right
      switch ( piece_from ) {
      case KING_WHITE:
	RIGHT_CASTLE &= 0xCF;
	break;
      case KING_BLACK:
	RIGHT_CASTLE &= 0x3F;
	break;
      case ROOK_WHITE:
	if ( mossada == 0 ) {
	  RIGHT_CASTLE &= 0xEF;
	}
	else if ( mossada == 7 ) {
	  RIGHT_CASTLE &= 0xDF;
	}
	break;
      case ROOK_BLACK:
	if ( mossada == 56 )
	  RIGHT_CASTLE &= 0xBF;
	else if ( mossada == 63 )
	  RIGHT_CASTLE &= 0x7F;
	break;
	//en passant
      case PAWN_WHITE:
	if ( ( RANK_1 & TABLOG[mossada] ) && ( RANK_3 & TABLOG[mossaa] ) )
	  enpassantPosition = mossaa;
	break;

      case PAWN_BLACK:
	if ( ( RANK_6 & TABLOG[mossada] ) && ( RANK_4 & TABLOG[mossaa] ) )
	  enpassantPosition = mossaa;
	break;
      }
    }
    else if ( mossa->type & 0xC ) {	//castle
#ifdef DEBUG_MODE
      ASSERT ( SIDE == 0 || SIDE == 1 );
      ASSERT ( mossa->side == SIDE );
#endif

      performCastle ( SIDE, mossa->type );
      if ( SIDE == WHITE ) {
	RIGHT_CASTLE &= 0xCF;
      }
      else {
	RIGHT_CASTLE &= 0x3F;
      }
    }
  }

  void takeback ( const Tmove * mossa ) {
    enpassantPosition = -1;
    int side, piece_from, mossaa, mossada, mossacapture;
    side = mossa->side;
    RIGHT_CASTLE = mossa->type & 0xF0;
    if ( ( mossa->type & 0x3 ) == STANDARD_MOVE_MASK || ( mossa->type & 0x3 ) == ENPASSANT_MOVE_MASK ) {
      mossaa = mossa->to;
      mossada = mossa->from;
      mossacapture = mossa->capture;
      ASSERT ( mossada >= 0 && mossada < 64 );
      ASSERT ( mossaa >= 0 && mossaa < 64 );
      piece_from = get_piece_at ( side, TABLOG[mossaa] );
      chessboard[piece_from] = ( chessboard[piece_from] & NOTTABLOG[mossaa] ) | TABLOG[mossada];
      if ( mossacapture != SQUARE_FREE ) {
	if ( ( ( mossa->type & 0x3 ) != ENPASSANT_MOVE_MASK ) ) {
	  chessboard[mossacapture] |= TABLOG[mossaa];
	}
	else {
	  ASSERT ( mossacapture == ( side ^ 1 ) );
	  if ( side ) {
	    chessboard[mossacapture] |= TABLOG[mossaa - 8];
	  }
	  else {
	    chessboard[mossacapture] |= TABLOG[mossaa + 8];
	  }
	}
      }

    }
    else if ( ( mossa->type & 0x3 ) == PROMOTION_MOVE_MASK ) {
      mossaa = mossa->to;
      mossada = mossa->from;
      mossacapture = mossa->capture;
      ASSERT ( mossaa >= 0 );
      piece_from = get_piece_at ( side, TABLOG[mossaa] );
      chessboard[side] = chessboard[side] | TABLOG[mossada];
      chessboard[mossa->promotion_piece] = chessboard[mossa->promotion_piece] & NOTTABLOG[mossaa];
      if ( mossacapture != SQUARE_FREE ) {
	chessboard[mossacapture] |= TABLOG[mossaa];
      }
    }
    else if ( mossa->type & 0xC ) {	//castle
      unperformCastle ( mossa->side, mossa->type );
    }
  }
  void setEvaluateMobilityMode ( bool b ) {
    evaluateMobilityMode = b;
  }
protected:
  u64 num_moves, num_movesq;
  int KillerHeuristic[64][64];
  bool evaluateMobilityMode;
  int list_id;
  Tmove **gen_list;
  int getNextMove ( Tmove * );
  int inCheck ( const int da, const int a, const uchar tipo, const int piece_from, const int piece_to, const int SIDE, int promotion_piece );
  void unperformCastle ( const int side, const uchar type );
  int performKing_Shift_Capture ( const uchar tipomove, const int piece, const u64 enemies, const int side );
  int performKnight_Shift_Capture ( const uchar tipomove, const int piece, const u64 enemies, const int side );
  int performBishopCapture ( const uchar tipomove, const int piece, const u64 enemies, const int side, const u64 ALLPIECES );
  int performRookQueenCapture ( const uchar tipomove, const int piece, const u64 enemies, const int side, const u64 ALLPIECES );
  int performPawnCapture ( const uchar tipomove, const u64 enemies, const int side );
  void performPawnShift ( const uchar tipomove, const int side, const u64 XALLPIECES );
  void tryAllCastle ( const int side, const u64 ALLPIECES );
  void performBishopShift ( const uchar tipomove, const int piece, const int side, const u64 ALLPIECES );
  void performRookQueenShift ( const uchar tipomove, const int piece, const int side, const u64 ALLPIECES );
  void checkJumpPawn ( const uchar tipomove, const u64 sc, const int side, const u64 XALLPIECES );
  void performCastle ( const int side, const uchar type );

  int pushmove ( const uchar tipomove1, const int da, const int a, const int SIDE, int promotion_piece ) {
    ASSERT ( chessboard[KING_BLACK] );
    ASSERT ( chessboard[KING_WHITE] );
#ifndef PERFT_MODE
    if ( evaluateMobilityMode ) {
      if ( !( tipomove1 & 0xC ) ) {	//no castle
	ASSERT ( da >= 0 && a >= 0 );
	structure.mobility[da] |= TABLOG[a];
	if ( ( TABLOG[a] & chessboard[KING_BLACK + ( SIDE ^ 1 )] ) )
	  structure.kingAttacked[da] = 1;
	ASSERT ( list_id < MAX_PLY && list_id >= 0 );
	ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
	gen_list[list_id][0].score++;
      }
      return 0;
    }

#endif
    int piece_to = -1;
    int res = 0;
    if ( ( ( tipomove1 & 0x3 ) != ENPASSANT_MOVE_MASK ) && !( tipomove1 & 0xC ) ) {
      piece_to = get_piece_at ( ( SIDE ^ 1 ), TABLOG[a] );
      if ( piece_to == KING_BLACK + ( SIDE ^ 1 ) )
	res = 1;
    }
    else if ( !( tipomove1 & 0xC ) )	//no castle
      piece_to = SIDE ^ 1;
#ifdef PERFT_MODE
    int piece_from = -5;
    if ( !( tipomove1 & 0xC ) ) {	//no castle
      piece_from = get_piece_at ( SIDE, TABLOG[da] );
      if ( inCheck ( da, a, tipomove1, piece_from, piece_to, SIDE, promotion_piece ) )
	return 0;
    }
#endif
    Tmove *mos;
#ifdef DEBUG_MODE
    if ( !( list_id >= 0 && list_id < MAX_PLY ) ) {
      cout << "list_id: " << list_id << endl << flush;
      myassert ( 0 );
    }
#endif
    ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
    mos = &gen_list[list_id][++gen_list[list_id][0].score];
    mos->type = RIGHT_CASTLE | tipomove1;
    mos->side = ( char ) SIDE;
#ifndef PERFT_MODE
#ifdef DEBUG_MODE
    assert ( mos->nextBoard < MAX_PLY );
    memcpy ( mos->stack_move[mos->nextBoard++], &chessboard, sizeof ( Tchessboard ) );
#endif
#endif
    if ( tipomove1 & 0x3 ) {
#ifndef PERFT_MODE
      int piece_from = get_piece_at ( SIDE, TABLOG[da] );
#ifdef DEBUG_MODE
      assert ( piece_from != 12 );
      if ( piece_to < 0 || piece_to > 12 ) {
	cout << "piece_to: " << piece_to << endl;
	assert ( 0 );
      };
      assert ( da >= 0 && da < 64 && a >= 0 && a < 64 );
#endif
#endif
      mos->from = ( char ) da;
      mos->to = ( char ) a;
      mos->promotion_piece = ( char ) promotion_piece;
#ifndef PERFT_MODE
      if ( res == 1 ) {
	mos->score = _INFINITE;
      }
      else {
	mos->score = 0;
	mos->score += KillerHeuristic[da][a];
	mos->score += ( PIECES_VALUE[piece_to] >= PIECES_VALUE[piece_from] ) ? ( PIECES_VALUE[piece_to] - PIECES_VALUE[piece_from] ) * 2 : PIECES_VALUE[piece_to];
	ASSERT ( piece_from >= 0 && piece_from < 12 && a >= 0 && a < 64 && da >= 0 && da < 64 );
      }
#endif
    }
    else if ( tipomove1 & 0xC ) {	//castle
      ASSERT ( RIGHT_CASTLE );
#ifndef PERFT_MODE
      mos->score = 100;
#endif
    };
    mos->used = false;
    ASSERT ( gen_list[list_id][0].score < MAX_MOVE );
    return res;
  }
};

#endif
