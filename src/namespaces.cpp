#include "namespaces.h"
using namespace _bits;
namespace _memory {

#ifdef _WIN32

  void *_mmap ( string fileName ) {
    void *blob;
    HANDLE hfile = CreateFile ( fileName.c_str (  ), FILE_SHARE_READ, 0x00000001, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hfile == INVALID_HANDLE_VALUE )
      return NULL;
    HANDLE map_handle = CreateFileMapping ( hfile, NULL, PAGE_READONLY, 0, 0, 0 );
    if ( !map_handle ) {
      CloseHandle ( hfile );
      return NULL;
    } blob = ( void * ) MapViewOfFile ( map_handle, FILE_MAP_READ, 0, 0, 0 );
    CloseHandle ( hfile );
    CloseHandle ( map_handle );
    return blob;
  }

  void _munmap ( void *blob, int fileSize ) {
    UnmapViewOfFile ( blob );
  }

#else

  void *_mmap ( string fileName ) {
    void *blob;
    FILE *bookBlob = fopen ( fileName.c_str (  ), "rb" );
    if ( !bookBlob )
       return NULL;
     blob = ( void * ) mmap ( 0, _file::fileSize ( fileName ), PROT_READ, MAP_PRIVATE, fileno ( bookBlob ), 0 );
     fclose ( bookBlob );
     return blob;
  } void _munmap ( void *blob, int fileSize ) {
    munmap ( blob, fileSize );
  }

#endif

}

namespace _time {

  int diffTime ( struct timeb t1, struct timeb t2 ) {
    return 1000 * ( t1.time - t2.time ) + t1.millitm - t2.millitm;
  } string getLocalTime (  ) {
    struct tm *current;
    time_t now;
    time ( &now );
    current = localtime ( &now );
    char tt[30];
    sprintf ( tt, "%i-%i-%i %i:%i:%i", current->tm_year + 1900, current->tm_mon, current->tm_mday, current->tm_hour, current->tm_min, current->tm_sec );
    return string ( tt );
  }
}

namespace _string {
  void trimRight ( string & str ) {
    string::size_type pos = str.find_last_not_of ( " " );
    str.erase ( pos + 1 );
  } void replace ( string & f, char c1, char c2 ) {
    for ( unsigned i = 0; i < f.size (  ); i++ )
      if ( f[i] == c1 )
	f[i] = c2;
  }

  void replace ( string & f, string s1, string s2 ) {
    int a;
    while ( ( a = f.find ( s1 ) ) != string::npos ) {
      f.replace ( a, s1.size (  ), s2 );
    };
  }
}

namespace _file {
  int fileSize ( const string & FileName ) {
    struct stat file;
    if ( !stat ( FileName.c_str (  ), &file ) )
       return file.st_size;
     return -1;
}}
/*
namespace _random {
u64** RANDOM_KEY;
void init() {
    time_t seed;
    srand ((unsigned) time(&seed));
    RANDOM_KEY=(u64**)malloc(15*sizeof(u64*));
    for(int i=0; i<15; i++) {
        RANDOM_KEY[i]=(u64*)malloc(64*sizeof(u64));
        for(int j=0; j<64; j++) {
            RANDOM_KEY[i][j]=getRandom();
            //cout <<hex<<RANDOM_KEY[i][j]<<endl;
        }
    }
}

void _free() {
    for(int i=0; i<15; i++) {
        free(RANDOM_KEY[i]);
    }
    free(RANDOM_KEY);
}

u64 getRandom() {
    return rand() | (u64)rand() << 15 | (u64)rand() << 30 | (u64)rand() << 45| (u64)rand() << 60;
}

}
*/ namespace _bits {

  u64 **LINK_ROOKS;
  void _free (  ) {
    for ( int i = 0; i < 64; i++ ) {
      free ( LINK_ROOKS[i] );
    } free ( LINK_ROOKS );
  }

  void init (  ) {
    //LINK_ROOKS
    LINK_ROOKS = ( u64 ** ) malloc ( 64 * sizeof ( u64 * ) );
    for ( int i = 0; i < 64; i++ ) {
      LINK_ROOKS[i] = ( u64 * ) malloc ( 64 * sizeof ( u64 ) );
    }
    int from, to;
    for ( int i = 0; i < 64; i++ ) {
      for ( int j = 0; j < 64; j++ ) {
	u64 t = 0;
	if ( RANK[i] & RANK[j] ) {	//rank
	  from = min ( i, j );
	  to = max ( i, j );
	  for ( int k = from + 1; k <= to - 1; k++ ) {
	    t |= POW2[k];
	  }
	}
	else if ( FILE_[i] & FILE_[j] ) {	//file
	  from = min ( i, j );
	  to = max ( i, j );
	  for ( int k = from + 8; k <= to - 8; k += 8 ) {
	    t |= POW2[k];
	  }
	}
	if ( !t )
	  t = 0xffffffffffffffffULL;
	LINK_ROOKS[i][j] = t;
      }
    }
  }
}
