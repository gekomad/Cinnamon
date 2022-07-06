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

#include "util/Singleton.h"
#include "perft/Perft.h"
#include "util/getopt.h"
#include "util/tuning/Texel.h"

static const string
        PERFT_HELP = "-perft [-d depth] [-c nCpu] [-h hash size (mb) [-F dump file]] [-Chess960] [-f \"fen position\"]";
static const string DTM_GTB_HELP = "-dtm-gtb -f \"fen position\" -p path [-s scheme] [-i installed pieces]";
static const string WDL_GTB_HELP = "-wdl-gtb -f \"fen position\" -p path [-s scheme] [-i installed pieces]";
static const string DTZ_SYZYGY_HELP = "-dtz-syzygy -f \"fen position\" -p path";
static const string WDL_SYZYGY_HELP = "-wdl-syzygy -f \"fen position\" -p path";
static const string PUZZLE_HELP = "-puzzle_epd -t K?K? ex: KRKP | KQKP | KBBKN | KQKR | KRKB | KRKN ...";

class GetOpt {

private:
    static void printHeader(const string &exe) {

        cout << NAME << " UCI chess engine by Giuseppe Cannella\n";

#ifdef HAS_POPCNT
        cout << "popcnt ";
#endif
#ifdef HAS_BSF
        cout << "bsf ";
#endif
#ifdef USE_BMI2
        cout << "bmi2 ";
#endif
        cout << "compiled " << __DATE__ << " with ";
#if defined(__clang__)
        cout << "Clang/LLVM " << __clang_major__ << "." << __clang_minor__ << "." << __clang_patchlevel__;
#elif defined(__ICC) || defined(__INTEL_COMPILER)
        cout << "Intel ICC " << __VERSION__;
#elif defined(__GNUC__) || defined(__GNUG__)
        cout << "GNU GCC " << __VERSION__;
#elif defined(__HP_cc) || defined(__HP_aCC)
        cout << "Hewlett-Packard aC++" << __HP_aCC;
#elif defined(__IBMC__) || defined(__IBMCPP__)
        cout << "IBM XL C++ <<"__IBMCPP__;
#elif defined(_MSC_VER)
        cout << "Microsoft Visual Studio. " << _MSC_VER;
#elif defined(__PGI)
        cout << "Portland Group PGCC/PGCPP " << __PGIC__;
#elif defined(__SUNPRO_C) || defined(__SUNPRO_CC)
        cout << "Oracle Solaris Studio " << __SUNPRO_CC;
#else
        cout << "Unknown compiler";
#endif
        cout << "\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n";
        cout << "Run " << exe << " -h for more commands." << endl << endl;

        DEBUG(cout << "DEBUG_MODE" << endl)
        DEBUG(cout << "Log level: " << LOG_LEVEL_STRING[DLOG_LEVEL] << endl)
        cout << flush;
    }

    static void help(char **argv) {
        string exe = FileUtil::getFileName(argv[0]);
        cout << "Perft test:            " << exe << " " << PERFT_HELP << endl;
        cout << "DTM (gtb):             " << exe << " " << DTM_GTB_HELP << endl;
        cout << "WDL (gtb):             " << exe << " " << WDL_GTB_HELP << endl;
        cout << "DTZ (syzygy):          " << exe << " " << DTZ_SYZYGY_HELP << endl;
        cout << "WDL (syzygy):          " << exe << " " << WDL_SYZYGY_HELP << endl;
        cout << "Generate puzzle epd:   " << exe << " " << PUZZLE_HELP << endl;
    }

    static void perft(int argc, char **argv) {
        if (string(optarg) != "erft") {
            help(argv);
            return;
        }
        int nCpu = 0;
        int perftDepth = 0;
        string fen;
        int perftHashSize = 0;
        string dumpFile;
        bool chess960 = false;
        int opt;
        string iniFile;
        bool useDump = false;
        while ((opt = getopt1(argc, argv, "d:f:h:f:c:F:9:C:")) != -1) {
            if (opt == 'd') {    //depth
                perftDepth = atoi(optarg);
            } else if (opt == 'c') {  //N cpu
                nCpu = atoi(optarg);
            } else if (opt == 'h') {  //hash
                perftHashSize = atoi(optarg);
            } else if (opt == 'F') { //use dump
                dumpFile = optarg;
                if (dumpFile.empty()) {
                    cout << "use: " << argv[0] << " " << PERFT_HELP << endl;
                    return;
                }
                useDump = true;
            } else if (opt == 'f') {  //fen
                fen = optarg;
            } else if (opt == 'C') {  //chess960
                if (!string(optarg).compare("hess960"))
                    chess960 = true;
            }
        }
        if (useDump && !FileUtil::fileExists(dumpFile) && !perftHashSize) {
            cout << "Error: with '-F' parameter you have to specify either an existing dump file or a hash size (-h)"
                 << endl << endl;
            help(argv);
            return;
        }
        if (useDump && FileUtil::fileExists(dumpFile) && perftHashSize) {
            cout << "Error: with '-F' parameter and existing dump file you can't specify hash size (-h)" << endl
                 << endl;
            help(argv);
            return;
        }
        Perft *perft = &Perft::getInstance();
        perft->setParam(fen, perftDepth, nCpu, perftHashSize, dumpFile, chess960);
        perft->start();
        perft->join();
    }

#ifndef JS_MODE

