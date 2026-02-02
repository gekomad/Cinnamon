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
#include "../Random.h"

class Tune {

protected:

    SearchManager &searchManager = Singleton<SearchManager>::getInstance();

    int cycle = 1;
    struct FEN {
        string fen;
        double win; // 1=WHITE, 0=BLACK, 0.5=DRAW
        int score;

        FEN(string f, int w) : fen(f), win(w) {}

        FEN(string f, int w, int sc) : fen(f), win(w), score(sc) {}
    };

    virtual double E(const set<FEN *> &fens) = 0;

    void saveParams(const string &iniFile, const std::array<Eval::PARAM, N_PARAMS>  params) {
        cout << endl << Time::getLocalTime() << " save parameters to " << iniFile << endl;
        ofstream myfile;
        myfile.open(iniFile);
        myfile << "#" << Time::getLocalTime() << endl;
        myfile << "cycle" << "=" << cycle << endl;
        for (auto &param:params) {
            myfile << param.name << "=" << *param.ref << endl;
        }
        myfile.close();
    }

    void loadParams(const string &iniFile) {
        cout << "\nload parameters from " << iniFile << endl;
        map<string, string> map = IniFile(iniFile).paramMap;
        for (std::map<string, string>::iterator it = map.begin(); it != map.end(); ++it) {
            std::cout << it->first << " => " << it->second << endl;
            if (it->first =="cycle") cycle = stoi(it->second); else
            searchManager.setParameter(it->first, stoi(it->second));
        }
    }
    // void tuneSPSA(const set<FEN *> &fens) {
    //     double a = 1.0;  // Magnitudo dello spostamento (step size)
    //     double c = 1.0;  // Magnitudo della perturbazione

    //
    //     for (int i = 1; ; ++i) {
    //         cout << "***************************************** Cycle #" << (cycle++) << " " << Time::getLocalTime()
    //            << " *****************************************" << endl;
    //         vector<int> delta(N_PARAM);
    //         vector<int> originalValues(N_PARAM);
    //
    //         int j=0;
    //         // 1. Genera perturbazione casuale (+1 o -1)
    //         for (auto &param:params) {
    //         // for (int j = 0; j < N_PARAM; ++j) {
    //             delta[j] = Random::getRandomBool() ? 1 : -1;
    //             originalValues[j] = searchManager.getParameter(param.name);
    //             ++j;
    //         }
    //         j=0;
    //         // 2. Calcola Errore Positivo (Tutti i parametri + delta)
    //         for (auto &param:params)
    //             searchManager.setParameter(param.name, originalValues[j] + c * delta[j]);
    //         double E_plus = E(fens);
    //         j=0;
    //         // 3. Calcola Errore Negativo (Tutti i parametri - delta)
    //         for (auto &param:params) {
    //             searchManager.setParameter(param.name, originalValues[j] - c * delta[j]);
    //         double E_minus = E(fens);
    //
    //         // 4. Stima del gradiente e aggiornamento
    //         // Se E_plus < E_minus, scendiamo verso E_plus
    //         double g = (E_plus - E_minus) / (2.0 * c);
    //         j=0;
    //         for (auto &param:params){
    //             // Aggiornamento basato sul gradiente stimato
    //             int step = round(-a * g * delta[j]);
    //             int newValue = originalValues[j] + step;
    //             if (newValue < 0) newValue = 0; // Protezione valori negativi
    //             searchManager.setParameter(param.name, newValue);
    //         }
    //
    //         //if (i % 10 == 0)
    //             {
    //             printf("Iter %d | E_plus: %.10f | E_minus: %.10f | g: %.10f\n", i, E_plus, E_minus, g);
    //             for (auto &param:params) param.print(searchManager);
    //             saveParams(params);
    //         }
    //             double globalBestError = 1e10; // Un valore altissimo all'inizio
    //
    //             // Ogni 20 iterazioni controlla il "vero" progresso
    //             if (i % 20 == 0) {
    //                 double currentTrueError = E(fens); // Calcolato sui parametri attuali "stabili"
    //                 if (currentTrueError < globalBestError) {
    //                     cout << "[PROGRESSO] Errore migliorato: " << globalBestError << " -> " << currentTrueError << endl;
    //                     globalBestError = currentTrueError;
    //
    //                 } else {
    //                     cout << "[STALLO] L'errore attuale (" << currentTrueError << ") non supera il record." << endl;
    //                 }
    //             }
    //     }
    //  }
    // }
    void tune(const string &path, const string &iniFile, const set<FEN *> &fens) {
        searchManager.setMaxTimeMillsec(2500);
        cout.precision(17);
        auto params= searchManager.getParameters();
        loadParams(path+"/"+iniFile);

        bool fullImproved;

        double bestError;
        do {
            cout << "***************************************** Cycle #" << (cycle++) << " " << Time::getLocalTime()
                 << " *****************************************" << endl;
            fullImproved = false;
            const double startError = E(fens);
            for (auto &param: *params) param.print();
            cout << "\nstartError: " << startError << endl;
            bestError = startError;
            for (auto &param:*params) {
                int bestValue = -1;
                for (int dir = 0; dir < 2; dir++) {
                    if (!dir)cout << "\nUP "; else cout << "\nDOWN ";
                    cout << Time::getLocalTime() << endl;

                    auto oldValue = *param.ref;
                    int newValue;
                    if (dir == 0) newValue = oldValue + 1;
                    else {
                        if (oldValue <= 0)continue;
                        else newValue = oldValue - 1;
                    }
                    *param.ref= newValue;

                    double currentError;

                    int eq = 0;
                    while (true) {
                        currentError = E(fens);
                        cout << param.name << " try value: " << newValue << "\terror: " << currentError
                             << "\tbestError: " << bestError;
                        if (currentError < bestError)cout << "\t(improved)";
                        else if (currentError > bestError)cout << "\t(got worse)";
                        else cout << "\t(same)";
                        cout << endl;
                        if (currentError <= bestError && eq < 3) {
                            if (currentError == bestError) eq++; else eq = 0;
                            if (currentError < bestError) {
                                bestValue = newValue;
                                bestError = currentError;
                                fullImproved = true;
                            }
                            if (dir == 0) newValue++; else { if (newValue <= 0)break; else newValue--; }
                            *param.ref= newValue;
                        } else break;
                    }
                    *param.ref=  oldValue;
                    if (bestValue >= 0) {
                        cout << "\n** Improved. ** bestError: " << bestError << " bestValue " << bestValue << " was "
                             << param.startValue << flush;
                        *param.ref= bestValue;
                        for (auto &param:*params) param.print();
                        saveParams(iniFile, *params);
                        assert(E(fens) == bestError);
                    } else cout << "\n** Not improved. **" << flush;
                }
            }
        } while (fullImproved);
        for (auto &param:*params) param.print();
        saveParams(iniFile, *params);
    }
};

#endif
