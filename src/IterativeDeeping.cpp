#include "IterativeDeeping.h"

IterativeDeeping::IterativeDeeping ( Search * b ) {
  search = b;
}

IterativeDeeping::~IterativeDeeping (  ) {
}

void
IterativeDeeping::run (  ) {
  int side = ( search->isBlackMove (  ) == 1 ? 0 : 1 );
  char t1[20];
  memset ( t1, 0, sizeof ( t1 ) );
  memset ( &result_move, 0, sizeof ( result_move ) );
  think ( side );
  if ( result_move.from == 0 && result_move.to == 0 ) {
    search->incListId (  );
    search->generateMoves ( STANDARD_MOVE_MASK, side );
    search->generateCap ( STANDARD_MOVE_MASK, side );
    int listcount = search->getListCount (  );
    if ( !listcount ) {
      myassert ( 0 );
      search->decListId (  );
      return;
    }
    for ( int ii = 1; ii <= listcount; ii++ ) {
      Tmove *mossa = search->getList ( ii );
      search->makemove ( mossa );
      if ( !( ( side ) == BLACK ? search->attackSquare ( BLACK, BITScanForward ( search->chessboard[KING_BLACK] ) ) : ( search->attackSquare ( WHITE, BITScanForward ( search->chessboard[KING_WHITE] ) ) ) ) ) {
	memcpy ( &result_move, mossa, sizeof ( Tmove ) );
	if ( !search->chessboard[KING_BLACK + side] ) {
	  break;
	}
      }
      search->takeback ( mossa );
    }
    search->decListId (  );
  }
  if ( result_move.from == 0 && result_move.to == 0 ) {
    myassert ( 0 );
  }

  search->makemove ( &result_move );

  if ( side == WHITE && result_move.from == 3 && search->get_piece_at ( side, TABLOG_3 ) == KING_WHITE )
    search->setRightCastle ( search->getRightCastle (  ) & ( ~RIGHT_KING_CASTLE_WHITE_MASK & ~RIGHT_QUEEN_CASTLE_WHITE_MASK ) );
  if ( side == BLACK && result_move.from == 59 && search->get_piece_at ( side, TABLOG_59 ) == KING_BLACK )
    search->setRightCastle ( search->getRightCastle (  ) & ( ~RIGHT_KING_CASTLE_BLACK_MASK & ~RIGHT_QUEEN_CASTLE_BLACK_MASK ) );
  strcat ( t1, search->decodeBoardinv ( result_move.type, result_move.from, result_move.side ) );
  if ( !( result_move.type & ( KING_SIDE_CASTLE_MOVE_MASK | QUEEN_SIDE_CASTLE_MOVE_MASK ) ) ) {
    strcat ( t1, search->decodeBoardinv ( result_move.type, result_move.to, result_move.side ) );
    if ( result_move.promotion_piece != -1 )
      t1[strlen ( t1 )] = ( char ) tolower ( FEN_PIECE[result_move.promotion_piece] );
  }
  cout << "bestmove " << t1 << endl << flush;

}

