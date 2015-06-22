/*
    Cinnamon is a UCI chess engine
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

Uci::Uci(Perft *perft1) {
    perft = perft1;
    perft->registerObservers([this]() {
        exit(0);//workaround
    });
    startListner();
}

void Uci::startListner() {
    iterativeDeeping = new IterativeDeeping();
    listner(iterativeDeeping);
}

Uci::Uci() {
    startListner();
}

Uci::~Uci() {
    delete iterativeDeeping;
}

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

    static const string BOOLEAN[] = {"false", "true"};
    while (!stop) {
        getline(cin, command);
        istringstream uip(command, ios::in);
        getToken(uip, token);
        knowCommand = false;
        if (token == "flush") {
            if (perft) {
                perft->dump();
            }
            knowCommand = true;
        } else if (token == "perft") {
            int perftDepth = -1;
            int nCpu = 1;
            string dumpFile;
            int PERFT_HASH_SIZE = 0;
            string fen;
            getToken(uip, token);
            while (!uip.eof()) {
                if (token == "depth") {
                    getToken(uip, token);
                    perftDepth = stoi(token);
                    if (perftDepth > MAX_PLY || perftDepth <= 0) {
                        perftDepth = 2;
                    }
                    getToken(uip, token);
                } else if (token == "ncpu") {
                    getToken(uip, token);
                    nCpu = stoi(token);
                    if (nCpu > 32 || nCpu <= 0) {
                        nCpu = 1;
                    }
                    getToken(uip, token);
                } else if (token == "dumpfile") {
                    getToken(uip, token);
                    dumpFile = token;
                    getToken(uip, token);
                } else if (token == "hash_size") {
                    getToken(uip, token);
                    PERFT_HASH_SIZE = stoi(token);
                    getToken(uip, token);
                } else if (token == "fen") {
                    uip >> token;
                    do {
                        fen += token;
                        fen += ' ';
                        uip >> token;
                    } while (token != "ncpu" && token != "hash_size" && token != "depth" && !uip.eof());
                } else {
                    break;
                }
            }
            if (perftDepth != -1) {
                int hashDepth = it->search[0].getHashSize();
                it->search[0].setHashSize(0);
                if (fen.empty()) {
                    fen = it->search[0].getFen();
                }
                it->search[0].setHashSize(hashDepth);
                //cout << "perft depth " << perftDepth << " nCpu " << nCpu << " hash_size " << PERFT_HASH_SIZE << " fen " << fen << " dumpFile '" << dumpFile << "'\n";
                perft = new Perft(fen, perftDepth, nCpu, PERFT_HASH_SIZE, dumpFile);
                perft->registerObservers([this]() {
                    perft->stop();
                    delete perft;
                    perft = nullptr;
                });
                perft->start();
            } else {
                cout << "use: perft depth d [nCpu n] [hash_size mb] [fen fen_string] [dumpFile file_name]\n";
            }
            knowCommand = true;
        } else if (token == "quit") {
            knowCommand = true;
            it->search[0].setRunning(false);
            stop = true;
        } else if (token == "ponderhit") {
            knowCommand = true;
            it->search[0].startClock();
            it->setMaxTimeMillsec(lastTime - lastTime / 3);
            it->search[0].setPonder(false);
        } else if (token == "display") {
            knowCommand = true;
            it->search[0].display();
        } else if (token == "isready") {
            knowCommand = true;
            it->search[0].setRunning(0);
            cout << "readyok\n";
        } else if (token == "uci") {
            knowCommand = true;
            uciMode = true;
            cout << "id name " << NAME << "\n";
            cout << "id author Giuseppe Cannella" << "\n";
            cout << "option name Hash type spin default 64 min 1 max 32768" << "\n";
            cout << "option name Clear Hash type button" << "\n";
            cout << "option name Nullmove type check default true" << "\n";
            cout << "option name Book File type string default cinnamon.bin" << "\n";
            cout << "option name OwnBook type check default " << BOOLEAN[it->getUseBook()] << "\n";
            cout << "option name Ponder type check default " << BOOLEAN[it->getPonderEnabled()] << "\n";
            cout << "option name TB Endgame type combo default none var Gaviota var none" << "\n";
            cout << "option name GaviotaTbPath type string default gtb/gtb4" << "\n";
            cout << "option name GaviotaTbCache type spin default 32 min 1 max 1024" << "\n";
            cout << "option name GaviotaTbScheme type combo default cp4 var none var cp1 var cp2 var cp3 var cp4" << "\n";
            cout << "option name TB Pieces installed type combo default 3 var none var 3 var 4 var 5" << "\n";
            cout << "option name TB probing depth type spin default 0 min 0 max 5" << "\n";
            cout << "option name TB Restart type button" << "\n";
            cout << "uciok\n";
        } else if (token == "score") {
            int side = it->search[0].getSide();
            int t;
            if (side == WHITE) {
                t = it->search[0].getScore(Bits::bitCount(it->search[0].getBitBoard<WHITE>()), side);
            } else {
                t = it->search[0].getScore(Bits::bitCount(it->search[0].getBitBoard<BLACK>()), side);
            }
            if (!it->search[0].getSide()) {
                t = -t;
            }
            cout << "Score: " << t << "\n";
            knowCommand = true;
        } else if (token == "stop") {
            knowCommand = true;
            it->search[0].setPonder(false);
            it->search[0].setRunning(0);
        } else if (token == "ucinewgame") {
            it->search[0].loadFen();
            it->search[0].clearHash();
            knowCommand = true;
        } else if (token == "setvalue") {
            getToken(uip, token);
            String value;
            getToken(uip, value);
            knowCommand = it->setParameter(token, stoi(value));
        } else if (token == "setoption") {
            getToken(uip, token);
            if (token == "name") {
                getToken(uip, token);
                if (token == "gaviotatbpath") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        it->search[0].createGtb();
                       // it->getGtb().setPath(token);
                    }
                } else if (token == "gaviotatbcache") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
//                        if (it->getGtb().setCacheSize(stoi(token))) {
//                            knowCommand = true;
//                        };
                    }
                } else if (token == "gaviotatbscheme") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
//                        if (it->getGtb().setScheme(token)) {
//                            knowCommand = true;
//                        };
                    }
                } else if (token == "tb") {
                    getToken(uip, token);
                    if (token == "pieces") {
                        getToken(uip, token);
                        if (token == "installed") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
//                                if (it->getGtb().setInstalledPieces(stoi(token))) {
//                                    knowCommand = true;
//                                };
                            }
                        }
                    } else if (token == "endgame") {
                        getToken(uip, token);
                        if (token == "value") {
                            getToken(uip, token);
                            knowCommand = true;
                            if (token == "none") {
                                it->search[0].deleteGtb();
                            } else if (token == "gaviota") {
                              //  it->getGtb();
                            } else {
                                knowCommand = false;
                            }
                        }
                    } else if (token == "restart") {
                        knowCommand = true;
                       // it->getGtb().restart();
                    } else if (token == "probing") {
                        getToken(uip, token);
                        if (token == "depth") {
                            getToken(uip, token);
                            if (token == "value") {
                                getToken(uip, token);
//                                if (it->getGtb().setProbeDepth(stoi(token))) {
//                                    knowCommand = true;
//                                };
                            }
                        }
                    }
                } else if (token == "hash") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        if (it->search[0].setHashSize(stoi(token))) {
                            knowCommand = true;
                        };
                    }
                } else if (token == "nullmove") {
                    getToken(uip, token);
                    if (token == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        it->search[0].setNullMove(token == "true");
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
                        it->search[0].clearHash();
                    }
                }
            }
        } else if (token == "position") {
            lock_guard<mutex> lock(it->mutex1);
            //it->lock();
            knowCommand = true;
            it->search[0].setRepetitionMapCount(0);
            getToken(uip, token);
            _Tmove move;
            if (token == "startpos") {
                it->setUseBook(it->getUseBook());
                it->search[0].loadFen();
                getToken(uip, token);
            }
            if (token == "fen") {
                string fen;
                while (token != "moves" && !uip.eof()) {
                    uip >> token;
                    fen += token;
                    fen += ' ';
                }
                it->search[0].init();
                it->search[0].setSide(it->search[0].loadFen(fen));
                it->search[0].pushStackMove();
            }
            if (token == "moves") {
                while (!uip.eof()) {
                    uip >> token;
                    it->search[0].setSide(!it->search[0].getMoveFromSan(token, &move));
                    it->search[0].makemove(&move);
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
                        it->setMaxTimeMillsec(0x7FFFFFFF);
                    }
                    it->setMaxDepth(depth);
                    forceTime = true;
                } else if (token == "movetime") {
                    int tim;
                    uip >> tim;
                    setMovetime = true;
                    it->setMaxTimeMillsec(tim);
                    forceTime = true;
                } else if (token == "infinite") {
                    it->setMaxTimeMillsec(0x7FFFFFFF);
                    forceTime = true;
                } else if (token == "ponder") {
                    it->search[0].setPonder(true);
                }
            }
            if (!forceTime) {
                if (it->search[0].getSide() == WHITE) {
                    winc -= (int) (winc * 0.1);
                    it->setMaxTimeMillsec(winc + wtime / 40);
                    if (btime > wtime) {
                        it->setMaxTimeMillsec(it->search[0].getMaxTimeMillsec() - (int) (it->search[0].getMaxTimeMillsec() * ((135.0 - wtime * 100.0 / btime) / 100.0)));
                    }
                } else {
                    binc -= (int) (binc * 0.1);
                    it->setMaxTimeMillsec(binc + btime / 40);
                    if (wtime > btime) {
                        it->setMaxTimeMillsec(it->search[0].getMaxTimeMillsec() - (int) (it->search[0].getMaxTimeMillsec() * ((135.0 - btime * 100.0 / wtime) / 100.0)));
                    }
                }
                lastTime = it->search[0].getMaxTimeMillsec();
            }
            if (!uciMode) {
                it->search[0].display();
            }
            it->stop();
            it->start();
            knowCommand = true;
        }
        if (!knowCommand) {
            cout << "Unknown command: " << command << "\n";
        };
        cout << flush;
    }
}
