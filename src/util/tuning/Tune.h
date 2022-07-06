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
    constexpr static int N_PARAM = 43;
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
        int phase;

        PARAMS(string p, const SearchManager &searchManager, const int ph) : name(p), phase(ph) {
            startValue = searchManager.getParameter(p, ph);
        }

        void print(SearchManager &searchManager) const {
            printf("\nname: %s(%s), startValue: %d, newValue: %d", name.c_str(), phase ? "EG" : "MG", startValue,
                   searchManager.getParameter(name, phase));
            if (startValue != searchManager.getParameter(name, phase))cout << " (*)";
        }
    };

    void saveParams(const array<PARAMS, N_PARAM> &params) {
        cout << endl << Time::getLocalTime() << " save parameters to " << iniFile << endl;
        ofstream myfile;
        myfile.open(iniFile);
        myfile << "#" << Time::getLocalTime() << endl;
        for (auto &param: params) {
            myfile << param.name << "=" << searchManager.getParameter(param.name, param.phase) << "|" << param.phase
                   << endl;
        }
        myfile.close();
    }

    void loadParams() {
        cout << "\nload parameters from " << iniFile << endl;
        map<string, string> map = IniFile(iniFile).paramMap;
        for (std::map<string, string>::iterator it = map.begin(); it != map.end(); ++it) {
            int pos = it->second.find("|");
            const auto value = stoi(it->second.substr(0, pos));
            const auto phase = stoi(it->second.substr(pos + 1, it->second.length()));
            cout << it->first << "(" << (phase ? "EG" : "MG") << ") => " << value << endl;
            searchManager.setParameter(it->first, value, phase);
        }
    }

    void tune(const set<FEN *> &fens) {
        searchManager.setMaxTimeMillsec(2500);
        cout.precision(17);

        loadParams();
        const array<PARAMS, N_PARAM> params{
                PARAMS("MOB_KING_", searchManager, Eval::MG),
                PARAMS("MOB_KNIGHT_", searchManager, Eval::MG),
                PARAMS("MOB_BISHOP_", searchManager, Eval::MG),
                PARAMS("MOB_QUEEN_", searchManager, Eval::MG),
//                PARAMS("ATTACK_KING", searchManager, Eval::MG),
//                PARAMS("BISHOP_ON_QUEEN", searchManager, Eval::MG),
                PARAMS("BACKWARD_PAWN", searchManager, Eval::MG),
                PARAMS("DOUBLED_ISOLATED_PAWNS", searchManager, Eval::MG),
                PARAMS("PAWN_IN_7TH", searchManager, Eval::MG),
                PARAMS("PAWN_IN_PROMOTION", searchManager, Eval::MG),
                PARAMS("PAWN_NEAR_KING", searchManager, Eval::MG),
                PARAMS("PAWN_BLOCKED", searchManager, Eval::MG),
                PARAMS("UNPROTECTED_PAWNS", searchManager, Eval::MG),
                PARAMS("FRIEND_NEAR_KING", searchManager, Eval::MG),
                PARAMS("BONUS2BISHOP", searchManager, Eval::MG),
                PARAMS("BISHOP_PAWN_ON_SAME_COLOR", searchManager, Eval::MG),
                PARAMS("OPEN_FILE_Q", searchManager, Eval::MG),
                PARAMS("ROOK_7TH_RANK", searchManager, Eval::MG),
                PARAMS("KNIGHT_PINNED", searchManager, Eval::MG),
                PARAMS("ROOK_PINNED", searchManager, Eval::MG),
                PARAMS("BISHOP_PINNED", searchManager, Eval::MG),
                PARAMS("ROOK_IN_7_KING_IN_8", searchManager, Eval::MG),
                PARAMS("PAWN_PASSED_", searchManager, Eval::MG),
                PARAMS("DISTANCE_KING_ENDING_", searchManager, Eval::MG),
                PARAMS("DISTANCE_KING_OPENING_", searchManager, Eval::MG),
//                PARAMS("QUEEN_PINNED", searchManager, Eval::MG),
//                PARAMS("ROOK_IN_7", searchManager, Eval::MG),
//                PARAMS("QUEEN_IN_7", searchManager, Eval::MG),

                PARAMS("MOB_KING_", searchManager, Eval::EG),
                PARAMS("MOB_KNIGHT_", searchManager, Eval::EG),
                PARAMS("MOB_BISHOP_", searchManager, Eval::EG),
                PARAMS("MOB_QUEEN_", searchManager, Eval::EG),
//                PARAMS("ATTACK_KING", searchManager, Eval::EG),
//                PARAMS("BISHOP_ON_QUEEN", searchManager, Eval::EG),
                PARAMS("BACKWARD_PAWN", searchManager, Eval::EG),
                PARAMS("DOUBLED_ISOLATED_PAWNS", searchManager, Eval::EG),
                PARAMS("PAWN_IN_7TH", searchManager, Eval::EG),
                PARAMS("PAWN_IN_PROMOTION", searchManager, Eval::EG),
                PARAMS("PAWN_NEAR_KING", searchManager, Eval::EG),
                PARAMS("PAWN_BLOCKED", searchManager, Eval::EG),
                PARAMS("UNPROTECTED_PAWNS", searchManager, Eval::EG),
                PARAMS("FRIEND_NEAR_KING", searchManager, Eval::EG),
                PARAMS("BONUS2BISHOP", searchManager, Eval::EG),
                PARAMS("BISHOP_PAWN_ON_SAME_COLOR", searchManager, Eval::EG),
                PARAMS("OPEN_FILE_Q", searchManager, Eval::EG),
                PARAMS("ROOK_7TH_RANK", searchManager, Eval::EG),
                PARAMS("KNIGHT_PINNED", searchManager, Eval::EG),
                PARAMS("ROOK_PINNED", searchManager, Eval::EG),
                PARAMS("BISHOP_PINNED", searchManager, Eval::EG),
                PARAMS("ROOK_IN_7_KING_IN_8", searchManager, Eval::EG)
//              PARAMS("QUEEN_PINNED", searchManager, Eval::EG),
//              PARAMS("ROOK_IN_7", searchManager, Eval::EG),
//              PARAMS("QUEEN_IN_7", searchManager, Eval::EG)
        };
        bool fullImproved;
        int cycle = 1;
        double bestError;
        do {
            cout << "***************************************** Cycle #" << (cycle++) << " " << Time::getLocalTime()
                 << " *****************************************" << endl << flush;
            fullImproved = false;
            const double startError = E(fens);
            for (auto &param: params) param.print(searchManager);
            cout << "\nstartError: " << startError << endl << flush;
            bestError = startError;
            for (auto &param: params) {
                int bestValue = INT_MAX;
                for (int dir = 0; dir < 2; dir++) {
                    if (!dir)cout << "\nUP "; else cout << "\nDOWN ";
                    cout << Time::getLocalTime() << endl << flush;

                    auto oldValue = searchManager.getParameter(param.name, param.phase);
                    int newValue;
                    if (dir == 0) newValue = searchManager.getParameter(param.name, param.phase) + 1;
                    else {
                        if (searchManager.getParameter(param.name, param.phase) <= 0)continue;
                        else newValue = searchManager.getParameter(param.name, param.phase) - 1;
                    }
                    searchManager.setParameter(param.name, newValue, param.phase);

                    double currentError;

                    int eq = 0;
                    while (true) {
                        currentError = E(fens);
                        cout << param.name << "(" << (param.phase ? "EG" : "MG") << ") try value: " << newValue
                             << "\terror: "
                             << currentError
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
                            if (dir == 0) newValue++;
                            else {
                                if (startError >= 0) { if (newValue <= 0)break; }
                                else if (newValue == 0)break; else newValue--;
                            }
                            searchManager.setParameter(param.name, newValue, param.phase);
                        } else break;
                    }
                    searchManager.setParameter(param.name, oldValue, param.phase);
                    if (bestValue != INT_MAX) {
                        cout << "\n** Improved. ** bestError: " << bestError << " bestValue " << bestValue << " was "
                             << param.getStartValue() << flush;
                        searchManager.setParameter(param.name, bestValue, param.phase);
                        for (auto &param: params) param.print(searchManager);
                        saveParams(params);
                        assert(E(fens) == bestError);
                    } else cout << "\n** Not improved. **" << flush;
                }
            }
        } while (fullImproved);
        for (auto &param: params) param.print(searchManager);
        saveParams(params);
    }
};

#endif
