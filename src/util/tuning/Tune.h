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
#ifdef TUNING

#include <set>
#include <map>
#include "../Random.h"
#include "../../Eval.h"
#include "../../SearchManager.h"
using namespace std;
class Tune {

public:
    ~Tune();

    static constexpr auto help = " Download a big strong main.pgn file\n"
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


      struct FEN {
        string fen;
        double win; // 1=WHITE, 0=BLACK, 0.5=DRAW
        int score;

        FEN(const string& f, int w) : fen(f), win(w) {}

        FEN(const string& f, const int w, const int sc) : fen(f), win(w), score(sc) {}
    };
protected:
    static set<FEN*> fens;
    Search search;
    volatile bool stopFlag = false;
    double E() ;
    void load(const string &path) ;
    static void printParams(const int cycle, const int id, const double currentError, const double bestError, const map<string, Eval::PARAM>& p){
        _CoutSyncSpinlock.lock();
        printf("\n\n ******** Thread #%d cycle: %d ", id, cycle );
        cout << Time::getLocalTime() << " ***********";
        for (auto &param: p) {
            printf("\nname: %s, startValue: %d, newValue: %d", param.first.c_str(), param.second.startValue,  *param.second.ref);
            if (param.second.startValue != *param.second.ref)cout << " (*)";
        }
        cout <<endl;
        printf("currentError: %.17f bestError: %.17f\n",currentError, bestError);
        _CoutSyncSpinlock.unlock();
    }

    static double randomPercent(const double val, const int perc) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        const double var = val * perc / 100.0;
        const double min = val - var;
        const double max = val + var;
        std::uniform_real_distribution<> dist(min, max);
        const auto a= dist(gen);
        return a;
    }
    int cycle = 0;
    void loadParams(const string &iniFile);
    void saveParams(const string &iniFile, const  std::map<string, Eval::PARAM>& params) const ;
    static Spinlock _CoutSyncSpinlock;

    static Spinlock _ShareParameterSpinlock;
private:

    bool inCheck(const string& fen) ;
    void clean(set <string> &draw, set <string> &black, set <string> &white) ;
    static void populateFens(const set <string> &s, set<FEN *> &fens, const double win) {
        for (auto itr = s.begin(); itr != s.end(); ++itr) {
            const string fen = *itr;
            auto x = new FEN(fen, win);
            fens.insert(x);
        }
    }

    static void fetch(const string& file, set <string> &fens) {
        if (!FileUtil::fileExists(file)) {
            cout << "Unable to open file " << file << endl;
            exit(1);
        }
        ifstream epdFile(file);
        if (epdFile.is_open()) {
            string line;
            while (getline(epdFile, line)) fens.insert(line);
            epdFile.close();
        }
    }

};

#endif
