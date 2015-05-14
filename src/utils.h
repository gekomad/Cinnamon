#ifndef Utils_H_
#define Utils_H_
#include "maindefine.h"
#include <stdint.h>
#include <time.h>
namespace bits {
  extern uchar *ROTATE_LEFT;
  extern uchar *ROTATE_RIGHT;
  void initBit (  );
} namespace _time {
  string getLocalTime (  );
} namespace file {
  string extractFileName ( string path );
  int fileSize ( const string FileName );
  int wc ( string fileName );
} __inline int
bitCount ( u64 bits ) {
  int count = 0;
  while ( bits ) {
    count++;
    bits &= bits - 1;
  }
  return count;
}

#endif
