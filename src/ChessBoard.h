#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_
#include "maindefine.h"
#include "Bits.h"
#include "bitmap.h"
#ifdef DEBUG_MODE
#include <map>
#endif

#define SQUARE_FREE 12
#define PAWN_BLACK 0
#define PAWN_WHITE 1
#define ROOK_BLACK 2
#define ROOK_WHITE 3
#define BISHOP_BLACK 4
#define BISHOP_WHITE 5
#define KNIGHT_BLACK 6
#define KNIGHT_WHITE 7
#define KING_BLACK 8
#define KING_WHITE 9
#define QUEEN_BLACK 10
#define QUEEN_WHITE 11

typedef struct {
  u64 allPieces;
  unsigned short posKing[2];
  uchar attacKingCount[2];
  u64 allPiecesSide[2];
  u64 pawns[2];
  u64 queens[2];
  u64 rooks[2];
  u64 openColumn;
  u64 semiOpenColumn[2];
  char kingAttacked[64];
  char status;
  int kingSecurityDistance[2];
  u64 isolated[2];
  u64 mobility[64];
#ifdef TEST_MODE
  int passed_pawn_score[2];
  int race_pawn_score[2];
#endif
} STRUCTURE_TAG;
class ChessBoard {

public:
  ChessBoard ( char * );
   virtual ~ ChessBoard (  );
  Tchessboard chessboard;
  int enpassantPosition;
  STRUCTURE_TAG *getStructure (  );
  void print ( char *s );
  int loadFen ( char * );
  void print (  );
  u64 makeZobristKey ( const int side );
  void setUci (  );
  bool getUci (  );
  void setBlackMove ( bool b );
  int getInvFen ( char a );
  uchar getRightCastle (  );
  void setRightCastle ( uchar r );
  const char *decodeBoardinv ( const uchar type, const int a, const int side );
  char decodeBoard ( char *a );
#ifdef TEST_MODE
  char test_ris[20];
  char test_found[20];
  u64 num_moves_test;
#endif
  bool isBlackMove (  ) {
    return blackMove;
  } u64 squareBitOccupied ( int side ) {
    return ( side ) == BLACK ? ( chessboard[PAWN_BLACK] | chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] | chessboard[KNIGHT_BLACK]
				 | chessboard[KING_BLACK] | chessboard[QUEEN_BLACK] ) : ( chessboard[PAWN_WHITE] | chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] | chessboard[KNIGHT_WHITE] | chessboard[KING_WHITE]
											  | chessboard[QUEEN_WHITE] );
  }

  u64 squareAllBitOccupied (  ) {
    return chessboard[PAWN_BLACK] | chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] | chessboard[KNIGHT_BLACK] | chessboard[KING_BLACK]
      | chessboard[QUEEN_BLACK] | chessboard[PAWN_WHITE] | chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] | chessboard[KNIGHT_WHITE]
      | chessboard[KING_WHITE] | chessboard[QUEEN_WHITE];
  }
  int n_pieces ( int side ) {
    return BitCount ( getPieces ( ( side ) ) );
  }
  int get_piece_at ( int side, u64 tablogpos ) {
    return ( ( side ) == WHITE ) ? ( ( chessboard[PAWN_WHITE] & tablogpos ) ? PAWN_WHITE : ( ( chessboard[KING_WHITE] & tablogpos ) ? KING_WHITE : ( ( chessboard[ROOK_WHITE] & tablogpos ) ? ROOK_WHITE : ( ( chessboard[BISHOP_WHITE] & tablogpos ) ? BISHOP_WHITE : ( ( chessboard[KNIGHT_WHITE] & tablogpos ) ? KNIGHT_WHITE : ( ( chessboard[QUEEN_WHITE] & tablogpos ) ? QUEEN_WHITE : SQUARE_FREE ) ) ) ) ) ) : ( ( chessboard[PAWN_BLACK] & tablogpos ) ? PAWN_BLACK : ( ( chessboard[KING_BLACK] & tablogpos ) ? KING_BLACK : ( ( chessboard[ROOK_BLACK] & tablogpos ) ? ROOK_BLACK : ( ( chessboard[BISHOP_BLACK] & tablogpos ) ? BISHOP_BLACK : ( ( chessboard[KNIGHT_BLACK] & tablogpos ) ? KNIGHT_BLACK : ( ( chessboard[QUEEN_BLACK] & tablogpos ) ? QUEEN_BLACK : SQUARE_FREE ) ) ) ) ) );
  }

  int getSide (  ) {
    return !blackMove;
  }
protected:
  uchar RIGHT_CASTLE;
  STRUCTURE_TAG structure;
  bool uci;
  bool blackMove;
#ifdef DEBUG_MODE
  map < u64, Tchessboard * >zobristKeysMap;
  bool checkZobristKey ( u64 key, Tchessboard * chessboard );
#endif
  int Friend_king[2];

  int loadFen ( char *ss, int check );
  void BoardToFEN ( char *FEN );
  void print ( Tchessboard * s );
  char *decodeType ( uchar );
  u64 getPieces ( int side ) {
    return ( side ) == BLACK ? chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] | chessboard[KNIGHT_BLACK] | chessboard[KING_BLACK]
      | chessboard[QUEEN_BLACK] : chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] | chessboard[KNIGHT_WHITE] | chessboard[KING_WHITE] | chessboard[QUEEN_WHITE];
  }
#if defined(PERFT_MODE) || defined(HASH_MODE)
  void updateZobristKey ( u64 * key, int piece, int position ) {
    *( key ) ^= zobrist_key[piece][position];
  }
#else
  void updateZobristKey ( u64 * key, int piece, int position ) {
    ;
  }
#endif
#ifdef TEST_MODE
  int extractTestResult ( char *ris1 );
  void getTestResult ( char *ss );
#endif

};

#endif
