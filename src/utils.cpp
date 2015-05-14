#include "utils.h"
#include <algorithm>

namespace bits {
  uchar *ROTATE_LEFT;
  uchar *ROTATE_RIGHT;

  void initBit (  ) {
    ROTATE_LEFT = ( uchar * ) malloc ( 32769 );
    ROTATE_RIGHT = ( uchar * ) malloc ( 32833 );

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
}} namespace _time {
  string getLocalTime (  ) {
    struct tm *current;
    time_t now;
     time ( &now );
     current = localtime ( &now );
    string t;

    char tt[30];
     sprintf ( tt, "%i-%i-%i %i:%i:%i", current->tm_year + 1900, current->tm_mon, current->tm_mday, current->tm_hour, current->tm_min, current->tm_sec );
     return string ( tt );
}} namespace file {

  int wc ( string fileName ) {
    ifstream inData;
     inData.open ( fileName.c_str (  ) );
    if ( !inData ) {
      cout << "file not found " << fileName << endl;
      myassert ( 0 );
    } int size = 0;
    string line;
    while ( !inData.eof (  ) ) {
      getline ( inData, line );
      size++;
    }
    inData.close (  );
    return size - 1;
  }
  int fileSize ( const string FileName ) {
    struct stat file;
    if ( !stat ( FileName.c_str (  ), &file ) )
      return file.st_size;
    return 0;
  }

  string extractFileName ( string path ) {
    string token;
    replace ( path.begin (  ), path.end (  ), ':', '/' );
    replace ( path.begin (  ), path.end (  ), '\\', '/' );
    istringstream iss ( path );
    while ( getline ( iss, token, '/' ) );
    return token;
  }

}
