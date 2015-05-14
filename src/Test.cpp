#ifdef TEST_MODE
#include "Test.h"

Test::Test (  ) {
  search = new Search ( NULL );
  it = new IterativeDeeping ( search );
}

Test::~Test (  ) {
  delete search;
  delete it;
}

int
Test::fileSize ( const char *FileName ) {
  struct stat file;
  if ( !stat ( FileName, &file ) )
    return file.st_size;
  return 0;
}

int
Test::wc ( const char *fileName ) {
  ifstream inData;
  inData.open ( fileName );
  if ( !inData )
    myassert ( 0 );
  int size = 0;
  string line;
  while ( !inData.eof (  ) ) {
    getline ( inData, line );
    size++;
  }
  return size;
}

/*example output crafty:
 material.......  -1.95
 pawns..........  -0.12
 passed pawns...  -0.27
 knights........   0.00
 bishops........   0.02
 rooks..........   0.22
 queens.........   0.00
 kings..........   0.00
 development....   0.00
 pawn races.....   0.00
 total..........  -2.10*/

void
Test::extract_crafty_scores ( const char *PATH_LOG, float *crafty_material, float *crafty_pawns, float *crafty_passed_pawns, float *crafty_knights, float *crafty_bishop, float *crafty_rooks, float *crafty_queens, float *crafty_kings, float *crafty_development, float *crafty_pawn_race, float *crafty_total ) {
  char buf[300];
  FILE *streamLog;
  streamLog = fopen ( PATH_LOG, "r" );
  myassert ( streamLog );
  while ( fgets ( buf, sizeof ( buf ), streamLog ) != NULL ) {
    if ( strstr ( buf, "material......." ) )
      *crafty_material = atof ( buf + 15 );
    if ( strstr ( buf, "pawns.........." ) )
      *crafty_pawns = atof ( buf + 15 );
    if ( strstr ( buf, "passed pawns..." ) )
      *crafty_passed_pawns = atof ( buf + 15 );
    if ( strstr ( buf, "knights........" ) )
      *crafty_knights = atof ( buf + 15 );
    if ( strstr ( buf, "bishops........" ) )
      *crafty_bishop = atof ( buf + 15 );
    if ( strstr ( buf, "rooks.........." ) )
      *crafty_rooks = atof ( buf + 15 );
    if ( strstr ( buf, "queens........." ) )
      *crafty_queens = atof ( buf + 15 );
    if ( strstr ( buf, "kings.........." ) )
      *crafty_kings = atof ( buf + 15 );
    if ( strstr ( buf, "development...." ) )
      *crafty_development = atof ( buf + 15 );
    if ( strstr ( buf, "pawn races....." ) )
      *crafty_pawn_race = atof ( buf + 15 );
    if ( strstr ( buf, "total.........." ) )
      *crafty_total = atof ( buf + 15 );
  }
  fclose ( streamLog );
}

