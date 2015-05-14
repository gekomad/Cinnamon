#if !defined(UWINBOARD_H)
#define UWINBOARD_H

#ifndef PERFT_MODE
#ifdef _MSC_VER
void listner_winboard (  );
#else
void *listner_winboard ( void * );
#endif
void writeWinboard ( char *msg );
#endif
#endif
