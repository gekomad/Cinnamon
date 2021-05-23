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

#include "../../def.h"
#include <set>
#include <array>

class Tune {

protected:
    constexpr static int N_PARAM = 20;
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    const string iniFile = "tuning.ini";

    struct FEN {
        string fen;
        double win; // 1=WHITE, 0=BLACK, 0.5=DRAW
        int score;

        FEN(string f, int w) : fen(f), win(w) {}

        FEN(string f, int w, int sc) : fen(f), win(w), score(sc) {}
    };

    virtual double E(const set<FEN *> &fens) = 0;

    struct PARAMS {
    private :
        int startValue;
    public:
        int getStartValue() const { return startValue; }

        string name;

        PARAMS(string n, const SearchManager &searchManager) : name(n) { startValue = searchManager.getParameter(n); }

        void print(SearchManager &searchManager) const {
            printf("\nname: %s, startValue: %d, newValue: %d", name.c_str(), startValue,
                   searchManager.getParameter(name));
            if (startValue != searchManager.getParameter(name))cout << " (*)";
        }
    };

    void saveParams(const array<PARAMS, N_PARAM> &params) {
        cout << endl << Time::getLocalTime() << " save parameters to " << iniFile << endl;
        ofstream myfile;
        myfile.open(iniFile);
        myfile << "#" << Time::getLocalTime() << endl;
        for (auto &param:params) {
            myfile << param.name << "=" << searchManager.getParameter(param.name) << endl;
        }
        myfile.close();
    }

    void loadParams() {
        cout << "\nload parameters from " << iniFile << endl;
        map<string, string> map = IniFile(iniFile).paramMap;
        for (std::map<string, string>::iterator it = map.begin(); it != map.end(); ++it) {
            std::cout << it->first << " => " << it->second << endl;
            searchManager.setParameter(it->first, stoi(it->second));
        }
    }

    void tune(const set<FEN *> &fens) {
        searchManager.setMaxTimeMillsec(2500);
        cout.precision(17);

        loadParams();
        const array<PARAMS, N_PARAM> params{
                PARAMS("ATTACK_KING", searchManager),
                PARAMS("BISHOP_ON_QUEEN", searchManager),
                PARAMS("BACKWARD_PAWN", searchManager),
                PARAMS("DOUBLED_ISOLATED_PAWNS", searchManager),
                PARAMS("PAWN_IN_7TH", searchManager),
                PARAMS("PAWN_IN_PROMOTION", searchManager),
                PARAMS("PAWN_NEAR_KING", searchManager),
                PARAMS("PAWN_BLOCKED", searchManager),
                PARAMS("UNPROTECTED_PAWNS", searchManager),
                PARAMS("FRIEND_NEAR_KING", searchManager),
                PARAMS("BONUS2BISHOP", searchManager),
                PARAMS("BISHOP_PAWN_ON_SAME_COLOR", searchManager),
                PARAMS("OPEN_FILE_Q", searchManager),
                PARAMS("ROOK_7TH_RANK", searchManager),
                PARAMS("KNIGHT_PINNED", searchManager),
                PARAMS("ROOK_PINNED", searchManager),
                PARAMS("BISHOP_PINNED", searchManager),
                PARAMS("QUEEN_PINNED", searchManager),
                PARAMS("ROOK_IN_7", searchManager),
                PARAMS("QUEEN_IN_7", searchManager)
        };
        bool fullImproved;
        int cycle = 1;
        double bestError;
        do {
            cout << "***************************************** Cycle #" << (cycle++) << " " << Time::getLocalTime()
                 << " *****************************************" << endl << flush;
            fullImproved = false;
            const double startError = E(fens);
            for (auto &param:params) param.print(searchManager);
            cout << "\nstartError: " << startError << endl << flush;
            bestError = startError;
            for (auto &param:params) {
                int bestValue = -1;
                for (int dir = 0; dir < 2; dir++) {
                    if (!dir)cout << "\nUP "; else cout << "\nDOWN ";
                    cout << Time::getLocalTime() << endl << flush;

                    auto oldValue = searchManager.getParameter(param.name);
                    int newValue;
                    if (dir == 0) newValue = searchManager.getParameter(param.name) + 1;
                    else {
                        if (searchManager.getParameter(param.name) <= 0)continue;
                        else newValue = searchManager.getParameter(param.name) - 1;
                    }
                    searchManager.setParameter(param.name, newValue);

                    double currentError;

                    int eq = 0;
                    while (true) {
                        currentError = E(fens);
                        cout << param.name << " try value: " << newValue << "\terror: " << currentError
                             << "\tbestError: "
                             << bestError;
                        if (currentError < bestError)cout << "\t(improved)";
                        else if (currentError > bestError)cout << "\t(got worse)";
                        else cout << "\t(same)";
                        cout << endl << flush;
                        if (currentError <= bestError && eq < 3) {
                            if (currentError == bestError) eq++; else eq = 0;
                            if (currentError < bestError) {
                                bestValue = newValue;
                                bestError = currentError;
                                fullImproved = true;
                            }
                            if (dir == 0) newValue++; else { if (newValue <= 0)break; else newValue--; }
                            searchManager.setParameter(param.name, newValue);
                        } else break;
                    }
                    searchManager.setParameter(param.name, oldValue);
                    if (bestValue >= 0) {
                        cout << "\n** Improved. ** bestError: " << bestError << " bestValue " << bestValue << " was "
                             << param.getStartValue() << flush;
                        searchManager.setParameter(param.name, bestValue);
                        for (auto &param:params) param.print(searchManager);
                        saveParams(params);
                        assert(E(fens) == bestError);
                    } else cout << "\n** Not improved. **" << flush;
                }
            }
        } while (fullImproved);
        for (auto &param:params) param.print(searchManager);
        saveParams(params);
    }
};

#endif