typedef struct {
  Tchessboard chessboard;
  u64 RIGHT_CASTLE;
  int ENP_POSSIBILE;

  int Friend_king[2];
  char side;
} CACHE_INIT_TAG;
typedef struct {
  char fen[200];
  float crafty_material;
  float crafty_pawns;
  float crafty_passed_pawns;
  float crafty_knights;
  float crafty_bishop;
  float crafty_rooks;
  float crafty_queens;
  float crafty_kings;
  float crafty_development;
  float crafty_pawn_race;
  float crafty_total;
} CRAFTY_TAG;
void
Test::test_eval_crafty_tuning ( const char *testfile, bool create_crafy_score_bin ) {
  int CD = rand (  );

  const char *PATH_CRAFTY_BIN = "/home/geko/chess/crafy_score.bin";
  int wc_file = 0;

  int butterfly_material;
  int butterfly_pawns;
  int butterfly_passed_pawns;
  int butterfly_knights;
  int butterfly_bishop;
  int butterfly_rooks;
  int butterfly_queens;
  int butterfly_kings;
  int butterfly_development;
  int butterfly_pawn_race;
  int butterfly_total;
  FILE *stream;
  char line[2001];
  char buf[2001];
  int side;
  if ( create_crafy_score_bin ) {
    const char *PATH_LOG = "/tmp/ramdisk/crafty.log";	//to speed up uses RAMDISK
    const char *PATH_CRAFTY = "/tmp/ramdisk/crafty";
    wc_file = wc ( testfile );
    CRAFTY_TAG *crafy_score = ( CRAFTY_TAG * ) calloc ( wc_file, sizeof ( CRAFTY_TAG ) );
    strcpy ( line, testfile );
    stream = fopen ( line, "r" );
    myassert ( stream );

    int count = 0;
    while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
      if ( !( count % 50 ) )
	cout << count << " " << endl;
      for ( int i = 0; i < strlen ( line ); i++ ) {
	if ( line[i] == '\n' || line[i] == '\r' || line[i] == '\"' ) {
	  line[i] = 0;
	  break;
	}
      }
      sprintf ( buf, "%s log=off ponder=off setboard \"%s\" score quit>%s", PATH_CRAFTY, line, PATH_LOG );

      int dummy = system ( buf );
      if ( dummy == -1 )
	myassert ( 0 );
      extract_crafty_scores ( PATH_LOG, &crafy_score[count].crafty_material, &crafy_score[count].crafty_pawns, &crafy_score[count].crafty_passed_pawns, &crafy_score[count].crafty_knights, &crafy_score[count].crafty_bishop, &crafy_score[count].crafty_rooks, &crafy_score[count].crafty_queens, &crafy_score[count].crafty_kings, &crafy_score[count].crafty_development, &crafy_score[count].crafty_pawn_race, &crafy_score[count].crafty_total );

      crafy_score[count].crafty_material *= 100;
      crafy_score[count].crafty_pawns *= 100;
      crafy_score[count].crafty_passed_pawns *= 100;
      crafy_score[count].crafty_knights *= 100;
      crafy_score[count].crafty_bishop *= 100;
      crafy_score[count].crafty_rooks *= 100;
      crafy_score[count].crafty_queens *= 100;
      crafy_score[count].crafty_kings *= 100;
      crafy_score[count].crafty_development *= 100;
      crafy_score[count].crafty_pawn_race *= 100;
      crafy_score[count].crafty_total *= 100;
      strcpy ( crafy_score[count].fen, line );
      count++;
    }
    fclose ( stream );
    stream = fopen ( PATH_CRAFTY_BIN, "wb" );
    fwrite ( crafy_score, wc_file, sizeof ( CRAFTY_TAG ), stream );
    fclose ( stream );
    return;
  }				//end create_crafy_score_bin
  ////////////////////////////////
  char tuning_file[255];
  char log_file[255];
  int Friend_king[2];
  sprintf ( tuning_file, "tuning%d.txt", CD );
  sprintf ( log_file, "log%d.txt", CD );
  CRAFTY_TAG *crafy_score;
  stream = fopen ( PATH_CRAFTY_BIN, "rb" );
  wc_file = fileSize ( PATH_CRAFTY_BIN ) / sizeof ( CRAFTY_TAG );
  crafy_score = ( CRAFTY_TAG * ) malloc ( wc_file * sizeof ( CRAFTY_TAG ) );
  fread ( crafy_score, wc_file, sizeof ( CRAFTY_TAG ), stream );
  fclose ( stream );
  ///////////CACHE_INIT_TAG

  CACHE_INIT_TAG *cache_init = ( CACHE_INIT_TAG * ) malloc ( wc_file * sizeof ( CACHE_INIT_TAG ) );
  printf ( "\nrecord: %d", wc_file );
  for ( int i = 0; i < wc_file; i++ ) {
    side = search->loadFen ( crafy_score[i].fen );
    cache_init[i].RIGHT_CASTLE = search->getRightCastle (  );

    cache_init[i].ENP_POSSIBILE = search->enpassantPosition;
    memcpy ( cache_init[i].Friend_king, Friend_king, sizeof ( Friend_king ) );
    cache_init[i].side = search->getSide (  );
    memcpy ( cache_init[i].chessboard, search->chessboard, sizeof ( search->chessboard ) );

  }
  //////////////////////////
  int param_count = 0;
  string line2;
  ofstream fout ( tuning_file, ios::app );
  ofstream flog ( log_file, ios::app );

  u64 start = 0, tot, min = 0x8000000000000000ULL;
  while ( 1 ) {
    if ( !( param_count % 100 ) )
      cout << param_count << endl;
    tot = 0;
    for ( int i = 0; i < wc_file; i++ ) {
      search->init (  );
      search->setRightCastle ( cache_init[i].RIGHT_CASTLE );
      search->enpassantPosition = cache_init[i].ENP_POSSIBILE;

      memcpy ( Friend_king, cache_init[i].Friend_king, sizeof ( Friend_king ) );
      side = cache_init[i].side;
      memcpy ( search->chessboard, cache_init[i].chessboard, sizeof ( search->chessboard ) );

      search->getScore ( side,
#ifdef FP_MODE
			 -_INFINITE, _INFINITE,
#endif
			 &butterfly_material, &butterfly_pawns, &butterfly_passed_pawns, &butterfly_knights, &butterfly_bishop, &butterfly_rooks, &butterfly_queens, &butterfly_kings, &butterfly_development, &butterfly_pawn_race, &butterfly_total );
      tot += sqrt ( pow ( butterfly_pawns - crafy_score[i].crafty_pawns, 2 ) );
    }
    if ( !param_count )
      start = tot;
    if ( tot < min ) {
      min = tot;
      search->writeParam ( tuning_file, param_count, true );
      flog << tot << " MSE " << "param" << param_count << ".txt " << 100 - ( min * 100 / start ) << "%" << endl;
      cout << tot << " MSE " << "param" << param_count << ".txt " << 100 - ( min * 100 / start ) << "%" << endl;
      flog.flush (  );
      cout.flush (  );
    }
    param_count++;
    search->setRandomParam (  );
  }
  fout.close (  );
  flog.close (  );
}

