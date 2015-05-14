#ifndef ITERATIVEDEEPING_H_
#define ITERATIVEDEEPING_H_
#include "Search.h"
#include "Thread.h"
class IterativeDeeping:public Thread {
public:
  IterativeDeeping ( Search * );
  virtual ~ IterativeDeeping (  );
  virtual void run (  );
private:
   Tmove result_move;
  Search *search;
  void think ( int );
};

#endif
