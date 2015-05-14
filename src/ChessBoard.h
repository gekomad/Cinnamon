#ifndef CHESSBOARD_H_
#define CHESSBOARD_H_
#include <iostream>
#include <sstream>
#include <string.h>
#include "Hash.h"
#include "namespaces.h"

using namespace _bits;
using namespace _board;

class ChessBoard {
public:

  ChessBoard (  );
  virtual ~ ChessBoard (  );
  static const u64 CENTER_MASK = 0x1818000000ULL;
  static const u64 BIG_DIAG_LEFT = 0x102040810204080ULL;
  static const u64 BIG_DIAG_RIGHT = 0x8040201008040201ULL;
  static const int SQUARE_FREE = 12;
  static const int PAWN_BLACK = 0;
  static const int PAWN_WHITE = 1;
  static const int ROOK_BLACK = 2;
  static const int ROOK_WHITE = 3;
  static const int BISHOP_BLACK = 4;
  static const int BISHOP_WHITE = 5;
  static const int KNIGHT_BLACK = 6;
  static const int KNIGHT_WHITE = 7;
  static const int KING_BLACK = 8;
  static const int KING_WHITE = 9;
  static const int QUEEN_BLACK = 10;
  static const int QUEEN_WHITE = 11;
  string START_FEN;
  void display (  );
  char decodeBoard ( string );
  void setUci ( bool );
  bool getUci (  );
  int getPieceByChar ( char );
#ifdef DEBUG_MODE
  u64 getBitBoard ( int side );
#endif
   template < int side > u64 getBitBoard (  ) {
    return chessboard[PAWN_BLACK + side] | chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side]
      | chessboard[KING_BLACK + side] | chessboard[QUEEN_BLACK + side];
  } void setSide ( bool b ) {
    sideToMove = b;
  }

  int getSide (  ) {
    return sideToMove;
  }

  template < int side > int getPieceAt ( u64 bitmapPos ) {
    return ( ( chessboard[PAWN_BLACK + side] & bitmapPos ) ? PAWN_BLACK + side : ( ( chessboard[KING_BLACK + side] & bitmapPos ) ? KING_BLACK + side : ( ( chessboard[ROOK_BLACK + side] & bitmapPos ) ? ROOK_BLACK + side : ( ( chessboard[KNIGHT_BLACK + side] & bitmapPos ) ? KNIGHT_BLACK + side : ( ( chessboard[BISHOP_BLACK + side] & bitmapPos ) ? BISHOP_BLACK + side : ( ( chessboard[QUEEN_BLACK + side] & bitmapPos ) ? QUEEN_BLACK + side : SQUARE_FREE ) ) ) ) ) );
  }
protected:

  typedef struct {
    u64 allPieces;
    unsigned short posKing[2];
    u64 kingAttackers[2];
    u64 allPiecesSide[2];
    u64 pawns[2];
    u64 queens[2];
    u64 rooks[2];
    u64 openColumn;
    u64 semiOpenColumn[2];
    int kingSecurityDistance[2];
    u64 isolated[2];
  } _Tboard;

  static const int E1 = 3;
  static const int E8 = 59;
  static const int C1 = 5;
  static const int F1 = 2;
  static const int C8 = 58;
  static const int F8 = 61;
  static const u64 BLACK_SQUARES = 0x55AA55AA55AA55AAULL;
  static const u64 WHITE_SQUARES = 0xAA55AA55AA55AA55ULL;
  static const uchar KING_SIDE_CASTLE_MOVE_MASK = 0 b00000100;
  static const uchar QUEEN_SIDE_CASTLE_MOVE_MASK = 0 b00001000;
  static const uchar RIGHT_KING_CASTLE_WHITE_MASK = 0 b00010000;
  static const uchar RIGHT_QUEEN_CASTLE_WHITE_MASK = 0 b00100000;
  static const uchar RIGHT_KING_CASTLE_BLACK_MASK = 0 b01000000;
  static const uchar RIGHT_QUEEN_CASTLE_BLACK_MASK = 0 b10000000;
  u64 zobristKey;
  int enpassantPosition;
  uchar RIGHT_CASTLE;
  u64 chessboard[12];
  _Tboard structure;
  bool sideToMove;
  int friendKing[2];
  virtual int loadFen ( string );
  string decodeBoardinv ( const uchar type, const int a, const int side );
  u64 makeZobristKey (  );

  template < int side > int getNpiecesNoPawnNoKing (  ) {
    return _bits::bitCount ( chessboard[ROOK_BLACK + side] | chessboard[BISHOP_BLACK + side] | chessboard[KNIGHT_BLACK + side] | chessboard[QUEEN_BLACK + side] );
  }



  void updateZobristKey ( u64 * key, uchar piece, uchar position ) {
    // *(key) ^= _random::RANDOM_KEY[piece][position];
    *( key ) ^= RANDOM_KEY[piece][position];
  }
#ifdef DEBUG_MODE
  int getPieceAt ( int side, u64 bitmapPos );
#endif
private:
  void setRightCastle ( uchar r );
  int loadFen (  );
  bool uci;
  uchar getRightCastle (  );
  void boardToFen ( string & fen );
};
#endif