    static void dtmWdlGtb(int argc, char **argv, const bool dtm) {
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();

        GTB *gtb = &GTB::getInstance();
        string fen, token;
        IterativeDeeping it;
        int opt;
        while ((opt = getopt1(argc, argv, "f:p:s:i:")) != -1) {
            if (opt == 'f') {    //fen
                fen = optarg;
            } else if (opt == 'p') { //path
                token = optarg;
                gtb->setPath(token);
            } else if (opt == 's') { //scheme
                token = optarg;
                if (!gtb->setScheme(token)) {
                    cout << "set scheme error" << endl;
                    return;
                }
            } else if (opt == 'i') {
                token = optarg;
                if (!gtb->setInstalledPieces(stoi(token))) {
                    cout << "set installed pieces error" << endl;
                    return;
                }
            }
        }
        if (gtb == nullptr) {
            cout << "error GTB not found" << endl;
            return;
        }
        searchManager.loadFen(fen);
        searchManager.printDtmGtb(dtm);
    }

    static void createSyzygy(int argc, char **argv) {
        string fen, token;
        IterativeDeeping it;
        int opt;
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();

        while ((opt = getopt1(argc, argv, "f:p:s:i:")) != -1) {
            if (opt == 'f') {    //fen
                fen = optarg;
                searchManager.loadFen(fen);
            } else if (opt == 'p') { //path
                token = optarg;
                SYZYGY::getInstance().createSYZYGY(token);
            }
        }
    }

    static void wdlSyzygy(int argc, char **argv) {

        createSyzygy(argc, argv);
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();
        searchManager.printWdlSyzygy();

    }

    static void dtmSyzygy(int argc, char **argv) {

        createSyzygy(argc, argv);
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();
        searchManager.printDtmSyzygy();

    }

#endif
public:

    static void parse(int argc, char **argv) {
#ifdef NDEBUG
        assert(0);
#endif
#ifdef TUNING
            if (argc != 2) {
                cout << Texel::help << endl;
                cout << "run " << FileUtil::getFileName(argv[0]) << " path" << endl;
                return;
            }
            new Texel(argv[1]);
	    return;
#endif
        if (!(argc > 1 && !strcmp("-puzzle_epd", argv[1])))
            printHeader(FileUtil::getFileName(argv[0]));
        if (argc == 2 && !strcmp(argv[1], "--help")) {
            help(argv);
            return;
        }

        int opt;

        while ((opt = getopt1(argc, argv, "p:e:hd:b:f:w:")) != -1) {
            if (opt == 'h') {
                help(argv);
                return;
            }
            if (opt == 'p') {  // perft test
                if (string(optarg) == "erft") {
                    perft(argc, argv);
                } else if (string(optarg) == "uzzle_epd") {
                    while ((opt = getopt1(argc, argv, "t:")) != -1) {
                        if (opt == 't') {    //file
                            Search a;
                            if (!a.generatePuzzle(optarg)) {
                                cout << "error use: " << PUZZLE_HELP << endl;
                            }
                            return;
                        }
                    }
                }
                return;
            }
#ifndef JS_MODE
            else {
                if (opt == 'e') {
                    help(argv);
                    return;
                } else if (opt == 'd') {
                    if (string(optarg) == "tm-gtb") {
                        dtmWdlGtb(argc, argv, true);
                        return;
                    } else if (string(optarg) == "tz-syzygy") {
                        dtmSyzygy(argc, argv);
                        return;
                    }
                    return;
                } else if (opt == 'w') {
                    if (string(optarg) == "dl-gtb") {
                        dtmWdlGtb(argc, argv, false);
                        return;
                    } else if (string(optarg) == "dl-syzygy") {
                        wdlSyzygy(argc, argv);
                        return;
                    }
                    return;
                }

            }
#endif
        }
        Uci::getInstance();
    }

};
