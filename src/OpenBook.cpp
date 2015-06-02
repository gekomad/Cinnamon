#include "OpenBook.h"

OpenBook::OpenBook (  ) {
  book = nullptr;
  sizeBook[WHITE] = sizeBook[BLACK] = 0;
  bookFile = "/home/geko/chess/cinnamon_book.bin";	//TODO
  fileWhite = "/home/geko/chess/white.txt";	//TODO
  fileBlack = "/home/geko/chess/black.txt";	//TODO
}

bool
OpenBook::load (  ) {
  book = nullptr;
  const char *_WHITE = "WHITE";
  const char *_BLACK = "BLACK";
  fileSize = _file::fileSize ( bookFile );
  if ( fileSize == -1 ) {
    useMmap = false;
    miniBook = "WHITE\1.>YQ/?VN%7`Y$5XH08\1.>YQ/?VN%7`Y$5TL(D\1/?WG0@G@$5XH.6VN%@\1/?WG0@G@$5XH.6\\M\10@YQ/?`Y)8\\M\10@YQ/?\\M\1$5WG,4XP%,`<&$\\M/7\1BLACK\1)8XH.>aP%IP?I@TL$5]T\1)8XH.>aP$5VF%@]V/7WO\1$5\\M0@VN)8]A'0^\\2:A8\1$5\\M0@VN)8]A'0^\\/?XH\1/?\\M(DWGDMVM.6]O0@G@\1/?WG0@G@$5ZR.6\\M%@VN\1.>VF%@\\M/7XP$5WG@9]O\1.>VF%@\\M/7]H$5WO08HQ\1.>VF$5\\M5FWOF5M>/7>M\1";
    fileSize = miniBook.size (  );
    _string::replace ( miniBook, 1, 0 );
    book = ( char * ) miniBook.c_str (  );
  }
  else {
    useMmap = true;
    book = ( char * ) _memory::_mmap ( bookFile );
    cout << "book " << bookFile << " loaded " << endl;
  }
  sizeBook[WHITE] = sizeBook[BLACK] = 0;
  if ( !book ) {
    cout << "error on load open book " << bookFile << endl;
    return false;
  }
  char *b = book;
  int side = -1;
  while ( b - book < fileSize ) {
    if ( !strcmp ( b, _WHITE ) ) {
      side = WHITE;
      b += strlen ( _WHITE ) + 1;
    }
    else if ( !strcmp ( b, _BLACK ) ) {
      side = BLACK;
      b += strlen ( _BLACK ) + 1;
    }
    if ( side != -1 )
      sizeBook[side]++;
    b += strlen ( b ) + 1;
  }
  if ( !sizeBook[WHITE] || !sizeBook[BLACK] ) {
    cout << "invalid open book" << endl;
    _memory::_munmap ( book, fileSize );
    sizeBook[WHITE] = sizeBook[BLACK] = 0;
    book = nullptr;
    return false;
  }
  random[WHITE] = ( int * ) malloc ( sizeBook[WHITE] * sizeof ( int ) );
  random[BLACK] = ( int * ) malloc ( sizeBook[BLACK] * sizeof ( int ) );
  int k = -1;
  b = book;
  while ( b - book < fileSize ) {
    if ( !strcmp ( b, _WHITE ) ) {
      side = WHITE;
      b += strlen ( _WHITE ) + 1;
      k = 0;
    }
    else if ( !strcmp ( b, _BLACK ) ) {
      side = BLACK;
      b += strlen ( _BLACK ) + 1;
      k = 0;
    }
    assert ( k != -1 );
    random[side][k++] = b - book;
    b += strlen ( b ) + 1;
  }
  //shuffle
  for ( side = 0; side < 2; side++ ) {
    for ( int j = 0; j < sizeBook[side] - 1; j++ ) {
      unsigned r = j + ( _random::getRandom64 (  ) % ( sizeBook[side] - j ) );
      int temp = random[side][j];
      random[side][j] = random[side][r];
      random[side][r] = temp;
    }
  }
  return true;
}

OpenBook::~OpenBook (  ) {
  if ( book ) {
    free ( random[BLACK] );
    free ( random[WHITE] );
    if ( useMmap )
      _memory::_munmap ( book, fileSize );
  }
}

string
OpenBook::search ( int side, string movesPath ) {
  if ( !book )
    return "";
  for ( unsigned i = 0; i < movesPath.size (  ); i++ ) {
    movesPath[i] += SHIFT;
  }
  for ( int ii1 = 0; ii1 < sizeBook[side]; ii1++ ) {
    int i = random[side][ii1];
    if ( !strncmp ( book + i, movesPath.c_str (  ), movesPath.size (  ) ) ) {
      if ( strlen ( book + i ) < movesPath.size (  ) + 2 ) {
	return "";
      }
      string m;
      m += BOARD[book[i + movesPath.size (  )] - SHIFT];
      m += BOARD[book[i + movesPath.size (  ) + 1] - SHIFT];
      return m;
    }
  }
  return "";
}

