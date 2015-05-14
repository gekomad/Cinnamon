#include "IterativeDeeping.h"

IterativeDeeping::IterativeDeeping (  ) {
  maxDepth = MAX_PLY;
  openBook = nullptr;
  ponderEnabled = true;
  followBook = true;
  setUseBook ( false );
}

void
IterativeDeeping::setMaxDepth ( int d ) {
  maxDepth = d;
}

IterativeDeeping::~IterativeDeeping (  ) {
  if ( openBook )
    delete openBook;
}

void
IterativeDeeping::enablePonder ( bool b ) {
  ponderEnabled = b;
}

void
IterativeDeeping::setFollowBook ( bool b ) {
  followBook = b;
}

void
IterativeDeeping::unLock (  ) {
  mutex1.unlock (  );
}

void
IterativeDeeping::lock (  ) {
  mutex1.lock (  );
}

bool
IterativeDeeping::getPonderEnabled (  ) {
  return ponderEnabled;
}

void
IterativeDeeping::clearMovesPath (  ) {
  followBook = true;
  Search::clearMovesPath (  );
}

bool
IterativeDeeping::getUseBook (  ) {
  return useBook;
}

void
IterativeDeeping::setUseBook ( bool b ) {
  useBook = b;
  bool valid = true;
  if ( b && openBook == nullptr ) {
    openBook = new OpenBook (  );
    valid = useBook = openBook->load (  );
  }
  if ( ( !b && openBook ) || !valid ) {
    delete openBook;
    openBook = nullptr;
    useBook = false;
  }
}

void
IterativeDeeping::run (  ) {
  lock (  );
  _Tmove resultMove;
  memset ( &resultMove, 0, sizeof ( resultMove ) );
  struct timeb start1;
  struct timeb end1;
  _TpvLine line;
  int val = 0, tmp;
  string pvv;
  _Tmove move2;
  int TimeTaken;
  setRunning ( 2 );
  int mply = 0;
  startClock (  );
  clearKillerHeuristic (  );
  clearAge (  );
  ftime ( &start1 );
  if ( useBook && followBook ) {
    ASSERT ( openBook );
    string obMove = openBook->search ( getSide (  ), getMovesPath (  ) );
    if ( !obMove.size (  ) ) {
      followBook = false;
    }
    else {
      _Tmove move;
      getMoveFromSan ( obMove, &move );
      makemove ( &move );
      cout << "bestmove " << obMove << endl << flush;
      pushMovesPath ( ( char ) decodeBoard ( obMove.substr ( 0, 2 ) ) );
      pushMovesPath ( decodeBoard ( obMove.substr ( 2, 2 ) ) );

      unLock (  );
      return;
    }
  }
  string ponderMove = "";
  while ( getRunning (  ) ) {
    init (  );
    ++mply;
    setMainPly ( mply );

    if ( mply == 1 ) {
      memset ( &line, 0, sizeof ( _TpvLine ) );
      val = search ( mply, -_INFINITE, _INFINITE, &line );
    }
    else {
      memset ( &line, 0, sizeof ( _TpvLine ) );

      tmp = search ( mply, val - valWINDOW, val + valWINDOW, &line );
      if ( tmp <= val - valWINDOW || tmp >= val + valWINDOW ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );

	tmp = search ( mply, val - valWINDOW * 2, val + valWINDOW * 2, &line );
      }
      if ( tmp <= val - valWINDOW * 2 || tmp >= val + valWINDOW * 2 ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );

	tmp = search ( mply, val - valWINDOW * 4, val + valWINDOW * 4, &line );
      }
      if ( tmp <= val - valWINDOW * 4 || tmp >= val + valWINDOW * 4 ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );

	tmp = search ( mply, -_INFINITE, _INFINITE, &line );
      }
      val = tmp;
    }
    if ( !getRunning (  ) ) {
      break;
    }
    u64 totMoves = 0;
    if ( mply == 2 )
      setRunning ( 1 );

    memcpy ( &move2, line.argmove, sizeof ( _Tmove ) );
    pvv.clear (  );
    string pvvTmp;

    for ( int t = 0; t < line.cmove; t++ ) {
      pvvTmp.clear (  );
      pvvTmp += decodeBoardinv ( line.argmove[t].type, line.argmove[t].from, getSide (  ) );
      if ( pvvTmp.length (  ) != 4 ) {
	pvvTmp += decodeBoardinv ( line.argmove[t].type, line.argmove[t].to, getSide (  ) );
      }
      pvv += pvvTmp;
      if ( t == 1 )
	ponderMove = pvvTmp;
      pvv += " ";
    };

    memcpy ( &resultMove, &move2, sizeof ( _Tmove ) );
    incKillerHeuristic ( resultMove.from, resultMove.to, 0x800 );
    ftime ( &end1 );
    TimeTaken = _time::diffTime ( end1, start1 );
    totMoves += getTotMoves (  );

    if ( !pvv.length (  ) ) {	//TODO
      break;
    }
    //assert(pvv.length());
    int sc = resultMove.score / 100;;
    if ( resultMove.score > _INFINITE - 100 )
      sc = 0x7fffffff;
