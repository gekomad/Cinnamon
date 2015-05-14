#include "Bits.h"

namespace Bits {
  uchar *ROTATE_LEFT;
  uchar *ROTATE_RIGHT;
  char *BITCOUNT;

  int BitCountSlow ( const u64 b ) {
    unsigned buf;
    register unsigned acc;
     buf = ( unsigned ) b;
     acc = buf;
     acc -= ( ( buf &= 0xEEEEEEEEUL ) >> 1 );
     acc -= ( ( buf &= 0xCCCCCCCCUL ) >> 2 );
     acc -= ( ( buf &= 0x88888888UL ) >> 3 );
     buf = ( unsigned ) ( b >> 32 );
     acc += buf;
     acc -= ( ( buf &= 0xEEEEEEEEUL ) >> 1 );
     acc -= ( ( buf &= 0xCCCCCCCCUL ) >> 2 );
     acc -= ( ( buf &= 0x88888888UL ) >> 3 );
     acc = ( acc & 0x0F0F0F0FUL ) + ( ( acc >> 4 ) & 0x0F0F0F0FUL );
     acc = ( acc & 0xFFFF ) + ( acc >> 16 );
     return ( ( acc & 0xFF ) + ( acc >> 8 ) );
  } void initBitCount (  ) {
    ROTATE_LEFT = ( uchar * ) malloc ( 32769 );
    ROTATE_RIGHT = ( uchar * ) malloc ( 32833 );
    BITCOUNT = ( char * ) malloc ( 65536 );

    for ( int t = 0; t < 65536; t++ )
      BITCOUNT[t] = ( char ) BitCountSlow ( t );
    memset ( ROTATE_LEFT, 0, sizeof ( ROTATE_LEFT ) );
    memset ( ROTATE_RIGHT, 0, sizeof ( ROTATE_RIGHT ) );
    ROTATE_LEFT[1] = ROTATE_RIGHT[1] = ROTATE_LEFT[256] = ROTATE_RIGHT[256] = 1;
    ROTATE_LEFT[2] = ROTATE_RIGHT[2] = ROTATE_LEFT[512] = ROTATE_RIGHT[512] = 2;
    ROTATE_LEFT[4] = ROTATE_RIGHT[4] = ROTATE_LEFT[1024] = ROTATE_RIGHT[1024] = 4;
    ROTATE_LEFT[8] = ROTATE_RIGHT[8] = ROTATE_LEFT[2048] = ROTATE_RIGHT[2048] = 8;
    ROTATE_LEFT[16] = ROTATE_RIGHT[16] = ROTATE_LEFT[4096] = ROTATE_RIGHT[4096] = 16;
    ROTATE_LEFT[32] = ROTATE_RIGHT[32] = ROTATE_LEFT[8192] = ROTATE_RIGHT[8192] = 32;
    ROTATE_LEFT[64] = ROTATE_RIGHT[64] = ROTATE_LEFT[16384] = ROTATE_RIGHT[16384] = 64;
    ROTATE_LEFT[128] = ROTATE_RIGHT[128] = ROTATE_LEFT[32768] = ROTATE_RIGHT[32768] = 128;
    ROTATE_LEFT[258] = ROTATE_RIGHT[513] = 3;
    ROTATE_LEFT[516] = ROTATE_RIGHT[1026] = 6;
    ROTATE_LEFT[1032] = ROTATE_RIGHT[2052] = 12;
    ROTATE_LEFT[2064] = ROTATE_RIGHT[4104] = 24;
    ROTATE_LEFT[4128] = ROTATE_RIGHT[8208] = 48;
    ROTATE_LEFT[8256] = ROTATE_RIGHT[16416] = 96;
    ROTATE_LEFT[16512] = ROTATE_RIGHT[32832] = 192;
  }

}
