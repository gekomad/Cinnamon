#ifndef GENMOVES_H_
#define GENMOVES_H_

#include "ChessBoard.h"
using namespace _eval;
class GenMoves:public ChessBoard {
public:
  static const int MAX_PLY = 72;
   GenMoves (  );
   virtual ~ GenMoves (  );
  void setPerft ( const bool b );
  bool generateCaptures ( const int side, u64, u64 );
  void generateMoves ( const int side, const u64 );
   template < int side > bool generateCaptures ( const u64, const u64 );
   template < int side > void generateMoves ( const u64 );
  int getMoveFromSan ( const string fenStr, _Tmove * move );
  void init (  );
  virtual int loadFen (  );
  virtual int loadFen ( string fen );
  void makemove ( _Tmove * move );
  void setRepetitionMapCount ( int i );
  bool performKingShiftCapture ( int side, const u64 enemies );
  bool performKnightShiftCapture ( const int piece, const u64 enemies, const int side );
  bool performDiagCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces );
  bool performRankFileCapture ( const int piece, const u64 enemies, const int side, const u64 allpieces );
   template < int side > bool performPawnCapture ( const u64 enemies );
   template < int side > void performPawnShift ( const u64 xallpieces );
  int performPawnShiftCount ( int side, const u64 xallpieces );
  void performDiagShift ( const int piece, const int side, const u64 allpieces );
  void performRankFileShift ( const int piece, const int side, const u64 allpieces );
  bool makemove ( _Tmove * move, bool rep, bool );
  bool isPinned ( const int side, const uchar Position, const uchar piece );

  void incListId (  ) {
    listId++;
    ASSERT ( listId < MAX_PLY );
    ASSERT ( listId >= 0 );
//        setZero(&gen_list[listId].used);
  } void decListId (  ) {
    gen_list[listId--].size = 0;
  }

  int getListSize (  ) {
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


protected:

  static const u64 RANK_1 = 0xff00ULL;
  static const u64 RANK_3 = 0xff000000ULL;
  static const u64 RANK_4 = 0xff00000000ULL;
  static const u64 RANK_6 = 0xff000000000000ULL;
  static const uchar STANDARD_MOVE_MASK = 0b00000011;
  static const uchar ENPASSANT_MOVE_MASK = 0b00000001;
  static const uchar PROMOTION_MOVE_MASK = 0b00000010;
  static const int MAX_REP_COUNT = 1024;
  int repetitionMapCount;
  u64 *repetitionMap;
  int currentPly;
  bool perftMode, forceCheck;
  u64 numMoves, numMovesq;
  int listId;
  _TmoveP *gen_list;
  _Tmove *getNextMove ( decltype ( gen_list ) );
  u64 getKingAttackers ( const int xside, u64, int );
  void clearKillerHeuristic (  );
  u64 getTotMoves (  );
  int getMobilityBishop ( int, u64, u64 );
  int getMobilityRook ( const int position, const u64 enemies, const u64 friends );
  int getMobilityPawns ( const int side, const int ep, const u64 ped_friends, const u64 enemies, const u64 xallpieces );
  int getMobilityCastle ( const int side, const u64 allpieces );
  int getMobilityQueen ( const int position, const u64 enemies, const u64 friends );
  //template <int side> void attackSquare1(const uchar Position,int* att,const int max);

  template < int side > bool attackSquare ( const uchar Position, u64 );

  void initKillerHeuristic (  );
#ifdef DEBUG_MODE
  int nCutAB, nNullMoveCut, nCutFp, nCutRazor, nCutInsufficientMaterial;
  double betaEfficiencyCumulative, betaEfficiency;
#endif
  void pushRepetition ( u64 );
  int killerHeuristic[64][64];
  template < int side > bool inCheckPerft ( const int from, const int to, const uchar type, const int pieceFrom, const int pieceTo, int promotionPiece );
  void performCastle ( const int side, const uchar type );
  void unPerformCastle ( const int side, const uchar type );
  void tryAllCastle ( const int side, const u64 allpieces );
  void takeback ( _Tmove * move, const u64 oldkey, bool rep );
  template < uchar type > bool pushmove ( const int from, const int to, const int side, int promotionPiece, int pieceFrom );

  template < int side > bool inCheck (  ) {
    return attackSquare < side > ( BITScanForward ( chessboard[KING_BLACK + side] ) );
  }

  template < int side > bool attackSquare ( const uchar position ) {
    return attackSquare < side > ( position, getBitBoard < BLACK > (  ) | getBitBoard < WHITE > (  ) );
  }
  void setKillerHeuristic ( const int from, const int to, const int value ) {
    ASSERT ( from >= 0 && from < 64 && to >= 0 && to < 64 );
    killerHeuristic[from][to] = value;
  }

  void incKillerHeuristic ( const int from, const int to, const int value ) {
    ASSERT ( from >= 0 && from < 64 && to >= 0 && to < 64 );
    ASSERT ( killerHeuristic[from][to] <= killerHeuristic[from][to] + value );
    killerHeuristic[from][to] += value;
  }

private:
  // u64 ALLPIECES;
//    int att[64];
  //  int def[64];

  static const int NO_PROMOTION = -1;
  static const int MAX_MOVE = 130;	// TODO sizeof(u64)*16;//128 bit used
  static const u64 TABJUMPPAWN = 0xFF00000000FF00ULL;
  static const u64 TABCAPTUREPAWN_RIGHT = 0xFEFEFEFEFEFEFEFEULL;
  static const u64 TABCAPTUREPAWN_LEFT = 0x7F7F7F7F7F7F7F7FULL;

  template < int side > void checkJumpPawn ( u64 x, const u64 xallpieces );

  int performRankFileCaptureCount ( const int, const u64 enemies, const u64 allpieces );
  int performDiagCaptureCount ( const int, const u64 enemies, const u64 allpieces );
  int performDiagShiftCount ( const int, const u64 allpieces );
  int performRankFileShiftCount ( const int piece, const u64 allpieces );

  //template <int side> int see( const int to,int) ;
  void popStackMove (  ) {
    ASSERT ( repetitionMapCount > 0 );
    if ( --repetitionMapCount && repetitionMap[repetitionMapCount - 1] == 0 )
      repetitionMapCount--;
  }

  void pushStackMove ( u64 key ) {
    ASSERT ( repetitionMapCount < MAX_REP_COUNT - 1 );
    repetitionMap[repetitionMapCount++] = key;
  }

  //  void attRankFile(int position ,int piece,int* att);
  //void attDiag(int position ,int piece,int* att);

};
#endif