int
OpenBook::getAttackers ( int piece, int side, int rank, int file, int to ) {

  gen->incListId (  );
  gen->resetList (  );
  u64 friends = side ? gen->getBitBoard < WHITE > (  ) : gen->getBitBoard < BLACK > (  );
  u64 enemies = side ? gen->getBitBoard < BLACK > (  ) : gen->getBitBoard < WHITE > (  );

  u64 allpieces = enemies | friends;
  if ( piece == -1 ) {

    gen->generateCaptures ( side, enemies, friends );
    gen->generateMoves ( side, allpieces );

  }
  else
    switch ( piece ) {
    case ChessBoard::PAWN_WHITE:
      if ( side ) {
	gen->performPawnShift < WHITE > ( ~allpieces );
	gen->performPawnCapture < WHITE > ( enemies );
      }
      else {
	gen->performPawnShift < BLACK > ( ~allpieces );
	gen->performPawnCapture < BLACK > ( enemies );
      }
      break;
    case ChessBoard::KNIGHT_WHITE:
      gen->performKnightShiftCapture ( ChessBoard::KNIGHT_BLACK + side, enemies, side );
      gen->performKnightShiftCapture ( ChessBoard::KNIGHT_BLACK + side, ~allpieces, side );
      break;
    case ChessBoard::BISHOP_WHITE:
      gen->performDiagCapture ( ChessBoard::BISHOP_BLACK + side, enemies, side, allpieces );
      gen->performDiagShift ( ChessBoard::BISHOP_BLACK + side, side, allpieces );
      break;
    case ChessBoard::QUEEN_WHITE:
      gen->performRankFileCapture ( ChessBoard::QUEEN_BLACK + side, enemies, side, allpieces );
      gen->performDiagCapture ( ChessBoard::QUEEN_BLACK + side, enemies, side, allpieces );
      gen->performDiagShift ( ChessBoard::QUEEN_BLACK + side, side, allpieces );
      gen->performRankFileShift ( ChessBoard::QUEEN_BLACK + side, side, allpieces );
      break;
    case ChessBoard::ROOK_WHITE:
      gen->performRankFileCapture ( ChessBoard::ROOK_BLACK + side, enemies, side, allpieces );
      gen->performRankFileShift ( ChessBoard::ROOK_BLACK + side, side, allpieces );
      break;
    case ChessBoard::KING_WHITE:
      if ( side ) {
	gen->performKingShiftCapture ( WHITE, enemies );
	gen->performKingShiftCapture ( WHITE, ~allpieces );
      }
      else {
	gen->performKingShiftCapture ( BLACK, enemies );
	gen->performKingShiftCapture ( BLACK, ~allpieces );
      }
      break;
    default:
      return -1;
      break;
    }

  int listcount = gen->getListSize (  );
  if ( !listcount ) {
    return -1;
  };
  for ( int j = 0; j < listcount; j++ ) {
    _Tmove *mov = gen->getMove ( j );
    if ( to == mov->to ) {

      if ( file == -1 && rank == -1 && !gen->isPinned ( side, mov->from, mov->side ? gen->getPieceAt < WHITE > ( POW2[mov->from] ) : gen->getPieceAt < BLACK > ( POW2[mov->from] ) ) ) {
	gen->decListId (  );
	return mov->from;
      }
      if ( file != -1 && file == FILE_AT[mov->from] && !gen->isPinned ( side, mov->from, mov->side ? gen->getPieceAt < WHITE > ( POW2[mov->from] ) : gen->getPieceAt < BLACK > ( POW2[mov->from] ) ) ) {
	gen->decListId (  );
	return mov->from;
      }
      if ( rank != -1 && rank == RANK_AT[mov->from] && !gen->isPinned ( side, mov->from, mov->side ? gen->getPieceAt < WHITE > ( POW2[mov->from] ) : gen->getPieceAt < BLACK > ( POW2[mov->from] ) ) ) {
	gen->decListId (  );
	return mov->from;
      }
    }
  }
  gen->decListId (  );
  return -1;
}