void
Test::test_eval_crafty ( const char *testfile ) {
  /*
     CREATE TABLE `eval` (
     `id` int(11) NOT NULL,
     `fen` text NOT NULL,
     `butterfly_material` int(11) NOT NULL,
     `butterfly_pawns` int(11) NOT NULL,
     `butterfly_passed_pawns` int(11) NOT NULL,
     `butterfly_knights` int(11) NOT NULL,
     `butterfly_bishop` int(11) NOT NULL,
     `butterfly_rooks` int(11) NOT NULL,
     `butterfly_queens` int(11) NOT NULL,
     `butterfly_kings` int(11) NOT NULL,
     `butterfly_development` int(11) NOT NULL,
     `butterfly_pawn_race` int(11) NOT NULL,
     `butterfly_total` int(11) NOT NULL,
     `crafty_material` double NOT NULL,
     `crafty_pawns` double NOT NULL,
     `crafty_passed_pawns` double NOT NULL,
     `crafty_knights` double NOT NULL,
     `crafty_bishop` double NOT NULL,
     `crafty_rooks` double NOT NULL,
     `crafty_queens` double NOT NULL,
     `crafty_kings` double NOT NULL,
     `crafty_development` double NOT NULL,
     `crafty_pawn_race` double NOT NULL,
     `crafty_total` double NOT NULL,
     PRIMARY KEY (`id`)
     ) ENGINE=MyISAM DEFAULT CHARSET=latin1
   */
  float crafty_material;
  float crafty_pawns;
  float crafty_passed_pawns;
  float crafty_knights;
  float crafty_bishop;
  float crafty_rooks;
  float crafty_queens;
  float crafty_kings;
  float crafty_development;
  float crafty_pawn_race;
  float crafty_total;
  int butterfly_material;
  int butterfly_pawns;
  int butterfly_passed_pawns;
  int butterfly_knights;
  int butterfly_bishop;
  int butterfly_rooks;
  int butterfly_queens;
  int butterfly_kings;
  int butterfly_development;
  int butterfly_pawn_race;
  int butterfly_total;
  FILE *stream;
  FILE *evalcsv;
  //const char* PATH_LOG = "/tmp/ramdisk/crafty.log";const char* PATH_CRAFTY = "/tmp/ramdisk/crafty";
  const char *PATH_LOG = "/home/geko/crafty.log";	//to speed up uses RAMDISK to store this file
  const char *PATH_CRAFTY = "/home/geko/chess/crafty";

  //const char* PATH_LOG="r:\\crafty.log";const char* PATH_CRAFTY="r:\\crafty-23.4-win32.exe";
  cout << "\n ****** START test_eval_crafty " << testfile << "  *******";
  cout << "\nTo speed up put crafty.log in RAMDISK:\n\ton linux: mkdir /tmp/ramdisk; chmod 777 /tmp/ramdisk;sudo mount -t tmpfs -o size=5M tmpfs /tmp/ramdisk/";

  cout << "\n\ton windows: http://www.mydigitallife.info/free-ramdisk-for-windows-vista-xp-2000-and-2003-server";

  cout << "\n\nLoad eval.csv in mysql: mysqlimport --local -u root -p username --fields-terminated-by='|' eval.csv";

  cout
    << "\nand calculates Mean Square Error:"
    "\nSELECT fen, butterfly_bishop, crafty_bishop *100, POW( butterfly_bishop - crafty_bishop *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
    "\nSELECT fen, butterfly_kings, crafty_kings *100, POW( butterfly_kings - crafty_kings *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
    "\nSELECT fen, butterfly_knights, crafty_knights *100, POW( butterfly_knights - crafty_knights *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
    "\nSELECT fen, butterfly_material, crafty_material *100, POW( butterfly_material - crafty_material *100, 2 ) FROM  `eval` ORDER BY 4 DESC"
    "\nSELECT fen, butterfly_passed_pawns, crafty_passed_pawns *100, POW( butterfly_passed_pawns - crafty_passed_pawns *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_pawns, crafty_pawns *100, POW( butterfly_pawns - crafty_pawns *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_pawn_race, crafty_pawn_race *100, POW( butterfly_pawn_race - crafty_pawn_race *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_queens, crafty_queens *100, POW( butterfly_queens - crafty_queens *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_rooks, crafty_rooks *100, POW( butterfly_rooks - crafty_rooks *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT fen, butterfly_total, crafty_total *100, POW( butterfly_total - crafty_total *100, 2 ) FROM  `eval` ORDER BY 4 DESC" "\nSELECT sum(pow(butterfly_total-crafty_total*100,2))/(select count(1) from eval )from eval\n" "\ngenerating eval.csv...";

  cout << flush;
  struct timeb start, end;
  ftime ( &start );
  int count = 0;
  //      num_tot_moves = 0;
  char line[2001];
  char buf[2001];
  int side;
  evalcsv = fopen ( "eval.csv", "w" );
  if ( !evalcsv )
    myassert ( 0 );
  strcpy ( line, testfile );
  stream = fopen ( line, "r" );
  if ( !stream ) {
    memset ( line, 0, sizeof ( line ) );
    strcpy ( line, "../" );
    strcat ( line, testfile );
    stream = fopen ( line, "r" );
  }
  if ( !stream )
    myassert ( 0 );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    if ( ( line[strlen ( line ) - 1] = '\n' ) )
      line[strlen ( line ) - 1] = 0;
    if ( ( line[strlen ( line ) - 1] = '\r' ) )
      line[strlen ( line ) - 1] = 0;
    side = search->loadFen ( line );
    count++;
    search->getScore ( side,
#ifdef FP_MODE
		       -_INFINITE, _INFINITE,
#endif
		       &butterfly_material, &butterfly_pawns, &butterfly_passed_pawns, &butterfly_knights, &butterfly_bishop, &butterfly_rooks, &butterfly_queens, &butterfly_kings, &butterfly_development, &butterfly_pawn_race, &butterfly_total );
    sprintf ( buf, "%s log=off ponder=off setboard \"%s\" score quit>%s", PATH_CRAFTY, line, PATH_LOG );
    int dummy = system ( buf );
    extract_crafty_scores ( PATH_LOG, &crafty_material, &crafty_pawns, &crafty_passed_pawns, &crafty_knights, &crafty_bishop, &crafty_rooks, &crafty_queens, &crafty_kings, &crafty_development, &crafty_pawn_race, &crafty_total );
    fprintf ( evalcsv, "%d|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f|%f\n", count, line, butterfly_material, butterfly_pawns, butterfly_passed_pawns, butterfly_knights, butterfly_bishop, butterfly_rooks, butterfly_queens, butterfly_kings, butterfly_development, butterfly_pawn_race, butterfly_total, crafty_material, crafty_pawns, crafty_passed_pawns, crafty_knights, crafty_bishop, crafty_rooks, crafty_queens, crafty_kings, crafty_development, crafty_pawn_race, crafty_total );
  }
  fclose ( stream );
  fclose ( evalcsv );
  ftime ( &end );
  cout << "\nEND test, time:  " << diff_time ( end, start ) << " total" << flush;
}

