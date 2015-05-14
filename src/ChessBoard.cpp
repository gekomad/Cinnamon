#include "ChessBoard.h"

ChessBoard::ChessBoard ( char *FEN ) {
  memset ( &structure, 0, sizeof ( STRUCTURE_TAG ) );
  blackMove = !loadFen ( FEN );
  uci = false;
}

ChessBoard::~ChessBoard (  ) {
}

void
ChessBoard::setUci (  ) {
  uci = true;
}

STRUCTURE_TAG *
ChessBoard::getStructure (  ) {
  return &structure;
}

uchar
ChessBoard::getRightCastle (  ) {
  return RIGHT_CASTLE;
}

void
ChessBoard::setRightCastle ( uchar r ) {
  RIGHT_CASTLE = r;
}

bool
ChessBoard::getUci (  ) {
  return uci;
}

void
ChessBoard::setBlackMove ( bool b ) {
  blackMove = b;
}

#if defined(PERFT_MODE) || defined(HASH_MODE)
#ifdef DEBUG_MODE
bool
ChessBoard::checkZobristKey ( u64 key, Tchessboard * board ) {
  map < u64, Tchessboard * >::iterator iterator;
  iterator = zobristKeysMap.find ( key );

  if ( iterator != zobristKeysMap.end (  ) && memcmp ( iterator->second, board, sizeof ( Tchessboard ) ) ) {
    print ( board );
    print ( iterator->second );
    cout << "key: 0x" << hex << key << " zobristKeysMap[key]: 0x" << hex << iterator->first << endl;
    return false;
  }
  if ( iterator == zobristKeysMap.end (  ) ) {
    Tchessboard *c = ( Tchessboard * ) malloc ( sizeof ( Tchessboard ) );
    memcpy ( c, board, sizeof ( Tchessboard ) );
    zobristKeysMap[key] = c;
  }
  return true;
}
#endif

u64
ChessBoard::makeZobristKey ( const int side ) {	//TODO inline?
  int i, position;
  u64 x2, result = 0;
  for ( i = 0; i < 12; i++ ) {
    x2 = chessboard[i];
    while ( x2 ) {
      position = BITScanForward ( x2 );
      result ^= zobrist_key[i][position];
      x2 &= NOTTABLOG[position];
    }
  }
  result ^= zobrist_key[12][side];

  if ( enpassantPosition != -1 ) {
    result ^= zobrist_key[13][enpassantPosition];
  }

  x2 = RIGHT_CASTLE;
  while ( x2 ) {
    position = BITScanForward ( x2 );
    result ^= zobrist_key[14][position];
    x2 &= NOTTABLOG[position];
  }
#ifdef DEBUG_MODE
  myassert ( checkZobristKey ( result, &chessboard ) );
#endif
  return result;
}
#endif
void
ChessBoard::print ( Tchessboard * s ) {
  Tchessboard bk;
  memcpy ( bk, chessboard, sizeof ( Tchessboard ) );
  memcpy ( chessboard, s, sizeof ( Tchessboard ) );
  print (  );
  memcpy ( chessboard, bk, sizeof ( Tchessboard ) );
}

void
ChessBoard::print ( char *s ) {
  if ( !uci )
    cout << s;
  print (  );
}

