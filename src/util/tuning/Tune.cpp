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

#include <set>
#include "../Random.h"
#include "Tune.h"

Spinlock Tune::_CoutSyncSpinlock;

Spinlock Tune::_ShareParameterSpinlock;
set<Tune::FEN*> Tune::fens;
Tune::~Tune(){
   for (auto itr = fens.begin(); itr != fens.end(); ++itr)
            delete *itr;
}

 double Tune::E() {
        constexpr double K = 1.13;
        double currentError = 0.0;
        for (auto itr = fens.begin(); itr != fens.end(); ++itr) {
            if (stopFlag)
                return INT_MAX;
            const FEN *fen = *itr;
            search.loadFen(fen->fen);
            search.setRunning(2);
            search.setRunningThread(true);
            search.setMaxTimeMillsec(2500);
            const double score = search.qSearch(15, -_INFINITE, _INFINITE);
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

    void Tune::load(const string &path) {
        cout << "Tuning " << Time::getLocalTime() << " start" << endl;
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

        populateFens(white, fens, 1);
        cout << "." << flush;
        populateFens(black, fens, 0);
        cout << "." << flush;
        populateFens(draw, fens, 0.5);
        cout << "." << flush;
        draw.clear();
        white.clear();
        black.clear();
    }

    void Tune::saveParams(const string &iniFile, const  std::map<string, Eval::PARAM>& params) const {
        cout << endl << Time::getLocalTime() << " save parameters to " << iniFile << endl;
        ofstream myfile;
        myfile.open(iniFile);
        myfile << "#" << Time::getLocalTime() << endl;
        myfile << "cycle" << "=" << cycle << endl;
        for (auto &param:params) {
            myfile << param.first << "=" << *param.second.ref << endl;
        }
        myfile.close();
    }

    void Tune::loadParams(const string &iniFile) {
        cout << "\nload parameters from " << iniFile << endl;
        map<string, string> map = IniFile(iniFile).paramMap;
        for (auto it = map.begin(); it != map.end(); ++it) {
            std::cout << it->first << " => " << it->second << endl;
            if (it->first =="cycle") cycle = stoi(it->second); else
                *search.eval.PARAMS[it->first].ref = stoi(it->second); 
        }
    }

    bool Tune::inCheck(const string& fen) {
        const int side = search.loadFen(fen);
        if (side == WHITE) return board::inCheck1<WHITE>(search.chessboard);
        return board::inCheck1<BLACK>(search.chessboard);
    }

    void Tune::clean(set <string> &draw, set <string> &black, set <string> &white) {
        auto itr = draw.begin();
        set<string>::iterator tmp;

        while (itr != draw.end()) {
            const auto check = inCheck(*itr);
            if (check || white.find(*itr) != white.end() || black.find(*itr) != black.end()) {
                white.erase(*itr);
                black.erase(*itr);
                tmp = itr;
                ++tmp;
                draw.erase(*itr);
                itr = tmp;
            } else ++itr;
        }

        itr = white.begin();
        while (itr != white.end()) {
            const auto check = inCheck(*itr);
            if (check || black.find(*itr) != black.end()) {
                black.erase(*itr);
                draw.erase(*itr);
                tmp = itr;
                ++tmp;
                white.erase(*itr);
                itr = tmp;
            } else ++itr;
        }

        itr = black.begin();
        while (itr != black.end()) {
            const auto check = inCheck(*itr);
            if (check) {
                white.erase(*itr);
                draw.erase(*itr);
                tmp = itr;
                ++tmp;
                black.erase(*itr);
                itr = tmp;
            } else ++itr;
        }
    }

 ;

#endif
