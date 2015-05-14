#include "ChessBoard.h"

ChessBoard::ChessBoard (  ) {
  INITIAL_FEN = START_FEN;
  memset ( &structure, 0, sizeof ( _Tboard ) );
  sideToMove = loadFen ( INITIAL_FEN );
  uci = false;
}

ChessBoard::~ChessBoard (  ) {
}

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
      x2 &= NOTTABLOG[position];
    }
  }
  if ( enpassantPosition != -1 )
    updateZobristKey ( &key, 13, enpassantPosition );
  x2 = RIGHT_CASTLE;
  while ( x2 ) {
    position = BITScanForward ( x2 );
    updateZobristKey ( &key, 14, position );
    x2 &= NOTTABLOG[position];
  }
  return key;
}

void
ChessBoard::display (  ) {
  if ( uci )
    return;

  int t;
  char x;

  cout << "\n.....a   b   c   d   e   f   g   h";
  for ( t = 0; t <= 63; t++ ) {
    if ( !( t % 8 ) ) {
      cout << "\n...---------------------------------\n";
    };
    x = FEN_PIECE[getPieceAtWhite ( TABLOG[63 - t] )];
    if ( x == '-' )
      x = FEN_PIECE[getPieceAtBlack ( TABLOG[63 - t] )];
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
  string fen;
  boardToFen ( fen );
  cout << endl << fen << endl << endl << flush;
}

int
ChessBoard::getPieceAtWhite ( u64 tablogpos ) {
  return ( chessboard[PAWN_WHITE] & tablogpos ) ? PAWN_WHITE : ( ( chessboard[KING_WHITE] & tablogpos ) ? KING_WHITE : ( ( chessboard[ROOK_WHITE] & tablogpos ) ? ROOK_WHITE : ( ( chessboard[BISHOP_WHITE] & tablogpos ) ? BISHOP_WHITE : ( ( chessboard[KNIGHT_WHITE] & tablogpos ) ? KNIGHT_WHITE : ( ( chessboard[QUEEN_WHITE] & tablogpos ) ? QUEEN_WHITE : SQUARE_FREE ) ) ) ) );
}

int
ChessBoard::getPieceAtBlack ( u64 tablogpos ) {
  return ( chessboard[PAWN_BLACK] & tablogpos ) ? PAWN_BLACK : ( ( chessboard[KING_BLACK] & tablogpos ) ? KING_BLACK : ( ( chessboard[ROOK_BLACK] & tablogpos ) ? ROOK_BLACK : ( ( chessboard[BISHOP_BLACK] & tablogpos ) ? BISHOP_BLACK : ( ( chessboard[KNIGHT_BLACK] & tablogpos ) ? KNIGHT_BLACK : ( ( chessboard[QUEEN_BLACK] & tablogpos ) ? QUEEN_BLACK : SQUARE_FREE ) ) ) ) );
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
      q = getPieceAtBlack ( TABLOG[63 - sq] );
      if ( q == SQUARE_FREE )
	q = getPieceAtWhite ( TABLOG[63 - sq] );
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
#ifdef DEBUG_MODE
  display (  );
  cout << endl << "CODE|" << a << "|type:" << ( int ) type << endl;
  ASSERT ( 0 );
#endif
  return "";

}

int
ChessBoard::loadFen (  ) {
  return loadFen ( INITIAL_FEN );
}

int
ChessBoard::loadFen ( string fen ) {
  istringstream iss ( fen );
  string pos, castle, enpassant, side;
  iss >> pos;
  iss >> side;
  iss >> castle;
  iss >> enpassant;
  int ix = 0;
  int s[64];
  memset ( chessboard, 0, sizeof ( chessboard ) );
  for ( int ii = 0; ii < pos.length (  ); ii++ ) {
    char ch = pos.at ( ii );
    if ( ch != '/' ) {
      if ( getInvFen[ch] != 0xFF )
	s[ix++] = getInvFen[ch];
      else if ( ch > 47 && ch < 58 ) {
	for ( int t = 0; t < ch - 48; t++ )
	  s[ix++] = SQUARE_FREE;
      }
      else {
	cout << "Bad FEN position format (" << ( char ) ch << ")" << endl << fen;
	exit ( 1 );
	return 0;
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
      chessboard[p] |= TABLOG[i];
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
  return sideToMove;
}

uchar
ChessBoard::rotateBoardLeft45 ( const u64 ss, const int pos ) {
  u64 xx = ss & LEFT[pos];
  return TABLOG_VERT45[pos] | bits::ROTATE_LEFT[( xx >> SHIFT_LEFT[pos] ) & 0xFFFFULL] | bits::ROTATE_LEFT[( ( xx ) >> 16 ) & 0xFFFFULL]
    | bits::ROTATE_LEFT[( ( xx ) >> 32 ) & 0xFFFFULL] | bits::ROTATE_LEFT[( ( xx ) >> 48 ) & 0xFFFFULL];
}

uchar
ChessBoard::rotateBoardRight45 ( const u64 ss, const int pos ) {
  u64 xx = ( ss & RIGHT[pos] );
  return TABLOG_VERT45[pos] | bits::ROTATE_RIGHT[( xx >> SHIFT_RIGHT[pos] ) & 0xFFFFULL] | bits::ROTATE_RIGHT[( ( xx ) >> 16 ) & 0xFFFFULL]
    | bits::ROTATE_RIGHT[( ( xx ) >> 32 ) & 0xFFFFULL] | bits::ROTATE_RIGHT[( ( xx ) >> 48 ) & 0xFFFFULL];
}
