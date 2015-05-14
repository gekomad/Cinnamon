#ifndef GENMOVES_H_
#define GENMOVES_H_

#include "ChessBoard.h"
using namespace _eval;
class GenMoves:public ChessBoard {
public:

  GenMoves (  );
  virtual ~ GenMoves (  );
  static const int MAX_PLY = 64;
  void setPerft ( bool b );
  bool generateCaptures ( const int side, u64, u64, u64 * key );
  void generateMoves ( const int side, u64 );
   template < int side > bool generateCaptures ( u64, u64, u64 * key );
   template < int side > void generateMoves ( u64 );
  int getMoveFromSan ( const string fenStr, _Tmove * move );
  void init (  );
  virtual int loadFen (  );
  virtual int loadFen ( string fen );
  void makemove ( _Tmove * move );
  void setRepetitionMapCount ( int i );
  bool performKingShiftCapture ( int side, const u64 enemies );
  bool performKnightShiftCapture ( const int piece, const u64 enemies, const int side );
  bool performBishopCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces );
  bool performRookQueenCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces );
   template < int side > bool performPawnCapture ( const u64 enemies, u64 * key );
   template < int side > void performPawnShift ( const u64 xallpieces );
  void performBishopShift ( const int piece, const int side, const u64 allpieces );
  void performRookQueenShift ( const int piece, const int side, const u64 allpieces );

  void incListId (  ) {
    listId++;
    ASSERT ( listId < MAX_PLY && listId >= 0 );
//        setZero(&gen_list[listId].used);
  } void decListId (  ) {
    listId--;
  }

  int getListCount (  ) {
    return gen_list[listId].size;
  }

  void pushStackMove (  ) {
    pushStackMove ( zobristKey );
  }

  _Tmove *getMove ( int i ) {
    return &gen_list[listId].moveList[i];
  }

  void resetList (  ) {
    gen_list[listId].size = 0;
  }

  void makemove ( _Tmove * move, u64 * key, bool rep );
  bool isPinned ( const int side, const uchar Position, const uchar piece );
protected:
  static const u64 RANK_1 = 0xff00ULL;
  static const u64 RANK_3 = 0xff000000ULL;
  static const u64 RANK_4 = 0xff00000000ULL;
  static const u64 RANK_6 = 0xff000000000000ULL;
  static const uchar STANDARD_MOVE_MASK = 0 b00000011;
  static const uchar ENPASSANT_MOVE_MASK = 0 b00000001;
  static const uchar PROMOTION_MOVE_MASK = 0 b00000010;
  static const int MAX_REP_COUNT = 512;
  int repetitionMapCount;
  u64 *repetitionMap;
  int currentPly;
  bool perftMode;
  u64 numMoves, numMovesq;
  int listId;
  _TmoveP *gen_list;
  _Tmove *getNextMove ( _TmoveP * );
  u64 getKingAttackers ( const int side );

  /*    template <int side> int evaluateMobility() {
     evaluateMobilityMode=true;
     incListId();
     u64 dummy=0;
     generateCaptures( side,structure.allPiecesSide[side^1],structure.allPiecesSide[side],&dummy);
     generateMoves( side,structure.allPieces);
     int listcount = getListCount();
     if (listcount == -1)
     listcount = 0;
     resetList();
     decListId();
     evaluateMobilityMode=false;
     return listcount;
     } */
  void clearKillerHeuristic (  );
  u64 getTotMoves (  );
  template < int side > bool attackSquare ( const uchar Position );
  void initKillerHeuristic (  );
#ifdef DEBUG_MODE
  int nCutAB, nNullMoveCut, nCutFp, nCutRazor;
  double betaEfficencyCumulative, betaEfficency;
#endif
  void pushRepetition ( u64 );
  int killerHeuristic[64][64];
//    bool evaluateMobilityMode;
  template < int side > bool inCheck ( const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, int promotionPiece );
  void performCastle ( const int side, const uchar type, u64 * key );
  void unPerformCastle ( const int side, const uchar type );
  void tryAllCastle ( const int side, const u64 allpieces );
  void setKillerHeuristic ( int from, int to, int value );
  void incKillerHeuristic ( int from, int to, int value );
  void takeback ( _Tmove * move, u64 * key, const u64 oldkey, bool rep );
  template < uchar type > bool pushmove ( const int from, const int to, const int side, int promotionPiece, int pieceFrom );

  template < int side > bool inCheck (  ) {
    return attackSquare < side > ( BITScanForward ( chessboard[KING_BLACK + side] ) );
  }

private:

  static const int NO_PROMOTION = -1;
  static const int MAX_MOVE = 130;	// TODO sizeof(u64)*16;//128 bit used
  static const u64 TABJUMPPAWN = 0xFF00000000FF00ULL;
  static const u64 TABCAPTUREPAWN_RIGHT = 0xFEFEFEFEFEFEFEFEULL;
  static const u64 TABCAPTUREPAWN_LEFT = 0x7F7F7F7F7F7F7F7FULL;

  template < int side > void checkJumpPawn ( u64 x, const u64 xallpieces );

  void popStackMove (  ) {
    ASSERT ( repetitionMapCount > 0 );
    if ( --repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0 )
      repetitionMapCount--;
  }

  void pushStackMove ( u64 key ) {
    ASSERT ( repetitionMapCount < MAX_REP_COUNT - 1 );
    repetitionMap[repetitionMapCount++] = key;
  }
  /*
     void setEvaluateMobilityMode(bool b) {
     evaluateMobilityMode = b;
     } */
};
#endif
