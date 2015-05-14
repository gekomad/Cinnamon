#include "IterativeDeeping.h"

IterativeDeeping::IterativeDeeping ( Search * b ) {
  search = b;
  ponderEnabled = false;
  ponder = 0;
}

IterativeDeeping::~IterativeDeeping (  ) {
}

void
IterativeDeeping::enablePonder ( bool b ) {
  ponderEnabled = b;
}

void
IterativeDeeping::setPonder ( bool b ) {
  return;
  if ( !b )
    ponder = 2;
  else
    ponder = 1;
  search->setRunning ( b );
}

void
IterativeDeeping::run (  ) {
  int side = search->getSide (  );
  _Tmove result_move;
  memset ( &result_move, 0, sizeof ( result_move ) );
  struct timeb start1;
  struct timeb end1;
  _TpvLine line;
  int val = 0, tmp;
  string pvv;
  _Tmove move2;
  int TimeTaken;
  search->setRunning ( 2 );
  int mply = 0;
  search->startClock (  );
  search->clearKillerHeuristic (  );
  search->clearAge (  );

  ftime ( &start1 );
  INC ( halfMove );
  string ponderMove;
  u64 key = search->makeZobristKey (  );
  while ( search->getRunning (  ) ) {
    if ( mply == MAX_PLY - 3 )
      break;
    search->init (  );
    ++mply;
#ifdef DEBUG_MODE
    cout << "info string ply: " << mply << " ...\n";
#endif
    search->setMainPly ( mply );
    if ( mply == 1 ) {
      memset ( &line, 0, sizeof ( _TpvLine ) );
      val = search->search ( key, side, 1, -_INFINITE, _INFINITE, &line );
    }
    else {
      memset ( &line, 0, sizeof ( _TpvLine ) );
      tmp = search->search ( key, side, mply, val - valWINDOW, val + valWINDOW, &line );
      if ( tmp <= val - valWINDOW || tmp >= val + valWINDOW ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );
	tmp = search->search ( key, side, mply, val - valWINDOW * 2, val + valWINDOW * 2, &line );
      }
      if ( tmp <= val - valWINDOW * 2 || tmp >= val + valWINDOW * 2 ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );
	tmp = search->search ( key, side, mply, val - valWINDOW * 4, val + valWINDOW * 4, &line );
      }
      if ( tmp <= val - valWINDOW * 4 || tmp >= val + valWINDOW * 4 ) {
	memset ( &line, 0, sizeof ( _TpvLine ) );
	tmp = search->search ( key, side, mply, -_INFINITE, _INFINITE, &line );
      }
      val = tmp;
    }
    if ( !search->getRunning (  ) ) {
      break;
    }
    u64 num_tot_moves = 0;
    if ( mply == 2 )
      search->setRunning ( 1 );



    memcpy ( &move2, line.argmove, sizeof ( _Tmove ) );
    pvv.clear (  );
    string pvv_tmp;

    for ( int t = 0; t < line.cmove; t++ ) {
      pvv_tmp.clear (  );
      pvv_tmp += search->decodeBoardinv ( line.argmove[t].type, line.argmove[t].from, side );
      if ( pvv_tmp.length (  ) != 4 ) {
	pvv_tmp += search->decodeBoardinv ( line.argmove[t].type, line.argmove[t].to, side );
      }
      pvv += pvv_tmp;
      if ( t == 1 )
	ponderMove = pvv_tmp;
      pvv += " ";
    };


    memcpy ( &result_move, &move2, sizeof ( _Tmove ) );
    search->incKillerHeuristic ( result_move.from, result_move.to, 0x800 );
    ftime ( &end1 );
    TimeTaken = diff_time ( end1, start1 );
    num_tot_moves += ( search->getTotMoves (  ) );
    myassert ( pvv.length (  ) );
    int sc = result_move.score / 100;;
    if ( result_move.score > _INFINITE - 100 )
      sc = 2147483647;
    cout << "info score cp " << sc << " depth " << ( int ) mply << " nodes " << num_tot_moves << " time " << TimeTaken << " pv " << pvv << endl << flush;
