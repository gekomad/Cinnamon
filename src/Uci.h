#ifndef UCI_H_
#define UCI_H_

#include "IterativeDeeping.h"
#include "Perft.h"

class Uci {
public:
  Uci (  );
  virtual ~ Uci (  );

private:
  bool uciMode;
  void listner (  );
  IterativeDeeping *it;
  void getToken ( istringstream & uip, string & token );
};
#endif
