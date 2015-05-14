#ifndef ITERATIVEDEEPING_H_
#define ITERATIVEDEEPING_H_
#include "Search.h"
#include "Thread.h"
class IterativeDeeping:public Thread {
public:
  IterativeDeeping ( Search * );
  void setPonder ( bool );
   virtual ~ IterativeDeeping (  );
  virtual void run (  );
  void enablePonder ( bool b );
private:
#ifdef DEBUG_MODE
  int halfMove;
#endif
  bool ponderEnabled;
  Search *search;
  int ponder;
};

#endif