#ifdef DEBUG_MODE
    int totStoreHash = nRecordHashA + nRecordHashB + nRecordHashE + 1;
    int percStoreHashA = nRecordHashA * 100 / totStoreHash;
    int percStoreHashB = nRecordHashB * 100 / totStoreHash;
    int percStoreHashE = nRecordHashE * 100 / totStoreHash;

    int totCutHash = n_cut_hashA + n_cut_hashB + n_cut_hashE + 1;
    int percCutHashA = n_cut_hashA * 100 / totCutHash;
    int percCutHashB = n_cut_hashB * 100 / totCutHash;
    int percCutHashE = n_cut_hashE * 100 / totCutHash;

    cout << endl << "info string ply: " << mply << endl;
    cout << "info string tot moves: " << totMoves << endl;
    cout << "info string hash stored " << totStoreHash * 100 / cumulativeMovesCount << "% (alpha=" << percStoreHashA << "% beta=" << percStoreHashB << "% exact=" << percStoreHashE << "%)" << endl;
    cout << "info string cut hash " << totCutHash * 100 / cumulativeMovesCount << "% (alpha=" << percCutHashA << "% beta=" << percCutHashB << "% exact=" << percCutHashE << "%)" << endl;
    u64 nps = 0;
    if ( TimeTaken )
      nps = totMoves * 1000 / TimeTaken;
    if ( nCutAB ) {
      betaEfficiencyCumulative += betaEfficiency / totGen * 10;
      cout << "info string beta efficiency: " << ( int ) betaEfficiencyCumulative << "%" << endl;
      betaEfficiency = totGen = 0.0;
    }
    cout << "info string millsec: " << TimeTaken << "  (" << nps / 1000 << "k nodes per seconds) " << endl;
    cout << "info string alphaBeta cut: " << nCutAB << endl;
    cout << "info string lazy eval cut: " << LazyEvalCuts << endl;
    cout << "info string futility pruning cut: " << nCutFp << endl;
    cout << "info string razor cut: " << nCutRazor << endl;
    cout << "info string null move cut: " << nNullMoveCut << endl;
    cout << "info string insufficientMaterial cut: " << nCutInsufficientMaterial << endl;
#endif

    cout << "info score cp " << sc << " depth " << ( int ) mply << " nodes " << totMoves << " time " << TimeTaken << " pv " << pvv << endl;
    if ( mply >= maxDepth - 1 )
      break;

  }

  resultMove.capturedPiece = ( resultMove.side ^ 1 ) == WHITE ? getPieceAt < WHITE > ( POW2[resultMove.to] ) : getPieceAt < BLACK > ( POW2[resultMove.to] );
  string bestmove = decodeBoardinv ( resultMove.type, resultMove.from, resultMove.side );
  if ( !( resultMove.type & ( KING_SIDE_CASTLE_MOVE_MASK | QUEEN_SIDE_CASTLE_MOVE_MASK ) ) ) {
    bestmove += decodeBoardinv ( resultMove.type, resultMove.to, resultMove.side );
    if ( resultMove.promotionPiece != -1 )
      bestmove += tolower ( FEN_PIECE[( uchar ) resultMove.promotionPiece] );
  }

  cout << "bestmove " << bestmove;
  if ( ponderEnabled && ponderMove.size (  ) ) {
    cout << " ponder " << ponderMove;
  }
  cout << endl << flush;
  pushMovesPath ( decodeBoard ( bestmove.substr ( 0, 2 ) ) );
  pushMovesPath ( decodeBoard ( bestmove.substr ( 2, 2 ) ) );
  unLock (  );
}