void
ChessBoard::print (  ) {
  if ( uci )
    return;
  int t;
  char x;
  char FEN[1000];
  cout << "\n.....a   b   c   d   e   f   g   h";
  for ( t = 0; t <= 63; t++ ) {
    if ( !( t % 8 ) ) {
      cout << "\n...---------------------------------\n";
    };
    x = FEN_PIECE[get_piece_at ( 1, TABLOG[63 - t] )];
    if ( x == '-' )
      x = FEN_PIECE[get_piece_at ( 0, TABLOG[63 - t] )];
    if ( x == '-' )
      x = ' ';
    switch ( t ) {
    case 0:
      cout << " 8 | ";
      break;
    case 8:
      cout << " 7 | ";
      break;
    case 16:
      cout << " 6 | ";
      break;
    case 24:
      cout << " 5 | ";
      break;
    case 32:
      cout << " 4 | ";
      break;
    case 40:
      cout << " 3 | ";
      break;
    case 48:
      cout << " 2 | ";
      break;
    case 56:
      cout << " 1 | ";
      break;
    default:
      ;
      break;
    }
    if ( x != ' ' )
      cout << x;
    else if ( t == 0 || t == 2 || t == 4 || t == 6 || t == 9 || t == 11 || t == 13 || t == 15 || t == 16 || t == 18 || t == 20 || t == 22 || t == 25 || t == 27 || t == 29 || t == 31 || t == 32 || t == 34 || t == 36 || t == 38 || t == 41 || t == 43 || t == 45 || t == 47 || t == 48 || t == 50 || t == 52 || t == 54 || t == 57 || t == 59 || t == 61 || t == 63 )
      cout << " ";
    else
      cout << ".";
    cout << " | " << flush;
  };
  cout << "\n";
  cout << "...---------------------------------\n";
  cout << ".....a   b   c   d   e   f   g   h\n\n";
  BoardToFEN ( FEN );
  cout << "\n" << FEN << endl << flush;
}

