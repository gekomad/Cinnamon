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

#ifdef TUNING
#pragma once

#include "Tune.h"
#include "../../def.h"
#include <set>
#include <array>

class Texel : Tune {

public:
    static constexpr auto help = "Texel's Tuning Method\n\n"
                                 " Download a big strong main.pgn file\n"
                                 " pgn-extract -Tr0-1       main.pgn >win_black.pgn 2>/dev/null\n"
                                 " pgn-extract -Tr1-0       main.pgn >win_white.pgn 2>/dev/null\n"
                                 " pgn-extract -Tr1/2-1/2   main.pgn >draw.pgn      2>/dev/null\n"
                                 "\n"
                                 " pgn-extract -Wepd win_black.pgn  >win_black.epd  2>/dev/null\n"
                                 " pgn-extract -Wepd win_white.pgn  >win_white.epd  2>/dev/null\n"
                                 " pgn-extract -Wepd draw.pgn       >draw.epd       2>/dev/null\n"
                                 "\n"
                                 " cat win_white.epd  | awk -F \" \" '{print $1\" \"$2\" \"$3}' >foo;sort -u foo |grep \"k\" > win_white.epd\n"
                                 " cat win_black.epd  | awk -F \" \" '{print $1\" \"$2\" \"$3}' >foo;sort -u foo |grep \"k\" > win_black.epd\n"
                                 " cat draw.epd       | awk -F \" \" '{print $1\" \"$2\" \"$3}' >foo;sort -u foo |grep \"k\" > draw.epd\n"
                                 " rm foo win_black.pgn win_white.pgn draw.pgn\n";

    Texel(const string &path) {
        this->path = path;
        cout << "Texel's Tuning Method " << Time::getLocalTime() << " start" << endl;
        set <string> draw;
        set <string> black;
        set <string> white;
        cout << "Fetch epd files..." << endl << flush;

        fetch(path + "/draw.epd", draw);
        fetch(path + "/win_white.epd", white);
        fetch(path + "/win_black.epd", black);

        cout << "ok" << flush << endl;
        cout << "draw size: " << draw.size() << endl;
        cout << "white size: " << white.size() << endl;
        cout << "black size: " << black.size() << endl;
        cout << "Purge fen..." << flush;
        clean(draw, black, white);

        cout << "ok, new size:" << endl;
        cout << "draw size: " << draw.size() << endl;
        cout << "white size: " << white.size() << endl;
        cout << "black size: " << black.size() << endl;

        cout << "Run tuning..." << flush;
        set < FEN * > fens;
        populateFens(white, fens, 1);
        cout << "." << flush;
        populateFens(black, fens, 0);
        cout << "." << flush;
        populateFens(draw, fens, 0.5);
        cout << "." << flush;
        draw.clear();
        white.clear();
        black.clear();

        tune(fens);

        cout << endl << endl << Time::getLocalTime() << " end\n";
        for (auto itr = fens.begin(); itr != fens.end(); itr++) delete *itr;
        fens.clear();
    }

private:
    string path;

    void populateFens(const set <string> &s, set<FEN *> &fens, const double win) {
        for (auto itr = s.begin(); itr != s.end(); itr++) {
            const string fen = *itr;
            FEN *x = new FEN(fen, win);
            fens.insert(x);
        }
    }

    bool inCheck(string fen) {
        const int side = searchManager.loadFen(fen);
        if (side == WHITE) return board::inCheck1<WHITE>(searchManager.getChessboard());
        else return board::inCheck1<BLACK>(searchManager.getChessboard());
    }

    void clean(set <string> &draw, set <string> &black, set <string> &white) {
        set<string>::iterator itr = draw.begin();
        set<string>::iterator tmp;

        for (; itr != draw.end();) {
            const auto check = inCheck(*itr);
            if (check || white.find(*itr) != white.end() || black.find(*itr) != black.end()) {
                white.erase(*itr);
                black.erase(*itr);
                tmp = itr;
                ++tmp;
                draw.erase(*itr);
                itr = tmp;
            } else itr++;
        }

        itr = white.begin();
        for (; itr != white.end();) {
            const auto check = inCheck(*itr);
            if (check || black.find(*itr) != black.end()) {
                black.erase(*itr);
                draw.erase(*itr);
                tmp = itr;
                ++tmp;
                white.erase(*itr);
                itr = tmp;
            } else itr++;
        }

        itr = black.begin();
        for (; itr != black.end();) {
            const auto check = inCheck(*itr);
            if (check) {
                white.erase(*itr);
                draw.erase(*itr);
                tmp = itr;
                ++tmp;
                black.erase(*itr);
                itr = tmp;
            } else itr++;
        }
    }

    void fetch(const string file, set <string> &fens) {
        if (!FileUtil::fileExists(file)) {
            cout << "Unable to open file " << file << endl;
            exit(1);
        }
        ifstream epdFile(file);
        string line;
        if (epdFile.is_open()) {
            while (getline(epdFile, line)) fens.insert(line);
            epdFile.close();
        }
    }

    double E(const set<FEN *> &fens) {
        constexpr double K = 1.13;
        double currentError = 0.0;
        for (auto itr = fens.begin(); itr != fens.end(); itr++) {
            const FEN *fen = *itr;
            searchManager.loadFen(fen->fen);
            searchManager.setRunning(2);
            searchManager.setRunningThread(true);
            searchManager.setMaxTimeMillsec(2500);
            const double score = searchManager.getQscore();
            if (abs(score) > _INFINITE - 1000) {
                cout << "skip mate score " << score << endl;
                continue;
            }
            const auto sigmoid = 1.0 / (1.0 + pow(10.0, -K * score / 400.0));
            const auto result = fen->win;
            currentError += pow(result - sigmoid, 2.0);
        }
        return 1.0 / fens.size() * currentError;
    }

};

#endif
