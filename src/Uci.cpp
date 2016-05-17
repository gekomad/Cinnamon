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

#include "Uci.h"

Uci::Uci(const string &fen, const int perftDepth, const int nCpu, const int perftHashSize, const string &dumpFile) {//perft locale
    perft = &Perft::getInstance();
    perft->setParam(fen, perftDepth, nCpu, perftHashSize, dumpFile);
    runPerftAndExit = true;
    startListner();
}

void Uci::startListner() {
    IterativeDeeping i;
    listner(&i);
}

Uci::Uci() {
    startListner();
}

Uci::~Uci() { }

void Uci::getToken(istringstream &uip, String &token) {
    token.clear();
    uip >> token;
    token.toLower();
}

void Uci::listner(IterativeDeeping *it) {
    string command;
    bool knowCommand;
    String token;
    bool stop = false;
    int lastTime = 0;
    uciMode = false;
    int perftThreads = 1;
    int perftHashSize = 0;
    string dumpFile;
    static const string _BOOLEAN[] = {"false", "true"};
    syzygy = &searchManager.createSYZYGY();

    while (!stop) {
        if (runPerftAndExit) {
            runPerftAndExit = false;
            perft->start();
            perft->join();
            break;
        }
        if (!getline(cin, command)) {
            break;
        }
        istringstream uip(command, ios::in);
        getToken(uip, token);
        knowCommand = false;

        if (token == "perft") {
            //compatible with ARENA (http://auriga-cinnamon.rhcloud.com)
            string fen;
            getToken(uip, token);
            int perftDepth = stoi(token);
            if (perftDepth > MAX_PLY || perftDepth <= 0) {
                perftDepth = 1;
            }
            int hashDepth = searchManager.getHashSize();
            searchManager.setHashSize(0);
            if (fen.empty()) {
                fen = searchManager.getFen();
            }
            searchManager.setHashSize(hashDepth);
            perft = &Perft::getInstance();
            perft->setParam(fen, perftDepth, perftThreads, perftHashSize, dumpFile);
            perft->join();
            perft->start();
            perft->join();
            knowCommand = true;
        }

        else if (token == "dump") {
            knowCommand = true;
            if (perft)perft->dump();
        }
        else if (token == "quit") {
            knowCommand = true;
            searchManager.setRunning(false);
            stop = true;
        } else if (token == "ponderhit") {
            knowCommand = true;
            searchManager.startClock();
            searchManager.setMaxTimeMillsec(lastTime - lastTime / 3);
            searchManager.setPonder(false);
        } else if (token == "display") {
            knowCommand = true;
            searchManager.display();
        } else if (token == "isready") {
            knowCommand = true;
            cout << "readyok\n";
        } else if (token == "uci") {
            knowCommand = true;
            uciMode = true;
            cout << "id name " << NAME << "\n";
            cout << "id author Giuseppe Cannella\n";
            cout << "option name Hash type spin default 64 min 1 max 1000\n";
            cout << "option name Clear Hash type button\n";
            cout << "option name Nullmove type check default true\n";
            cout << "option name Book File type string default cinnamon.bin\n";
            cout << "option name OwnBook type check default " << _BOOLEAN[it->getUseBook()] << "\n";
            cout << "option name Ponder type check default " << _BOOLEAN[it->getPonderEnabled()] << "\n";
            cout << "option name Threads type spin default 1 min 1 max 64\n";
            cout << "option name TB Endgame type combo default none var Gaviota var none\n";
            cout << "option name GaviotaTbPath type string default gtb/gtb4\n";
            cout << "option name GaviotaTbCache type spin default 32 min 1 max 1024\n";
            cout << "option name GaviotaTbScheme type combo default cp4 var none var cp1 var cp2 var cp3 var cp4\n";
            cout << "option name TB Pieces installed type combo default 3 var none var 3 var 4 var 5\n";
            cout << "option name TB probing depth type spin default 0 min 0 max 5\n";
            cout << "option name TB Restart type button\n";

            cout << "option name PerftThreads type spin default 1 min 1 max 64\n";
            cout << "option name PerftHashSize type spin default 0 min 0 max 100000\n";
            cout << "option name PerftDumpFile type string\n";
            cout << "uciok\n";
        } else if (token == "score") {
            int side = searchManager.getSide();
            int t = searchManager.getScore(side, true);

            if (!searchManager.getSide()) {
                t = -t;
            }
            cout << "Score: " << t << "\n";
            knowCommand = true;
        } else if (token == "stop") {
            knowCommand = true;
            searchManager.setPonder(false);
            searchManager.setRunning(0);
            searchManager.setRunningThread(false);
        } else if (token == "ucinewgame") {
            while (it->getRunning())usleep(5);
            searchManager.loadFen();
            searchManager.clearHash();
            knowCommand = true;
        } else if (token == "setvalue") {
            getToken(uip, token);
            String value;
            getToken(uip, value);
            knowCommand = searchManager.setParameter(token, stoi(value));
        } else if (token == "setoption") {
            getToken(uip, token);
            if (token == "name") {
                getToken(uip, token);
                if (token == "gaviotatbpath") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        gtb = &searchManager.createGtb();
                        gtb->setPath(token);
                    }
                }

                else if (token == "perftthreads") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        perftThreads = stoi(token);
                        knowCommand = true;
                    }
                } else if (token == "perfthashsize") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        perftHashSize = stoi(token);
                        knowCommand = true;
                    }
                } else if (token == "perftdumpfile") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        dumpFile = token;
                        knowCommand = true;
                    }
                } else if (token == "gaviotatbcache") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (gtb->setCacheSize(stoi(token))) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "threads") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = searchManager.setNthread(stoi(token));
                    }
                } else if (token == "gaviotatbscheme") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (gtb->setScheme(token)) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "tb") {
                    getToken(uip, token);
                    if (token == "pieces") {
                        getToken(uip, token);
                        if (token == "installed") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
                                if (gtb->setInstalledPieces(stoi(token))) {
                                    knowCommand = true;
                                };
                            }
                        }
                    } else if (token == "endgame") {
                        getToken(uip, token);
                        if (token == "value") {
                            getToken(uip, token);
                            knowCommand = true;
                            if (token == "none") {
                                searchManager.deleteGtb();
                                knowCommand = true;
                            } else if (token == "gaviota") {
                                knowCommand = true;
                            }
                        }
                    } else if (token == "restart") {
                        knowCommand = true;
                        gtb->restart();
                    } else if (token == "probing") {
                        getToken(uip, token);
                        if (token == "depth") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
                                if (gtb->setProbeDepth(stoi(token))) {
                                    knowCommand = true;
                                };
                            }
                        }
                    }
                } else if (token == "hash") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        searchManager.setHashSize(stoi(token));
                        knowCommand = true;
                    }
                } else if (token == "nullmove") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        searchManager.setNullMove(token == "true");
                    }
                } else if (token == "ownbook") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        it->setUseBook(token == "true");
                        knowCommand = true;
                    }
                } else if (token == "book") {
                    getToken(uip, token);
                    if (token == "file") {
                        getToken(uip, token);
                        if (token == "value") {
                            getToken(uip, token);
                            it->loadBook(token);
                            knowCommand = true;
                        }
                    }
                } else if (token == "ponder") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        it->enablePonder(token == "true");
                        knowCommand = true;
                    }
                } else if (token == "clear") {
                    getToken(uip, token);
                    if (token == "hash") {
                        knowCommand = true;
                        searchManager.clearHash();
                    }
                }
            }
        } else if (token == "position") {
            while (it->getRunning())usleep(5);
            knowCommand = true;
            searchManager.setRepetitionMapCount(0);
            getToken(uip, token);
            _Tmove move;
            if (token == "startpos") {
                it->setUseBook(it->getUseBook());
                searchManager.loadFen();
                getToken(uip, token);
            }
            if (token == "fen") {
                string fen;
                while (token != "moves" && !uip.eof()) {
                    uip >> token;
                    fen.append(token);
                    fen.append(" ");
                };
                searchManager.init();
                int x = searchManager.loadFen(fen);
                searchManager.setSide(x);
                searchManager.pushStackMove();
            }
            if (token == "moves") {
                while (!uip.eof()) {
                    uip >> token;
                    int x = !searchManager.getMoveFromSan(token, &move);
                    searchManager.setSide(x);
                    searchManager.makemove(&move);
                }
            }

        } else if (token == "go") {
            it->setMaxDepth(MAX_PLY);
            int wtime = 200000; //5 min
            int btime = 200000;
            int winc = 0;
            int binc = 0;
            bool forceTime = false;
            bool setMovetime = false;
            while (!uip.eof()) {
                getToken(uip, token);
                if (token == "wtime") {
                    uip >> wtime;
                } else if (token == "btime") {
                    uip >> btime;
                } else if (token == "winc") {
                    uip >> winc;
                } else if (token == "binc") {
                    uip >> binc;
                } else if (token == "depth") {
                    int depth;
                    uip >> depth;
                    if (depth > MAX_PLY) {
                        depth = MAX_PLY;
                    }
                    if (!setMovetime) {
                        searchManager.setMaxTimeMillsec(0x7FFFFFFF);
                    }
                    it->setMaxDepth(depth);
                    forceTime = true;
                } else if (token == "movetime") {
                    int tim;
                    uip >> tim;
                    setMovetime = true;
                    searchManager.setMaxTimeMillsec(tim);
                    forceTime = true;
                } else if (token == "infinite") {
                    searchManager.setMaxTimeMillsec(0x7FFFFFFF);
                    forceTime = true;
                } else if (token == "ponder") {
                    searchManager.setPonder(true);
                }
            }
            if (!forceTime) {
                if (searchManager.getSide() == WHITE) {
                    winc -= (int) (winc * 0.1);
                    searchManager.setMaxTimeMillsec(winc + wtime / 40);
                    if (btime > wtime) {
                        searchManager.setMaxTimeMillsec(searchManager.getMaxTimeMillsec(0) - (int) (searchManager.getMaxTimeMillsec(0) * ((135.0 - wtime * 100.0 / btime) / 100.0)));
                    }
                } else {
                    binc -= (int) (binc * 0.1);
                    searchManager.setMaxTimeMillsec(binc + btime / 40);
                    if (wtime > btime) {
                        searchManager.setMaxTimeMillsec(searchManager.getMaxTimeMillsec(0) - (int) (searchManager.getMaxTimeMillsec(0) * ((135.0 - btime * 100.0 / wtime) / 100.0)));
                    }
                }
                lastTime = searchManager.getMaxTimeMillsec(0) * 3;
                searchManager.setMaxTimeMillsec(lastTime);
            }
            if (!uciMode) {
                searchManager.display();
            }
            it->join();
            it->start();
            knowCommand = true;
        }
        if (!knowCommand) {
            cout << "Unknown command: " << command << "\n";
        };
        cout << flush;
    }
}
