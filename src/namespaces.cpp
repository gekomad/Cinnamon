#include "namespaces.h"
using namespace _bits;
namespace _memory {

#ifdef _WIN32

  void *_mmap ( string fileName ) {
    void *blob;
    HANDLE hfile = CreateFile ( fileName.c_str (  ), FILE_SHARE_READ, 1, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr );
    if ( hfile == INVALID_HANDLE_VALUE )
      return nullptr;
    HANDLE map_handle = CreateFileMapping ( hfile, nullptr, PAGE_READONLY, 0, 0, 0 );
    if ( !map_handle ) {
      CloseHandle ( hfile );
      return nullptr;
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
       return nullptr;
     blob = ( void * ) mmap ( 0, _file::fileSize ( fileName ), PROT_READ, MAP_PRIVATE, fileno ( bookBlob ), 0 );
     fclose ( bookBlob );
     return blob;
  } void _munmap ( void *blob, int fileSize ) {
    munmap ( blob, fileSize );
  }

#endif

}

#include <chrono>
namespace _time {

  int diffTime ( struct timeb t1, struct timeb t2 ) {
    return 1000 * ( t1.time - t2.time ) + t1.millitm - t2.millitm;
  } string getLocalTime (  ) {
    time_t current = chrono::system_clock::to_time_t ( chrono::system_clock::now (  ) );
    return ctime ( &current );
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
    while ( ( a = f.find ( s1 ) ) != ( int ) string::npos ) {
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
  } string extractFileName ( string path ) {
    replace ( path.begin (  ), path.end (  ), ':', '/' );
    replace ( path.begin (  ), path.end (  ), '\\', '/' );
    istringstream iss ( path );
    string token;
    while ( getline ( iss, token, '/' ) );
    return token;
  }

}

namespace _random {
#include <random>
#include <functional>

  function < u64 (  ) > generator;

  void init (  ) {
    uniform_int_distribution < u64 > distribution ( 1, ULLONG_MAX );
    mt19937 engine;
     engine.seed ( time ( NULL ) );
     generator = bind ( distribution, engine );
  } u64 getRandom64 (  ) {
    return generator (  );
  }

}

namespace _bits {
  u64 MASK_BIT_SET[64][64];
  u64 MASK_BIT_SET_NOBOUND[64][64];
  char MASK_BIT_SET_COUNT[64][64];
  char MASK_BIT_SET_NOBOUND_COUNT[64][64];
  u64 **LINK_ROOKS;
  void _free (  ) {
    for ( int i = 0; i < 64; i++ ) {
      free ( LINK_ROOKS[i] );
    } free ( LINK_ROOKS );
  }

  void init (  ) {


    memset ( MASK_BIT_SET, 0, sizeof ( MASK_BIT_SET ) );
    for ( int i = 0; i < 64; i++ ) {
      for ( int j = 0; j < 64; j++ ) {
	int a = min ( i, j );
	int b = max ( i, j );
	MASK_BIT_SET[i][i] = 0;
	for ( int e = a; e <= b; e++ ) {
	  u64 r = ( RANK[i] | POW2[i] ) & ( RANK[j] | POW2[j] );
	  if ( r )
	    MASK_BIT_SET[i][j] |= POW2[e] & r;
	  else {
	    r = ( FILE_[i] | POW2[i] ) & ( FILE_[j] | POW2[j] );
	    if ( r )
	      MASK_BIT_SET[i][j] |= POW2[e] & r;

	    else {
	      r = ( LEFT_DIAG[i] | POW2[i] ) & ( LEFT_DIAG[j] | POW2[j] );
	      if ( r )
		MASK_BIT_SET[i][j] |= POW2[e] & r;

	      else {
		r = ( RIGHT_DIAG[i] | POW2[i] ) & ( RIGHT_DIAG[j] | POW2[j] );
		if ( r )
		  MASK_BIT_SET[i][j] |= POW2[e] & r;
	      }
	    }
	  }

	}
	if ( i == j )
	  MASK_BIT_SET[i][i] &= NOTPOW2[i];
      }
    }
    for ( int i = 0; i < 64; i++ ) {
      for ( int j = 0; j < 64; j++ ) {
	MASK_BIT_SET_NOBOUND[i][j] = MASK_BIT_SET[i][j];
	MASK_BIT_SET_NOBOUND[i][j] &= NOTPOW2[i];
	MASK_BIT_SET_NOBOUND[i][j] &= NOTPOW2[j];
	MASK_BIT_SET[i][j] &= NOTPOW2[i];
      }
    }
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
    for ( int i = 0; i < 64; i++ ) {
      for ( int j = 0; j < 64; j++ ) {
	MASK_BIT_SET_COUNT[i][j] = _bits::bitCount ( MASK_BIT_SET[i][j] );
	MASK_BIT_SET_NOBOUND_COUNT[i][j] = _bits::bitCount ( MASK_BIT_SET_NOBOUND[i][j] );
      }
    }

  }
}