void
Test::test_epd ( char *testfile, int maxTimeMillsec ) {
  cout << "\n ****** START TEST " << testfile << " max time " << maxTimeMillsec / 1000 << " sec *******" << flush;
  struct timeb start, end;
  ftime ( &start );
  int count = 0;
  int foundit = 0;
  int num_tot_moves = 0;
  FILE *stream;
  char line[2001];
  strcpy ( line, testfile );
  stream = fopen ( line, "r" );
  if ( !stream ) {
    memset ( line, 0, sizeof ( line ) );
    strcpy ( line, "../" );
    strcat ( line, testfile );
    stream = fopen ( line, "r" );
  }
  myassert ( stream );
  while ( fgets ( line, sizeof ( line ), stream ) != NULL ) {
    cout << endl << line;
    search->loadFen ( line );
    count++;
    search->print (  );
    search->setMaxTimeMillsec ( maxTimeMillsec );
    it->run (  );
    if ( strstr ( search->test_ris, search->test_found ) ) {
      foundit++;
      cout << "\nOK";
    }
    else {
      cout << "\nKO";
    }
    cout << "RESULT: (" << search->test_found << " " << search->test_ris << ") " << line << " \nfound " << foundit << "/" << count;
  }
  fclose ( stream );
  ftime ( &end );
  cout << "\ntime:  " << diff_time ( end, start ) << " total nodes per whole test " << num_tot_moves;
  cout << "\n ****** END TEST  " << testfile << " found " << foundit << "/" << count << " *******";
  cout << flush;
}

#endif
