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
#ifndef JS_MODE

#include "Uci.h"

void Uci::startListner() {
    IterativeDeeping i;
    listner(&i);
}

Uci::Uci() {
    startListner();
}

void Uci::getToken(istringstream &uip, string &token) const {
    token.clear();
    uip >> token;
}

void Uci::listner(IterativeDeeping *it) {
    string command;
    bool knowCommand;
    string token;
    bool stop = false;
    int lastTime = 0;
    uciMode = false;
    int gaviotatbcache = -1;
    int tb_pieces = -1;
    string gaviotatbscheme;
    string dumpFile;
    static const string _BOOLEAN[] = {"false", "true"};
    while (!stop) {
        if (!getline(cin, command)) {
            break;
        }
        istringstream uip(command, ios::in);
        getToken(uip, token);
        knowCommand = false;
        if (String::toLower(token) == "quit") {
            knowCommand = true;
            searchManager.setRunning(false);
            stop = true;
            while (it->getRunning());
        } else if (String::toLower(token) == "ponderhit") {
            knowCommand = true;
            searchManager.startClock();
            searchManager.setMaxTimeMillsec(lastTime - lastTime / 3);
            searchManager.setPonder(false);
        } else if (String::toLower(token) == "display") {
            knowCommand = true;
            searchManager.display();
        } else if (String::toLower(token) == "isready") {
            knowCommand = true;
            cout << "readyok\n";
        } else if (String::toLower(token) == "uci") {
            knowCommand = true;
            uciMode = true;
            cout << "id name " << NAME << endl;
            cout << "id author Giuseppe Cannella" << endl;
            cout << "option name Hash type spin default 64 min 1 max "
                 << (0xffffffff / (1024 * 1024 / (sizeof(Hash::_Thash) * 2))) << endl;
            cout << "option name Clear Hash type button" << endl;
            cout << "option name Nullmove type check default true" << endl;
            cout << "option name Ponder type check default " << _BOOLEAN[it->getPonderEnabled()] << "" << endl;
            cout << "option name Threads type spin default 1 min 1 max 64" << endl;
            cout << "option name UCI_Chess960 type check default false" << endl;
            cout << "option name GaviotaTbPath type string default <empty>" << endl;
            cout << "option name GaviotaTbCache type spin default 32 min 1 max 1024" << endl;
            cout << "option name GaviotaTbScheme type combo default cp4 var none var cp1 var cp2 var cp3 var cp4" <<
                 endl;

            cout << "option name TB Pieces installed type combo default 3 var none var 3 var 4 var 5" << endl;
            cout << "option name TB Restart type button" << endl;
            cout << "option name SyzygyPath type string default <empty>" << endl;
            cout << "uciok" << endl;
        } else if (String::toLower(token) == "score" || String::toLower(token) == "eval") {
            uchar side = searchManager.getSide();
            int t = searchManager.getScore(side);
            if (!searchManager.getSide()) t = -t;
            cout << "\nTotal (white)..........   " << (float) t / 100.0 << endl;
            knowCommand = true;
        } else if (String::toLower(token) == "perft") {
            cout << "Can't run perft here, view \"cinnamon.exe -help\"" << endl;
        } else if (String::toLower(token) == "stop") {
            knowCommand = true;
            searchManager.setPonder(false);
            searchManager.setRunning(0);
            searchManager.setRunningThread(false);
        } else if (String::toLower(token) == "ucinewgame") {
            while (it->getRunning());
            it->plyFromRoot = 0;
            searchManager.loadFen();
            knowCommand = true;
        } else if (String::toLower(token) == "setoption") {
            getToken(uip, token);
            if (String::toLower(token) == "name") {
                getToken(uip, token);
                if (String::toLower(token) == "gaviotatbpath") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        auto gtb = &GTB::getInstance();
                        gtb->setPath(token);
                        if (gaviotatbcache != -1)
                            gtb->setCacheSize(gaviotatbcache);
                        if (!gaviotatbscheme.empty())
                            gtb->setScheme(token);
                        if (tb_pieces != -1)
                            gtb->setInstalledPieces(tb_pieces);
                    }
                } else if (String::toLower(token) == "syzygypath") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        SYZYGY::getInstance().createSYZYGY(token);
                    }
                } else if (String::toLower(token) == "gaviotatbcache") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        gaviotatbcache = stoi(token);
                        GTB *gtb = &GTB::getInstance();
                        if (gtb == nullptr) {
                            knowCommand = true;
                        } else if (gtb->setCacheSize(gaviotatbcache)) {
                            knowCommand = true;
                        }
                    }
                } else if (String::toLower(token) == "threads") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        knowCommand = searchManager.setNthread(stoi(token));
                    }
                } else if (String::toLower(token) == "gaviotatbscheme") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        gaviotatbscheme = token;
                        GTB *gtb = &GTB::getInstance();
                        if (gtb == nullptr) knowCommand = true;
                        else if (gtb->setScheme(token)) {
                            knowCommand = true;
                        }
                    }
                } else if (String::toLower(token) == "tb") {
                    getToken(uip, token);
                    if (String::toLower(token) == "pieces") {
                        getToken(uip, token);
                        if (String::toLower(token) == "installed") {
                            getToken(uip, token);
                            if (String::toLower(token) == "value") {
                                getToken(uip, token);
                                tb_pieces = stoi(token);
                                GTB *gtb = &GTB::getInstance();
                                if (gtb == nullptr)knowCommand = true;
                                else if (gtb->setInstalledPieces(tb_pieces)) {
                                    knowCommand = true;
                                }
                            }
                        }
                    } else if (String::toLower(token) == "restart") {
                        knowCommand = true;
                        GTB *gtb = &GTB::getInstance();
                        if (gtb != nullptr)gtb->restart();
                    }
                } else if (String::toLower(token) == "hash") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        hash.setHashSize(stoi(token));
                        knowCommand = true;
                    }
                } else if (String::toLower(token) == "nullmove") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        searchManager.setNullMove(String::toLower(token) == "true");
                    }
                } else if (String::toLower(token) == "uci_chess960") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        knowCommand = true;
                        searchManager.setChess960(String::toLower(token) == "true");
                    }
                } else if (String::toLower(token) == "ponder") {
                    getToken(uip, token);
                    if (String::toLower(token) == "value") {
                        getToken(uip, token);
                        it->enablePonder(token == "true");
                        knowCommand = true;
                    }
                } else if (String::toLower(token) == "clear") {
                    getToken(uip, token);
                    if (String::toLower(token) == "hash") {
                        knowCommand = true;
                        hash.clearHash();
                    }
                }
            }
        } else if (String::toLower(token) == "position") {
            while (it->getRunning());
            knowCommand = true;
            searchManager.setRepetitionMapCount(0);
            getToken(uip, token);
            _Tmove move;
            if (String::toLower(token) == "startpos") {
                searchManager.loadFen(STARTPOS);
                getToken(uip, token);
            }
            if (String::toLower(token) == "fen") {
                string fen;
                while (String::toLower(token) != "moves" && !uip.eof()) {
                    getToken(uip, token);
                    fen.append(token);
                    fen.append(" ");
                }
                searchManager.init();
                int x = searchManager.loadFen(fen);
                searchManager.setSide(x);
                searchManager.pushStackMove();
            }
            if (String::toLower(token) == "moves") {
                while (!uip.eof()) {
                    getToken(uip, token);
                    if (!token.empty()) {
                        int x = !searchManager.getMoveFromSan(token, &move);
                        searchManager.setSide(x);
                        searchManager.makemove(&move);
                    }
                }
            }

        } else if (String::toLower(token) == "go") {
            it->setMaxDepth(MAX_PLY);
            searchManager.unsetSearchMoves();
            int wtime = 200000; //5 min
            int btime = 200000;
            int winc = 0;
            int binc = 0;
            bool forceTime = false;
            bool setMovetime = false;
            while (!uip.eof()) {
                getToken(uip, token);
                if (String::toLower(token) == "wtime") {
                    uip >> wtime;
                } else if (String::toLower(token) == "btime") {
                    uip >> btime;
                } else if (String::toLower(token) == "winc") {
                    uip >> winc;
                } else if (String::toLower(token) == "binc") {
                    uip >> binc;
                } else if (String::toLower(token) == "depth") {
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
                } else if (String::toLower(token) == "movetime") {
                    int tim;
                    uip >> tim;
                    setMovetime = true;
                    searchManager.setMaxTimeMillsec(tim);
                    forceTime = true;
                } else if (String::toLower(token) == "infinite") {
                    searchManager.setMaxTimeMillsec(0x7FFFFFFF);
                    forceTime = true;
                } else if (String::toLower(token) == "searchmoves") {
                    vector<string> searchmoves;
                    while (!uip.eof()) {
                        uip >> token;
                        searchmoves.push_back(token);
                    }
                    searchManager.setSearchMoves(searchmoves);
                } else if (String::toLower(token) == "ponder") {
                    searchManager.setPonder(true);
                }
            }
            if (!forceTime) {
                if (searchManager.getSide() == WHITE) {
                    int timeForThisMove = wtime / 40 + (winc / 2);
                    if (timeForThisMove >= wtime) timeForThisMove = wtime - 500;
                    if (timeForThisMove < 0) timeForThisMove = 100;
                    searchManager.setMaxTimeMillsec(timeForThisMove);
                } else {
                    int timeForThisMove = btime / 40 + (binc / 2);
                    if (timeForThisMove >= btime) timeForThisMove = btime - 500;
                    if (timeForThisMove < 0) timeForThisMove = 100;
                    searchManager.setMaxTimeMillsec(timeForThisMove);
                }
                lastTime = searchManager.getMaxTimeMillsec();
            }
            if (!uciMode) {
                searchManager.display();
            }
            it->join();
            it->start();
            knowCommand = true;
        }
        if (!knowCommand) {
            cout << "Unknown command: " << command << endl;
        }
        cout << flush;
    }
}

#endif