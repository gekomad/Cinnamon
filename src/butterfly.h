#if !defined(_butterfly_h)
#define _butterfly_h

void iterative_deeping ( int side );
#ifdef PERFT_MODE
void do_perft (  );
#endif
#ifndef PERFT_MODE
#ifdef _MSC_VER
void hand_do_move (  );
#else
void *hand_do_move ( void * );
#endif
#endif
#endif