void
ChessBoard::BoardToFEN ( char *FEN ) {
  int x, y, l = 0, i = 0, sq;
  char row[8];
  int q;
  strcpy ( FEN, "" );
  for ( y = 0; y < 8; y++ ) {
    i = l = 0;
    strcpy ( row, "" );
    for ( x = 0; x < 8; x++ ) {
      sq = ( y * 8 ) + x;
      q = get_piece_at ( 0, TABLOG[63 - sq] );
      if ( q == SQUARE_FREE )
	q = get_piece_at ( 1, TABLOG[63 - sq] );
      if ( q == SQUARE_FREE )
	l++;
      else {
	if ( l > 0 ) {
	  row[i] = ( char ) ( l + 48 );
	  i++;
	}
	l = 0;
	row[i] = FEN_PIECE[q];
	i++;
      }
    }
    if ( l > 0 ) {
      row[i] = ( char ) ( l + 48 );
      i++;
    }
    strncat ( FEN, row, i );
    if ( y < 7 )
      strcat ( FEN, "/" );
  }
  if ( blackMove )
    strcat ( FEN, " b " );
  else
    strcat ( FEN, " w " );
  int cst = 0;
  if ( RIGHT_CASTLE & RIGHT_KING_CASTLE_WHITE_MASK ) {
    strcat ( FEN, "K" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_WHITE_MASK ) {
    strcat ( FEN, "Q" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_KING_CASTLE_BLACK_MASK ) {
    strcat ( FEN, "k" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_BLACK_MASK ) {
    strcat ( FEN, "q" );
    cst++;
  }
  if ( !cst )
    strcat ( FEN, "-" );
  strcat ( FEN, " -" );

}

int
ChessBoard::loadFen ( char *ss ) {
  return loadFen ( ss, 1 );
}

#ifdef TEST_MODE
int
ChessBoard::extractTestResult ( char *ris1 ) {

  int i, result = 1;
  char *x;
  char dummy[3];
  if ( ris1[strlen ( ris1 ) - 1] == ';' ) {
    result = 0;
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  if ( ris1[strlen ( ris1 ) - 1] == '+' ) {
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  if ( ris1[strlen ( ris1 ) - 1] == '#' ) {
    ris1[strlen ( ris1 ) - 1] = 0;
  }
  i = ( int ) strlen ( ris1 );
  memset ( dummy, 0, sizeof ( dummy ) );
  switch ( i ) {
  case 2:
    strncpy ( dummy, ris1, 2 );
    break;
  case 3:
    if ( !strcmp ( ris1, "O-O" ) ) {
      if ( blackMove )
	strcpy ( dummy, "g8" );
      else
	strcpy ( dummy, "g1" );
    }
    else if ( ris1[0] < 91 )
      strncpy ( dummy, ris1 + 1, 2 );
    else
      strncpy ( dummy, ris1, 2 );
    break;
  case 4:
    strncpy ( dummy, ris1 + 2, 2 );
    break;
  case 5:
    if ( !strcmp ( ris1, "O-O-O" ) ) {
      if ( blackMove )
	strcpy ( dummy, "c8" );
      else
	strcpy ( dummy, "c1" );
    }
    else if ( ris1[3] == '+' ) {
      if ( ris1[0] < 91 )
	strncpy ( dummy, ris1 + 1, 2 );
      else
	strncpy ( dummy, ris1, 2 );
    }
    else if ( ( x = strstr ( ris1, "x" ) ) )
      strncpy ( dummy, x + 1, 2 );
    else
      strncpy ( dummy, ris1 + 3, 2 );
    break;
  case 6:
    strncpy ( dummy, ris1 + 3, 2 );
    break;
  default:
    printf ( "\nPARSE ERROR" );
    break;
  }
  if ( !( dummy[0] >= 'a' && dummy[0] <= 'h' && dummy[1] >= '1' && dummy[1] <= '8' ) )
    printf ( "\nPARSE ERROR" );
  strcat ( test_ris, dummy );
  strcat ( test_ris, " " );
  return result;
}

void
ChessBoard::getTestResult ( char *ss ) {
  memset ( test_ris, 0, sizeof ( test_ris ) );
  char *ris1 = strstr ( ss, " bm " );
  if ( !ris1 )
    ris1 = strstr ( ss, " am " );
  if ( !ris1 )
    return;
#ifdef DEBUG_MODE
  if ( !ris1 ) {
    cout << endl << ss << endl;
    myassert ( 0 );
  };
#endif
  ASSERT ( ris1 );
  char dummy[100];
  myassert ( ris1 );
  ris1 += 4;
  char *ris2;
  do {
    ris2 = strstr ( ris1, " " );
    if ( !ris2 )
      return;
#ifdef DEBUG_MODE
    if ( !ris2 ) {
      cout << ss << endl;
      myassert ( 0 );
    };
#endif
    ASSERT ( ris2 );
    strncpy ( dummy, ris1, ris2 - ris1 );
    dummy[ris2 - ris1] = 0;
    ris1 += ris2 - ris1 + 1;
  } while ( extractTestResult ( dummy ) );
  ris1 = dummy;
  strcpy ( dummy, test_ris );
  do {
    ris2 = strstr ( dummy, " " );
    if ( ris2 ) {
      strncpy ( dummy, ris1, ris2 - ris1 );
      dummy[ris2 - ris1] = 0;
      ris1 += ris2 - ris1 + 1;
    }
    if ( decodeBoard ( dummy ) == -1 )
      myassert ( 0 );
  } while ( ris2 );
}
#endif

char
ChessBoard::decodeBoard ( char *a ) {
  for ( int i = 0; i < 64; i++ ) {
    if ( !strcmp ( a, BOARD[i] ) )
      return i;
  }
  printf ( "\n||%s||", a );
  fflush ( stdout );
  ASSERT ( 0 );
  return -1;
}

int
ChessBoard::getInvFen ( char a ) {
  switch ( a ) {
  case 'r':
    return 2;
    break;
  case 'n':
    return 6;
    break;
  case 'b':
    return 4;
    break;
  case 'q':
    return 10;
    break;
  case 'k':
    return 8;
    break;
  case 'p':
    return 0;
    break;
  case 'P':
    return 1;
    break;
  case 'R':
    return 3;
    break;
  case 'B':
    return 5;
    break;
  case 'Q':
    return 11;
    break;
  case 'K':
    return 9;
    break;
  case 'N':
    return 7;
    break;
  default:
    myassert ( 0 );
  }
  return -1;
}

char *
ChessBoard::decodeType ( uchar type ) {
  switch ( type & 0xF ) {
  case STANDARD_MOVE_MASK:
    return "STANDARD_MOVE_MASK";
    break;
  case ENPASSANT_MOVE_MASK:
    return "ENPASSANT_MOVE_MASK";
    break;
  case PROMOTION_MOVE_MASK:
    return "PROMOTION_MOVE_MASK";
    break;
  case KING_SIDE_CASTLE_MOVE_MASK:
    return "KING_SIDE_CASTLE_MASK";
    break;
  case QUEEN_SIDE_CASTLE_MOVE_MASK:
    return "QUEEN_SIDE_CASTLE_MASK";
    break;
  default:
    myassert ( 0 );
  }
  myassert ( 0 );
}

const char *
ChessBoard::decodeBoardinv ( const uchar type, const int a, const int side ) {
  if ( type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == WHITE ) {
    return "e1c1";
  }
  if ( type & KING_SIDE_CASTLE_MOVE_MASK && side == WHITE ) {
    return "e1g1";
  }
  if ( type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == BLACK ) {
    return "e8c8";
  }
  if ( type & KING_SIDE_CASTLE_MOVE_MASK && side == BLACK ) {
    return "e8g8";
  }
  ASSERT ( !( type & 0xC ) );
  if ( a >= 0 && a < 64 )
    return BOARD[a];
#ifdef DEBUG_MODE
  cout << "\n" << a;
  ASSERT ( 0 );
#endif
  return "";

}

int
ChessBoard::loadFen ( char *ss, int check ) {
  if ( !ss )
    return -1;
  int i, ii, t, p;
  int dummy = check;
  char ch;
  char *x;
  int s[64];
  char aa[2];
  memset ( chessboard, 0, sizeof ( chessboard ) );

  i = 0;
  ii = 0;
  if ( strlen ( ss ) )
    do {
      ch = ss[ii];
      switch ( ch ) {
      case 'r':
	s[i++] = 2;
	break;
      case 'n':
	s[i++] = 6;
	break;
      case 'b':
	s[i++] = 4;
	break;
      case 'q':
	s[i++] = 10;
	break;
      case 'k':
	s[i++] = 8;
	break;
      case 'p':
	s[i++] = 0;
	break;
      case 'P':
	s[i++] = 1;
	break;
      case 'R':
	s[i++] = 3;
	break;
      case 'B':
	s[i++] = 5;
	break;
      case 'Q':
	s[i++] = 11;
	break;
      case 'K':
	s[i++] = 9;
	break;
      case 'N':
	s[i++] = 7;
	break;
      case '/':
	;
	break;
      case ' ':
	;
	break;
      case '-':
	;
	break;
      case 'w':
	;
	break;
      case 10:
	;
	break;
      case 13:
	;
	break;
      default:{
	if ( ch > 47 && ch < 58 ) {
	  aa[0] = ch;
	  aa[1] = 0;
	  for ( t = 1; t <= atoi ( aa ); t++ )
	    s[i++] = SQUARE_FREE;
	}
	else {
	  cout << "Bad FEN position format.|" << ( char ) ch << "|\n" << ss;
	  cout << "\nerror";
	  return 0;
	};
      }
      }
      ii++;
    } while ( i < 64 );
  for ( i = 0; i <= 63; i++ ) {
    p = s[63 - i];
    if ( p != SQUARE_FREE )
      chessboard[p] |= TABLOG[i];
  };
  if ( ( x = strstr ( ss, " b " ) ) )
    blackMove = true;
  else if ( ( x = strstr ( ss, " w " ) ) )
    blackMove = false;
  else
    cout << "Bad FEN position format.\n" << ss;
  x += 3;
#ifdef TEST_MODE
  if ( check )
    getTestResult ( ss );
#endif
  RIGHT_CASTLE = 0;

  enpassantPosition = -1;
  i = 0;
  while ( ( unsigned ) i < strlen ( x ) && x[i] != ' ' ) {
    switch ( x[i++] ) {
    case 'K':
      RIGHT_CASTLE |= RIGHT_KING_CASTLE_WHITE_MASK;
      break;
    case 'k':
      RIGHT_CASTLE |= RIGHT_KING_CASTLE_BLACK_MASK;
      break;
    case 'Q':
      RIGHT_CASTLE |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
      break;
    case 'q':
      RIGHT_CASTLE |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
      break;
    default:
      ;

    };
  };
  Friend_king[blackMove] = BITScanForward ( chessboard[KING_BLACK + blackMove] );
  Friend_king[blackMove ^ 1] = BITScanForward ( chessboard[KING_BLACK + ( blackMove ^ 1 )] );

  return !blackMove;
}