void
IterativeDeeping::think ( int side ) {
  {
    struct timeb start1;
    struct timeb end1;
    LINE line;
    int val, tmp;
    char pvv[200];
    Tmove move2;
    int TimeTaken;
    search->setRunning ( 2 );
    int mply = 0;
    search->startClock (  );
    search->initKillerHeuristic (  );
#ifdef HASH_MODE
    memset ( hash_array, 0, HASH_SIZE * sizeof ( Thash ) );
#endif
    ftime ( &start1 );
    while ( search->getRunning (  ) ) {
      if ( mply >= MAX_PLY )
	break;
      search->init (  );
      memset ( &line, 0, sizeof ( LINE ) );
      memset ( &pvv, 0, sizeof ( pvv ) );
      ++mply;

#ifdef DEBUG_MODE
      cout << "info string ply: " << mply << " ...\n";
#endif
      search->setMainPly ( mply );
      Tmove mossa;
      memset ( &mossa, 0, sizeof ( Tmove ) );
      if ( 1 ) {
	/// NO ASPIRATION TODO
	search->search ( side, mply, -_INFINITE, _INFINITE, &line
#ifdef DEBUG_MODE
			 , &mossa
#endif
	   );
	///  ASPIRATION
      }
      else {
	if ( mply == 1 ) {
	  val = search->search ( side, 1, -_INFINITE, _INFINITE, &line
#ifdef DEBUG_MODE
				 , &mossa
#endif
	     );
	}
	else {
	  tmp = search->search ( side, mply, val - valWINDOW, val + valWINDOW, &line
#ifdef DEBUG_MODE
				 , &mossa
#endif
	     );
	  if ( tmp <= val - valWINDOW || tmp >= val + valWINDOW ) {
	    memset ( &line, 0, sizeof ( LINE ) );
	    tmp = search->search ( side, mply, -_INFINITE, _INFINITE, &line
#ifdef DEBUG_MODE
				   , &mossa
#endif
	       );
	  }
	  val = tmp;
	}
      }
      u64 num_tot_moves = 0;
      if ( mply == 2 )
	search->setRunning ( 1 );
      ///  END ASPIRATION
      if ( !search->getRunning (  ) )
	return;
      memset ( &move2, 0, sizeof ( Tmove ) );
      memcpy ( &move2, line.argmove, sizeof ( Tmove ) );
      for ( int t = 0; t < line.cmove; t++ ) {
	char pvv_tmp[20];
	memset ( pvv_tmp, 0, sizeof ( pvv_tmp ) );
	strcat ( pvv_tmp, search->decodeBoardinv ( line.argmove[0].type, line.argmove[t].from, side ) );
	if ( line.argmove[t].from >= 0 )
	  strcat ( pvv_tmp, search->decodeBoardinv ( line.argmove[0].type, line.argmove[t].to, side ) );
	strcat ( pvv, pvv_tmp );
	strcat ( pvv, " " );
      };
#ifdef TEST_MODE
      if ( line.argmove[0].from == KING_SIDE_CASTLE_MOVE_MASK ) {
	if ( side )
	  strcpy ( search->test_found, "g1" );
	else
	  strcpy ( search->test_found, "g8" );
      }
      else if ( line.argmove[0].from == QUEEN_SIDE_CASTLE_MOVE_MASK ) {

	if ( side )
	  strcpy ( search->test_found, "c1" );
	else
	  strcpy ( search->test_found, "c8" );
      }
      else
	strcpy ( search->test_found, search->decodeBoardinv ( line.argmove[0].type, line.argmove[0].to, side ) );
#endif
      memcpy ( &result_move, &move2, sizeof ( Tmove ) );

      if ( result_move.from == 0 && result_move.to == 0 ) {
	cout << "mply: " << ( int ) mply << " line.cmove: " << line.cmove << endl;
	myassert ( 0 );
      }

      if ( result_move.from == 0 && result_move.to == 0 ) {
	return;
      }
      if ( result_move.from >= 0 && result_move.from < 64 && result_move.to >= 0 && result_move.to < 64 )
	search->setKillerHeuristic ( result_move.from, result_move.to, 0x800 );

      ftime ( &end1 );
      TimeTaken = diff_time ( end1, start1 );
      num_tot_moves += ( search->getTonMoves (  ) );
      myassert ( strlen ( pvv ) );
      cout << "info score cp " << result_move.score / 100 << " depth " << ( int ) mply << " nodes " << num_tot_moves << " time " << TimeTaken << " pv " << pvv << endl << flush;

      u64 nps = 0;
      if ( ( TimeTaken ) )
	nps = ( num_tot_moves * 1000 / TimeTaken );

#ifdef DEBUG_MODE
      if ( search->n_cut ) {
	double b = search->beta_efficency / ( ( int ) search->n_cut ) * 100;
	cout << "info string beta_efficency:" << b << endl;
	search->beta_efficency_tot += b;
	search->beta_efficency_tot_count++;
      }
      cout << "info string millsec: " << TimeTaken << "  (" << nps << " nodes per seconds) \ninfo string tot moves: " << num_tot_moves << endl;
#ifdef FP_MODE
      cout << "info string n_cut: " << search->n_cut << "\ninfo string LazyEvalCuts: " << search->LazyEvalCuts << "\n";
      cout << "info string n_cut_FutilityPruning: " << search->n_cut_fp << "\n";
      cout << "info string n_cut_razor: " << search->n_cut_razor << "\n";
#endif
#ifdef NULL_MODE
      cout << "info string null_move_cut: " << search->null_move_cut << "\n";
#endif
#ifdef HASH_MODE
      cout << "info string hash_record: " << search->n_record_hash << "\ninfo string n_cut_hash:" << n_cut_hash << "\nhash_collisions: " << collisions << "\n";
#endif
#endif
      cout << flush;
      if ( search->isBlackMove (  ) && result_move.score == _INFINITE )
	break;
      if ( !search->isBlackMove (  ) && result_move.score == -_INFINITE )
	break;
    }
    cout << flush;
  }
}
