#ifndef Bits_H_
#define Bits_H_
#include "maindefine.h"
namespace Bits {
  extern uchar *ROTATE_LEFT;
  extern uchar *ROTATE_RIGHT;
  extern char *BITCOUNT;
  void initBitCount (  );
} __inline int
BitCount ( u64 bits ) {
  return Bits::BITCOUNT[( unsigned short ) ( bits )] + Bits::BITCOUNT[( ( unsigned ) ( bits ) ) >> 16] + Bits::BITCOUNT[( unsigned short ) ( ( bits ) >> 32 )]
    + Bits::BITCOUNT[( bits ) >> 48];
}

#ifdef HAS_64BITS
__inline int
BITScanForward ( u64 bits ) {
  return ( __builtin_ffsll ( bits ) - 1 );
}

#else
__inline int
BITScanForward ( u64 bits ) {
  return ( ( unsigned ) ( bits ) ) ? __builtin_ffs ( ( unsigned ) ( bits ) ) - 1 : __builtin_ffs ( ( bits ) >> 32 ) + 31;
}

#endif
int BitCountSlow ( const u64 b );

#endif
