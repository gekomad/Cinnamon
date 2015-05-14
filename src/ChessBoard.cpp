#include "ChessBoard.h"

ChessBoard::ChessBoard (  ) {
  START_FEN = string ( STARTPOS );
  memset ( &structure, 0, sizeof ( _Tboard ) );
  sideToMove = loadFen ( START_FEN );
  uci = false;
}

ChessBoard::~ChessBoard (  ) {
}

#ifdef DEBUG_MODE
u64
ChessBoard::getBitBoard ( int side ) {
  return side ? getBitBoard < WHITE > (  ) : getBitBoard < BLACK > (  );

}

int
ChessBoard::getPieceAt ( int side, u64 bitmapPos ) {
  return side ? getPieceAt < WHITE > ( bitmapPos ) : getPieceAt < BLACK > ( bitmapPos );
}
#endif

void
ChessBoard::setUci ( bool b ) {
  uci = b;
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

u64
ChessBoard::makeZobristKey (  ) {
  u64 x2, key = 0;
  int i, position;
  for ( i = 0; i < 12; i++ ) {
    x2 = chessboard[i];
    while ( x2 ) {
      position = BITScanForward ( x2 );
      updateZobristKey ( &key, i, position );
      x2 &= NOTPOW2[position];
    }
  }
  if ( enpassantPosition != -1 )
    updateZobristKey ( &key, 13, enpassantPosition );
  x2 = RIGHT_CASTLE;
  while ( x2 ) {
    position = BITScanForward ( x2 );
    updateZobristKey ( &key, 14, position );
    x2 &= NOTPOW2[position];
  }
  return key;
}

int
ChessBoard::getPieceByChar ( char c ) {
  for ( int i = 0; i < 12; i++ )
    if ( c == FEN_PIECE[i] )
      return i;
  return -1;
}

void
ChessBoard::display (  ) {
  if ( uci )
    return;			//TODO
  cout << endl << "     a   b   c   d   e   f   g   h";
  for ( int t = 0; t <= 63; t++ ) {
    char x = ' ';
    if ( t % 8 == 0 ) {
      cout << endl << "   ---------------------------------" << endl;
      cout << " " << 8 - RANK_AT[t] << " | ";
    }
    x = ( x = ( x = FEN_PIECE[getPieceAt < WHITE > ( POW2[63 - t] )] ) != '-' ? x : FEN_PIECE[getPieceAt < BLACK > ( POW2[63 - t] )] ) == '-' ? ' ' : x;
    x != ' ' ? cout << x : POW2[t] & WHITE_SQUARES ? cout << " " : cout << ".";
    cout << " | ";
  };
  cout << endl << "   ---------------------------------" << endl;
  cout << "     a   b   c   d   e   f   g   h" << endl << endl;
  string fen;
  boardToFen ( fen );
  cout << endl << fen << endl << endl << flush;
}

void
ChessBoard::boardToFen ( string & fen ) {
  int x, y, l = 0, i = 0, sq;
  char row[8];
  int q;
  for ( y = 0; y < 8; y++ ) {
    i = l = 0;
    strcpy ( row, "" );
    for ( x = 0; x < 8; x++ ) {
      sq = ( y * 8 ) + x;
      q = getPieceAt < BLACK > ( POW2[63 - sq] );
      if ( q == SQUARE_FREE )
	q = getPieceAt < WHITE > ( POW2[63 - sq] );
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
    fen.append ( row, i );
    if ( y < 7 )
      fen.append ( "/" );
  }
  if ( sideToMove == BLACK )
    fen.append ( " b " );
  else
    fen.append ( " w " );
  int cst = 0;
  if ( RIGHT_CASTLE & RIGHT_KING_CASTLE_WHITE_MASK ) {
    fen.append ( "K" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_WHITE_MASK ) {
    fen.append ( "Q" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_KING_CASTLE_BLACK_MASK ) {
    fen.append ( "k" );
    cst++;
  }
  if ( RIGHT_CASTLE & RIGHT_QUEEN_CASTLE_BLACK_MASK ) {
    fen.append ( "q" );
    cst++;
  }
  if ( !cst )
    fen.append ( "-" );
  if ( enpassantPosition == -1 )
    fen.append ( " -" );
  else {
    fen.append ( " " );
    sideToMove ? fen.append ( BOARD[enpassantPosition + 8] ) : fen.append ( BOARD[enpassantPosition - 8] );
  }
}

string
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
  assert ( 0 );
  return "";
}

char
ChessBoard::decodeBoard ( string a ) {
  for ( int i = 0; i < 64; i++ ) {
    if ( !a.compare ( BOARD[i] ) )
      return i;
  }
  cout << endl << a << endl;
  ASSERT ( 0 );
  return -1;
}

int
ChessBoard::loadFen (  ) {
  return loadFen ( START_FEN );
}

int
ChessBoard::loadFen ( string fen ) {
  if ( fen.length (  ) == 0 )
    return loadFen (  );
  istringstream iss ( fen );
  string pos, castle, enpassant, side;
  iss >> pos;
  iss >> side;
  iss >> castle;
  iss >> enpassant;
  int ix = 0;
  int s[64];
  memset ( chessboard, 0, sizeof ( chessboard ) );
  for ( unsigned ii = 0; ii < pos.length (  ); ii++ ) {
    uchar ch = pos.at ( ii );
    if ( ch != '/' ) {
      if ( INV_FEN[ch] != 0xFF )
	s[ix++] = INV_FEN[ch];
      else if ( ch > 47 && ch < 58 ) {
	for ( int t = 0; t < ch - 48; t++ )
	  s[ix++] = SQUARE_FREE;
      }
      else {
	cout << "Bad FEN position format (" << ( char ) ch << ") " << fen << endl;
	exit ( 1 );
      };
    }
  }
  if ( ix != 64 ) {
    cout << "Bad FEN position format " << fen << endl;
    exit ( 1 );
  }
  for ( int i = 0; i < 64; i++ ) {
    int p = s[63 - i];
    if ( p != SQUARE_FREE ) {
      chessboard[p] |= POW2[i];
    }
  };
  if ( side == "b" )
    sideToMove = BLACK;
  else if ( side == "w" )
    sideToMove = WHITE;
  else {
    cout << "Bad FEN position format " << fen << endl;
    exit ( 1 );
  }
  RIGHT_CASTLE = 0;
  for ( unsigned e = 0; e < castle.length (  ); e++ ) {
    switch ( castle.at ( e ) ) {
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
  friendKing[WHITE] = BITScanForward ( chessboard[KING_WHITE] );
  friendKing[BLACK] = BITScanForward ( chessboard[KING_BLACK] );
  enpassantPosition = -1;
  for ( int i = 0; i < 64; i++ ) {
    if ( enpassant == BOARD[i] ) {
      enpassantPosition = i;
      if ( sideToMove )
	enpassantPosition -= 8;
      else
	enpassantPosition += 8;
      break;
    }
  }
  zobristKey = makeZobristKey (  );
  return sideToMove;
}
