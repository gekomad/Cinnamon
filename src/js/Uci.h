/*
    Cinnamon UCI chess engine
    Copyright (C) Giuseppe Cannella

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string.h>
#include "../IterativeDeeping.h"

class Uci {
 public:
  Uci() {
    iterativeDeeping = new IterativeDeeping(hash, searchManager);
  }

  virtual ~Uci() { delete iterativeDeeping; }

  char *command(const char *cmd, const char *arg) {
    string a = "";
    if (strcmp(cmd, "go") == 0) {
      a = iterativeDeeping->go();
    } else if (strcmp(cmd, "setMaxTimeMillsec") == 0) {
      searchManager->setMaxTimeMillsec(atoi(arg));
    } else if (strcmp(cmd, "position") == 0) {
      searchManager->setRepetitionMapCount(0);
      searchManager->init();
      searchManager->setSide(iterativeDeeping->loadFen(arg));
    }
    return const_cast<char *>(a.c_str());
  }

 private: 
  shared_ptr<Hash> hash = std::make_shared<Hash>();
  shared_ptr<SearchManager> searchManager = std::make_shared<SearchManager>(hash);
  IterativeDeeping * iterativeDeeping = new IterativeDeeping(hash, searchManager);
};
