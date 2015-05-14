#ifndef TEST_MODE
#ifndef UCI_H_
#define UCI_H_
#include "IterativeDeeping.h"
#include "maindefine.h"
class Uci {
public:
  Uci ( char * );
  virtual ~ Uci (  );
  void listner (  );
private:
   IterativeDeeping * it;
  Search *search;
  int moveFen ( const char *fenStr );
};
#endif
#endif
