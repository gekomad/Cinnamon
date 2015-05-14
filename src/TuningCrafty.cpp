#ifdef TUNE_CRAFTY_MODE
#include "TuningCrafty.h"

TuningCrafty::TuningCrafty ( string craftyExe ) {
  search = new Search (  );
  it = new IterativeDeeping ( search );
  this->craftyExe = craftyExe;
}

TuningCrafty::~TuningCrafty (  ) {
  delete search;
  delete it;
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
TuningCrafty::extract_crafty_scores ( const string crafty_og, float *crafty_material, float *crafty_pawns, float *crafty_passed_pawns, float *crafty_knights, float *crafty_bishop, float *crafty_rooks, float *crafty_queens, float *crafty_kings, float *crafty_development, float *crafty_pawn_race, float *crafty_total ) {
  string buf;
  ifstream streamLog ( crafty_og );
  myassert ( streamLog.is_open (  ) );
  while ( streamLog.good (  ) ) {
    getline ( streamLog, buf );
    if ( buf.find ( "material......." ) )
      *crafty_material = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "pawns.........." ) )
      *crafty_pawns = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "passed pawns..." ) )
      *crafty_passed_pawns = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "knights........" ) )
      *crafty_knights = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "bishops........" ) )
      *crafty_bishop = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "rooks.........." ) )
      *crafty_rooks = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "queens........." ) )
      *crafty_queens = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "kings.........." ) )
      *crafty_kings = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "development...." ) )
      *crafty_development = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "pawn races....." ) )
      *crafty_pawn_race = atof ( buf.c_str (  ) + 15 );
    if ( buf.find ( "total.........." ) )
      *crafty_total = atof ( buf.c_str (  ) + 15 );
  }
  streamLog.close (  );
}

CRAFTY_TAG *
TuningCrafty::loadBin ( string epdfile, int *wc_file ) {
  string bin = epdfile + ".bin";
  CRAFTY_TAG *crafy_score;
  ifstream stream ( bin, ifstream::binary );
  if ( !stream.is_open (  ) ) {
    return createBin ( epdfile, wc_file );
  }
  *wc_file = file::fileSize ( bin ) / sizeof ( CRAFTY_TAG );
  crafy_score = ( CRAFTY_TAG * ) malloc ( *wc_file * sizeof ( CRAFTY_TAG ) );
  stream.read ( ( char * ) crafy_score, *wc_file * sizeof ( CRAFTY_TAG ) );
  stream.close (  );
  return crafy_score;

}

CRAFTY_TAG *
TuningCrafty::createBin ( string epdfile, int *wc_file ) {
  string line;
  string buf;
  cout << "creating .bin file..." << endl << flush;
  static const string PATH_LOG = "crafty.log";
  *wc_file = file::wc ( epdfile );
  CRAFTY_TAG *crafy_score = ( CRAFTY_TAG * ) calloc ( *wc_file, sizeof ( CRAFTY_TAG ) );
  ifstream stream ( epdfile );
  myassert ( stream.is_open (  ) );
  int count = 0;
  for ( int i = 0; i < *wc_file; i++ ) {
    getline ( stream, line );
    line.erase ( line.find_last_not_of ( "\n\r" ) + 1 );
    if ( line.length (  ) >= 200 ) {
      cout << "skip fen exceed size" << line << endl;
      continue;
    }
    if ( !( count % 100 ) )
      cout << count << "/" << *wc_file << "..." << flush;
    buf = craftyExe + " log=off ponder=off setboard \"" + line + "" "\" score quit>" + PATH_LOG;

    int dummy = system ( buf.c_str (  ) );
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
    strcpy ( crafy_score[count].fen, line.c_str (  ) );
    count++;
  }
  stream.close (  );
  epdfile += ".bin";
  ofstream outfile ( epdfile, ofstream::binary );
  outfile.write ( ( char * ) crafy_score, *wc_file * sizeof ( CRAFTY_TAG ) );
  outfile.close (  );
  cout << "done" << endl << flush;
  return crafy_score;
}

void
TuningCrafty::evalCraftyTuning ( string epdfile ) {
  int CD = rand (  );

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

  int side, wc_file;

  CRAFTY_TAG *crafy_score = loadBin ( epdfile, &wc_file );

  char tuning_file[255];
  char log_file[255];
  int friendKing[2];
  sprintf ( tuning_file, "tuning%d.txt", CD );
  sprintf ( log_file, "log%d.txt", CD );

  CACHE_INIT_TAG *cache_init = ( CACHE_INIT_TAG * ) malloc ( wc_file * sizeof ( CACHE_INIT_TAG ) );

  cout << "rows: " << wc_file << endl << flush;
  for ( int i = 0; i < wc_file; i++ ) {
    side = search->loadFen ( crafy_score[i].fen );
    cache_init[i].RIGHT_CASTLE = search->getRightCastle (  );
    cache_init[i].ENP_POSSIBILE = search->enpassantPosition;
    memcpy ( cache_init[i].friendKing, friendKing, sizeof ( friendKing ) );
    cache_init[i].side = search->getSide (  );
    memcpy ( cache_init[i].chessboard, search->chessboard, sizeof ( search->chessboard ) );

  }
  cout << endl << "start.." << endl << flush;

  int param_count = 0;
  string line2;
  ofstream fout ( tuning_file, ios::app );
  ofstream flog ( log_file, ios::app );

  u64 start = 0, tot, min = -1;
  while ( 1 ) {
    if ( param_count && !( param_count % 10000 ) ) {
      cout << param_count << "..." << endl << flush;
    }
    tot = 0;
    for ( int i = 0; i < wc_file; i++ ) {
      search->init (  );
      search->setRightCastle ( cache_init[i].RIGHT_CASTLE );
      search->enpassantPosition = cache_init[i].ENP_POSSIBILE;

      memcpy ( friendKing, cache_init[i].friendKing, sizeof ( friendKing ) );
      side = cache_init[i].side;
      memcpy ( search->chessboard, cache_init[i].chessboard, sizeof ( search->chessboard ) );

      search->getScore ( side, &butterfly_material, &butterfly_pawns, &butterfly_passed_pawns, &butterfly_knights, &butterfly_bishop, &butterfly_rooks, &butterfly_queens, &butterfly_kings, &butterfly_development, &butterfly_pawn_race, &butterfly_total );

      tot += sqrt ( pow ( butterfly_total - crafy_score[i].crafty_total, 2 ) );

    }
    if ( !param_count )
      start = tot;
    if ( tot < min ) {
      min = tot;
      search->writeParam ( tuning_file, param_count, true );
      flog << tot << " MSE " << "param" << param_count << " " << 100 - ( min * 100 / start ) << "%" << endl;
      cout << tot << " MSE " << "param" << param_count << " " << 100 - ( min * 100 / start ) << "%" << endl;
      flog.flush (  );
      cout.flush (  );
      param_count++;
    }
    search->setRandomParam (  );
  }
  fout.close (  );
  flog.close (  );
}
#endif
