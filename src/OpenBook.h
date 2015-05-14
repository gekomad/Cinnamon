#ifndef OPENBOOK_H
#define OPENBOOK_H
#include <fstream>
#include "Eval.h"

class OpenBook {
public:
  OpenBook (  );
  virtual ~ OpenBook (  );
  bool load (  );
  string search ( int side, string movesPath );
  void create ( string fileWhite, string fileBlack );
private:
  static const int SHIFT = 35;
  string bookFile;
  int fileSize;
  GenMoves *gen;
  char *book;
  string miniBook;
  bool useMmap;
  void create ( string fileIn, ofstream & logFile );
  int sizeBook[2];
  int *random[2];
  bool san2coord ( string san, int *from, int *to, int side );
  int getAttackers ( int piece, int side, int rank, int file, int to );
};
#endif
