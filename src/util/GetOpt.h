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


class GetOpt {

private:
    static void help(char **argv) {
        string exe = FileUtil::getFileName(argv[0]);
        cout << "Perft test:\t\t\t\t\t" << exe << " " << PERFT_HELP << "\n";
        cout << "Distance to mate:\t\t\t" << exe << " " << DTM_HELP << "\n";
        cout << "Create .pgn from .epd:\t\t" << exe << " " << EPD2PGN_HELP << endl;
        cout << "Generate endgame epd file:\t" << exe << " " << ENDGAME_HELP << endl;
    }

    static void perft(int argc, char **argv) {
        if (string(optarg) != "erft") {
            help(argv);
            return;
        };
        int nCpu = 0;
        int perftDepth = 0;
        string fen;
        int perftHashSize = 0;
        string dumpFile;
        int opt;
        string iniFile;
        while ((opt = getopt(argc, argv, "d:f:h:f:c:F:")) != -1) {
            if (opt == 'd') {    //depth
                perftDepth = atoi(optarg);
            } else if (opt == 'F') { //use dump
                dumpFile = optarg;
                if (dumpFile.empty()) {
                    cout << "use: " << argv[0] << " " << PERFT_HELP << endl;
                    return;
                }
            } else if (opt == 'c') {  //N cpu
                nCpu = atoi(optarg);
            } else if (opt == 'h') {  //hash
                perftHashSize = atoi(optarg);
            } else if (opt == 'f') {  //fen
                fen = optarg;
            }
//            else if (opt == 'i') {  //distributed
//                iniFile = optarg;
//            }
        }

//        if (iniFile.size()) {
//            //"/home/geko/workspace/workspace_my/cinnamon/src/perft_distributed_nodes.ini"
//            new Uci(fen, perftDepth, iniFile);//perft distributed
//        } else {
            new Uci(fen, perftDepth, nCpu, perftHashSize, dumpFile);//perft locale
//        }
    }

    static void epd2pgn(int argc, char **argv) {
        string epdfile;
        int m = 64;
        int opt;
        while ((opt = getopt(argc, argv, "f:m:")) != -1) {
            if (opt == 'f') {    //file
                epdfile = optarg;
            }
            if (opt == 'm') {    //n' pieces
                string h = optarg;
                m = stoi(h);
            }
        }

        ifstream inData;
        string fen;
        if (!FileUtil::fileExists(epdfile)) {
            cout << "error file not found  " << epdfile << endl;
            return;
        }
        inData.open(epdfile);
        int count = 0;
        int n = 0;
        ostringstream os;
        os << "[Date \"" << Time::getYear() << "." << Time::getMonth() << "." << Time::getDay() << "\"]";
        string date = os.str();
        while (!inData.eof()) {
            getline(inData, fen);
            n = 0;
            for (unsigned i = 0; i < fen.size(); i++) {
                int c = tolower(fen[i]);
                if (c == ' ') {
                    break;
                }
                if (c == 'b' || c == 'k' || c == 'r' || c == 'q' || c == 'p' || c == 'n') {
                    n++;
                }
            }
            if (n > 0 && n <= m) {
                count++;
                cout << "[Site \"" << count << " (" << n << " pieces)\"]\n";
                cout << date << "\n";
                cout << "[Result \"*\"]\n";
                string fenClean, token;
                istringstream uip(fen, ios::in);
                uip >> token;
                fenClean += token + " ";
                uip >> token;
                fenClean += token + " ";
                uip >> token;
                fenClean += token + " ";
                uip >> token;
                fenClean += token;
                cout << "[FEN \"" << fenClean << "\"]\n";
                cout << "*" << "\n";
            }
        }
        cout << endl;
    }

    static void dtm(int argc, char **argv) {
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();
        searchManager.createGtb();
        string fen, token;
        IterativeDeeping it;
        int opt;
        while ((opt = getopt(argc, argv, "f:p:s:i:")) != -1) {
            if (opt == 'f') {    //fen
                fen = optarg;
            } else if (opt == 'p') { //path
                token = optarg;
                searchManager.getGtb().setPath(token);
            } else if (opt == 's') { //scheme
                token = optarg;
                if (!searchManager.getGtb().setScheme(token)) {
                    cout << "set scheme error" << endl;
                    return;
                }
            } else if (opt == 'i') {
                token = optarg;
                if (!searchManager.getGtb().setInstalledPieces(stoi(token))) {
                    cout << "set installed pieces error" << endl;
                    return;
                }
            }
        }
        if (!it.getGtbAvailable()) {
            cout << "error TB not found" << endl;
            return;
        }
        searchManager.loadFen(fen);
        searchManager.printDtm();
    }

public:

    static void parse(int argc, char **argv) {
        if (argc == 2 && !strcmp(argv[1], "--help")) {
            help(argv);
            return;
        }

        int opt;
        while ((opt = getopt(argc, argv, "p:e:hd:bf:")) != -1) {
            if (opt == 'h') {
                help(argv);
                return;
            }
            if (opt == 'p') {  // perft test
                perft(argc, argv);
                return;
            } else {
                if (opt == 'e') {
                    if (string(optarg) == "pd2pgn") {
                        epd2pgn(argc, argv);
                        return;
                    } else if (string(optarg) == "ndgame_epd") {
                        while ((opt = getopt(argc, argv, "t:")) != -1) {
                            if (opt == 't') {    //file
                                Search a;
                                a.generatePuzzle(optarg);
                                return;
                            }
                        }
                    } else {
                        help(argv);
                        return;
                    }
                } else if (opt == 'b') {
                    unique_ptr<IterativeDeeping> it(new IterativeDeeping());
                    it->setUseBook(false);
                    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
                    searchManager.setMaxTimeMillsec(10000);
                    it->run();
                    return;
                } else if (opt == 'd') {  // gtb dtm
                    if (string(optarg) != "tm") {
                        cout << "use: " << argv[0] << " " << DTM_HELP << endl;
                        return;
                    };
                    dtm(argc, argv);
                    return;
                }
            }
        }
        Uci::getInstance();
    }
};



