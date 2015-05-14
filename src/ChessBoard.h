#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_
#include "maindefine.h"
#include "utils.h"
#include "bitmap.h"
#include "Hash.h"

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
  bool reset;
  u64 key;
} _Trep;
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
  u64 starRayKing[2];
  char kingAttacked[64];
  char status;
  int kingSecurityDistance[2];
  u64 isolated[2];
  u64 mobility[64];
#ifdef TUNE_CRAFTY_MODE
  int passed_pawn_score[2];
  int race_pawn_score[2];
#endif
} _Tboard;

class ChessBoard {
public:
  ChessBoard (  );
  virtual ~ ChessBoard (  );
  int loadFen ( string );
  int loadFen (  );
  void display (  );
  u64 makeZobristKey (  );
  void setUci ( bool );
  bool getUci (  );

  uchar getRightCastle (  );
  void setRightCastle ( uchar r );
  void updateZobristKey ( u64 * key, uchar piece, uchar position ) {
#ifndef NO_HASH_MODE
    *( key ) ^= zobrist_key[piece][position];
#endif
  } string decodeBoardinv ( const uchar type, const int a, const int side );

  u64 getBitBoard ( int side ) {
    return ( side ) == BLACK ? ( chessboard[PAWN_BLACK] | chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] | chessboard[KNIGHT_BLACK]
				 | chessboard[KING_BLACK] | chessboard[QUEEN_BLACK] ) : ( chessboard[PAWN_WHITE] | chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] | chessboard[KNIGHT_WHITE] | chessboard[KING_WHITE]
											  | chessboard[QUEEN_WHITE] );
  }

  int getNpieces ( int side ) {
    return bitCount ( getPieces ( ( side ) ) );
  }
  int getPieceAtWhite ( u64 tablogpos );
  int getPieceAtBlack ( u64 tablogpos );
  int getPieceAt ( int side, u64 tablogpos ) {
    return ( ( side ) == WHITE ) ? ( ( chessboard[PAWN_WHITE] & tablogpos ) ? PAWN_WHITE : ( ( chessboard[KING_WHITE] & tablogpos ) ? KING_WHITE : ( ( chessboard[ROOK_WHITE] & tablogpos ) ? ROOK_WHITE : ( ( chessboard[BISHOP_WHITE] & tablogpos ) ? BISHOP_WHITE : ( ( chessboard[KNIGHT_WHITE] & tablogpos ) ? KNIGHT_WHITE : ( ( chessboard[QUEEN_WHITE] & tablogpos ) ? QUEEN_WHITE : SQUARE_FREE ) ) ) ) ) ) : ( ( chessboard[PAWN_BLACK] & tablogpos ) ? PAWN_BLACK : ( ( chessboard[KING_BLACK] & tablogpos ) ? KING_BLACK : ( ( chessboard[ROOK_BLACK] & tablogpos ) ? ROOK_BLACK : ( ( chessboard[BISHOP_BLACK] & tablogpos ) ? BISHOP_BLACK : ( ( chessboard[KNIGHT_BLACK] & tablogpos ) ? KNIGHT_BLACK : ( ( chessboard[QUEEN_BLACK] & tablogpos ) ? QUEEN_BLACK : SQUARE_FREE ) ) ) ) ) );
  }

  int getSide (  ) {
    return sideToMove;
  }
  void setSide ( bool b ) {
    sideToMove = b;
  }

protected:
  Tchessboard chessboard;
  int enpassantPosition;
  // int repetitionMapCount;
  bool sideToMove;
  uchar RIGHT_CASTLE;

  _Tboard structure;
  string INITIAL_FEN;
  bool uci;

  uchar rotateBoardLeft45 ( const u64, const int );
  uchar rotateBoardRight45 ( const u64, const int );
  int friendKing[2];
  void boardToFen ( string & fen );
  u64 getPieces ( int side ) {
    return ( side ) == BLACK ? chessboard[ROOK_BLACK] | chessboard[BISHOP_BLACK] | chessboard[KNIGHT_BLACK] | chessboard[KING_BLACK]
      | chessboard[QUEEN_BLACK] : chessboard[ROOK_WHITE] | chessboard[BISHOP_WHITE] | chessboard[KNIGHT_WHITE] | chessboard[KING_WHITE] | chessboard[QUEEN_WHITE];
  }

};

#endif