#ifdef DEBUG_MODE
#ifndef NO_HASH_MODE
    int percCutTot = ( search->n_cut_hashA + search->n_cut_hashB + search->n_cut_hashE ) * 100 / ( search->totmosse + 1 );
    int percCutA = search->n_cut_hashA * 100 / ( search->totmosse + 1 );
    int percCutB = search->n_cut_hashB * 100 / ( search->totmosse + 1 );
    int percCutE = search->n_cut_hashE * 100 / ( search->totmosse + 1 );
    cout << "info string\thalfMove: " << halfMove << "\ttotmosse " << search->totmosse << "\t\thashed " << search->n_record_hash * 100 / search->totmosse << "%\tcut " << percCutTot << "% (A:" << percCutA << "% B:" << percCutB << "% E:" << percCutE << "%)\tcoll " << search->collisions * 100 / ( search->n_record_hash + 1 )
      << "% probeHash: " << search->probeHash * 100 / search->totmosse << "% cutFailed: " << search->cutFailed * 100 / ( search->probeHash + 1 ) << "%" << endl;

#else
    cout << "halfMove: " << halfMove << " totmosse: " << search->totmosse << endl;
#endif
    u64 nps = 0;
    if ( ( TimeTaken ) )
      nps = ( num_tot_moves * 1000 / TimeTaken );
    if ( search->n_cut ) {
      search->beta_efficency_tot += search->beta_efficency / search->totGen * 10;
      cout << "info string beta_efficency:" << search->beta_efficency_tot << endl;
      search->beta_efficency = search->totGen = 0.0;
    }
    cout << "info string millsec: " << TimeTaken << "  (" << nps << " nodes per seconds) \ninfo string tot moves: " << num_tot_moves << endl;
#ifndef NO_FP_MODE
    cout << "info string n_cut: " << search->n_cut << "\ninfo string LazyEvalCuts: " << search->LazyEvalCuts << "\n";
    cout << "info string n_cut_FutilityPruning: " << search->n_cut_fp << "\n";
    cout << "info string n_cut_razor: " << search->n_cut_razor << "\n";
#endif
    cout << "info string null_move_cut: " << search->null_move_cut << "\n";
#ifndef NO_HASH_MODE
    cout << "info string hash_record: " << search->n_record_hash << "\ninfo string n_cut_hash:" << search->n_cut_hash << "\nhash_collisions: " << search->collisions << "\n";
#endif
    cout << flush;
#endif
  }

  result_move.capturedPiece = ( result_move.side ^ 1 ) == WHITE ? search->getPieceAtWhite ( TABLOG[result_move.to] ) : search->getPieceAtBlack ( TABLOG[result_move.to] );
  string bestmove = search->decodeBoardinv ( result_move.type, result_move.from, result_move.side );
  if ( ponder == 0 ) {
    u64 dummy = 0;
    search->makemove ( &result_move, &dummy );
    /* if(result_move.pieceFrom==WHITE||result_move.pieceFrom==BLACK|| result_move.capturedPiece!=12)
       search->pushStackMove(search->makeZobristKey(),true);
       else
       search->pushStackMove(search->makeZobristKey(),false); */
    if ( side == WHITE && result_move.from == 3 && search->getPieceAtWhite ( TABLOG_3 ) == KING_WHITE )
      search->setRightCastle ( search->getRightCastle (  ) & ( ~RIGHT_KING_CASTLE_WHITE_MASK & ~RIGHT_QUEEN_CASTLE_WHITE_MASK ) );
    if ( side == BLACK && result_move.from == 59 && search->getPieceAtBlack ( TABLOG_59 ) == KING_BLACK )
      search->setRightCastle ( search->getRightCastle (  ) & ( ~RIGHT_KING_CASTLE_BLACK_MASK & ~RIGHT_QUEEN_CASTLE_BLACK_MASK ) );

  }

  if ( !( result_move.type & ( KING_SIDE_CASTLE_MOVE_MASK | QUEEN_SIDE_CASTLE_MOVE_MASK ) ) ) {

    bestmove += search->decodeBoardinv ( result_move.type, result_move.to, result_move.side );

    if ( result_move.promotionPiece != 0xFF )
      bestmove += tolower ( FEN_PIECE[result_move.promotionPiece] );
  }

  cout << "bestmove " << bestmove;
  if ( ponderEnabled )
    cout << " ponder " << ponderMove;
  cout << endl << flush;
  ponder = 0;

}