bool
OpenBook::san2coord ( string san1, int *from, int *to, int side ) {
  string san;
  for ( unsigned i = 0; i < san1.size (  ); i++ )
    if ( san1.at ( i ) != 'x' && san1.at ( i ) != '+' )
      san += san1.at ( i );
  int rank = -1;
  int file = -1;
  //castle
  if ( san.at ( 0 ) == 'O' ) {
    //O-O
    //O-O-O
    if ( !side ) {
      *from = 59;
      if ( !san.compare ( "O-O" ) )
	*to = 57;
      else
	*to = 61;
    }
    else {
      *from = 3;
      if ( !san.compare ( "O-O" ) )
	*to = 1;
      else
	*to = 5;
    }
    return true;
  }

  if ( san.find ( "=" ) != string::npos ) {
    assert ( 0 );
  }

  int piece = -1;
  if ( san.at ( 0 ) == toupper ( san.at ( 0 ) ) ) {
    piece = gen->getPieceByChar ( san.at ( 0 ) );
    if ( san.size (  ) == 4 ) {
      if ( isdigit ( san.at ( 1 ) ) ) {
	rank = san.at ( 1 ) - 49;
      }
      else
	file = 7 - ( san.at ( 1 ) - 97 );
    }

  }
  else {
    if ( san.size (  ) != 2 ) {
      if ( isdigit ( san.at ( 0 ) ) )
	rank = san.at ( 0 ) + 95 - 1;
      else
	file = 7 - ( san.at ( 0 ) - 97 );
    }
    else
      piece = ChessBoard::PAWN_WHITE;
  }
  *to = gen->decodeBoard ( ( san.substr ( san.size (  ) - 2, 2 ) ) );
  *from = getAttackers ( piece, side, rank, file, *to );
  if ( *from == -1 ) {
    //display();
    return false;
  }
  return true;
}

void
OpenBook::printError (  ) {
  cout << "have you run ?" << endl;
  cout << "\tpolyglot make-book -pgn book.pgn" << endl;
  cout << "\tpolyglot dump-book -bin book.bin -color white -out " << fileWhite << endl;
  cout << "\tpolyglot dump-book -bin book.bin -color black -out " << fileBlack << endl;
}

bool
OpenBook::create (  ) {
  ifstream inData;
  inData.open ( fileWhite.c_str (  ) );
  if ( !inData ) {
    cout << "error file not found: " << fileWhite << endl;
    printError (  );
    return false;
  }
  inData.close (  );
  inData.open ( fileBlack.c_str (  ) );
  if ( !inData ) {
    cout << "error file not found: " << fileBlack << endl;
    printError (  );
    return false;
  }
  inData.close (  );
  gen = new GenMoves (  );
  ofstream f;
  f.open ( bookFile, fstream::out );
  if ( !f.is_open (  ) ) {
    cout << "write error " << bookFile << endl;
    return false;
  }
  f << "WHITE";
  //cout<<"WHITE"<<"\\1";;
  f.put ( 0 );
  create ( fileWhite, f );
  f << "BLACK";
  //cout<< "BLACK"<<"\\1";;
  f.put ( 0 );
  create ( fileBlack, f );
  f.close (  );
  delete gen;
  return true;
}

void
OpenBook::create ( string fileIn, ofstream & f ) {
  ifstream inData;
  inData.open ( fileIn.c_str (  ) );
  if ( !inData ) {
    cout << "error file not found: " << fileIn;
    return;
  }
  string fen;
  string s1;
  stringstream s2;
  stringstream s3;
  bool error = false;
  while ( !inData.eof (  ) ) {
    getline ( inData, fen );

    if ( fen.find ( "{" ) == string::npos )
      continue;
    if ( fen.find ( " {cycle" ) != string::npos )
      fen.erase ( fen.find ( " {cycle" ), fen.size (  ) - fen.find ( " {cycle" ) );
    if ( fen.find ( " {tran" ) != string::npos )
      fen.erase ( fen.find ( " {tran" ), fen.size (  ) - fen.find ( " {tran" ) );
    _string::replace ( fen, "{", " " );

    if ( fen.find ( "=Q" ) != string::npos )
      continue;
    if ( fen.find ( "=R" ) != string::npos )
      continue;
    if ( fen.find ( "=N" ) != string::npos )
      continue;
    if ( fen.find ( "=B" ) != string::npos )
      continue;
    if ( fen.find ( "#" ) != string::npos )
      continue;
    _string::trimRight ( fen );
    gen->loadFen (  );

    string token;
    istringstream uip ( fen );
    uip >> token;
    int from, to;
    _Tmove move;

    string san;
    int side = WHITE;
    string castle;
    s1.clear (  );
    s2.str ( "" );
    s2.clear (  );
    s3.str ( "" );
    s3.clear (  );
    while ( !uip.eof (  ) && !error ) {
      token = "";
      uip >> token;
      if ( token.find ( "." ) != string::npos )
	continue;
      if ( token.find ( "%" ) != string::npos )
	continue;
      castle.clear (  );
      if ( !san2coord ( token, &from, &to, side ) ) {
	error = true;
	continue;
      }

      s1 += token + " ";
      s3 << BOARD[from] << BOARD[to] << " ";
      s2 << ( char ) ( from + SHIFT ) << ( char ) ( to + SHIFT );
      san.clear (  );
      san += BOARD[from];
      san += BOARD[to];

      gen->getMoveFromSan ( san, &move );
      gen->makemove ( &move, true, false );
      side ^= 1;
    }
    if ( !error ) {
      f << s2.str (  );
      /*for(unsigned t=0;t<s2.str().size();t++){
         if(s2.str()[t]=='\\')cout<<"\\";
         cout<<s2.str()[t];
         }
         cout<<"\\1"; */
      f.put ( 0 );
    }
    error = false;
  }
  inData.close (  );
}
