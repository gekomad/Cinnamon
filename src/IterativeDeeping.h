#ifndef ITERATIVEDEEPING_H_
#define ITERATIVEDEEPING_H_

#include "Search.h"
#include "Thread.h"
#include "OpenBook.h"
#include <mutex>

class IterativeDeeping:public Thread, public Search {
public:
  IterativeDeeping (  );
  virtual ~ IterativeDeeping (  );
  virtual void run (  );
  bool getPonderEnabled (  );
  void setUseBook ( bool );
  bool getUseBook (  );
  void unLock (  );
  void lock (  );
  void clearMovesPath (  );
  void enablePonder ( bool );
  void setFollowBook ( bool b );
  void setMaxDepth ( int );
private:
  int maxDepth;
  static const int valWINDOW = 50;
  bool useBook;
  mutex mutex1;
  bool followBook;
  OpenBook *openBook;
  bool ponderEnabled;
  //string bookFileName;
};
#endif
