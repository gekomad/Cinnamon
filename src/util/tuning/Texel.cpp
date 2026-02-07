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

#include "Texel.h"


    Texel::Texel(const string &path) {
        load(path);
        const string &iniFile = "texel.ini";
        SearchManager::setMaxTimeMillsec(2500);
        cout.precision(17);
        const auto params= search.eval.PARAMS;
        loadParams(path+"/"+iniFile);
        bool fullImproved;
        do {
            cout << "***************************************** Texel's method Cycle #" << cycle++ << " " << Time::getLocalTime()
                 << " *****************************************" << endl;
            fullImproved = false;
            const double startError = E();
            printParams(cycle, 0,startError,startError, params);
            cout << "\nstartError: " << startError << endl;
            double bestError = startError;
            for (auto &param: params) {
                int bestValue = -1;
                for (int dir = 0; dir < 2; dir++) {
                    if (!dir)cout << "\nUP "; else cout << "\nDOWN ";
                    cout << Time::getLocalTime() << endl;

                    const auto oldValue = *param.second.ref;
                    int newValue;
                    if (dir == 0) newValue = oldValue + 1;
                    else {
                        if (oldValue <= 0)continue;
                        newValue = oldValue - 1;
                    }
                    *param.second.ref= newValue;

                    int eq = 0;
                    while (true) {
                        const double currentError = E();
                        cout << param.first << " try value: " << newValue << "\terror: " << currentError
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
                            *param.second.ref= newValue;
                        } else break;
                    }
                    *param.second.ref=  oldValue;
                    if (bestValue >= 0) {
                        cout << "\n** Improved. ** bestError: " << bestError << " bestValue " << bestValue << " was "
                             << param.second.startValue << flush;
                        *param.second.ref= bestValue;
                        printParams(cycle, 0, bestValue, bestValue,params);
                        saveParams(iniFile, params);
                        assert(E(fens) == bestError);
                    } else cout << "\n** Not improved. **" << flush;
                }
            }
        } while (fullImproved);
        printParams(cycle, 0, 0, 0,params);
        saveParams(iniFile, params);
        cout << endl << endl << Time::getLocalTime() << " end\n";
        for (auto itr = fens.begin(); itr != fens.end(); ++itr) delete *itr;
        fens.clear();
    }


#endif
