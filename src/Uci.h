#ifndef TUNE_CRAFTY_MODE
#ifndef UCI_H_
#define UCI_H_
#include "IterativeDeeping.h"
#include "maindefine.h"

class Uci {
public:
  Uci (  );
  virtual ~ Uci (  );
  void listner (  );
private:
   IterativeDeeping * it;
  Search *search;
  void setPonder ( bool b );
  void getToken ( istringstream & uip, string & token );
  int getMove ( const string fenStr, _Tmove * move );
};
#endif
#endif
