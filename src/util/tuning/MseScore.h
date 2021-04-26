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

class MseScore : Tune { //TODO

public:
    static constexpr auto help = "--- Step 1 - create oracle_score.epd ---\n"
                                 "  mkdir /tmp/ramdisk; chmod 777 /tmp/ramdisk\n"
                                 "  sudo mount -t tmpfs -o size=100m tmpfs /tmp/ramdisk/\n"
                                 "  cp crafty /tmp/ramdisk\n"
                                 "  cd /tmp/ramdisk\n"
                                 "  ./cinnamon step1 huge_epd.epd oracle_epd.epd /tmp/ramdisk/\n"
                                 "\n--- Step 2 - tuning through Crafty ---\n"
                                 "./cinnamon step2 oracle_epd.epd\n";

    MseScore(const string &huge_epd_in, const string &oracle_epd_out, const string &ramdisk) {
        cout << Time::getLocalTime() << " Start" << endl;
        writeOracleScore(huge_epd_in, oracle_epd_out, ramdisk);
        cout << Time::getLocalTime() << " End" << endl;
    }

    MseScore(const string &oracle_score_in) {
        cout << Time::getLocalTime() << " Start" << endl;
        cout << "Fetch epd files " << oracle_score_in << "..." << endl << flush;
        set<FEN *> fens;
        fetch(oracle_score_in, fens);
        tune(fens);
        cout << endl << endl << Time::getLocalTime() << " End\n";
        for (auto itr = fens.begin(); itr != fens.end(); itr++) delete *itr;
        fens.clear();
    }

private:

    void fetch(const string file, set<FEN *> &fens) {
        if (!FileUtil::fileExists(file)) {
            cout << "Unable to open file " << file << endl;
            exit(1);
        }

        ifstream epdFile(file);
        string line;

        std::size_t offset = 0;
        int c = 0;
        if (epdFile.is_open()) {
            while (getline(epdFile, line)) {
                int pos = line.find(";");
                string fen = line.substr(0, pos - 1);
                string score = line.substr(pos + 1, line.size());

                if (searchManager.loadFen(fen) == 2) {
                    cout << "error fen format" << (c++) << "  |||" << fen << "||||||" << score << "|||" << endl;
                    continue;
                }
                fens.insert(new FEN(fen, 0, stoi(score, &offset)));
            }
            epdFile.close();
        }

    }

    void writeOracleScore(const string &huge_epd_in, const string &oracle_epd_out, const string &ramdisk) {
        cout <<"\nwriteOracleScore\n";
        if (!FileUtil::fileExists(huge_epd_in)) {
            cout << "Unable to open file " << huge_epd_in << endl;
            exit(1);
        }
        int c = 1;
        ofstream oracle_score;
        oracle_score.open(oracle_epd_out);
        ifstream epdFile(huge_epd_in);
        string line;
        if (epdFile.is_open()) {
            while (getline(epdFile, line)) {
                if (!((c++) % 100)) cout << "calculateOracleScore " << c << endl;
                oracle_score << line << ";" << getCraftyScore(line, ramdisk) << endl;
            }
            epdFile.close();
        }else {cout <<"\nnot found";}
        oracle_score.close();
    }

    int getCraftyScore(const string &fen, const string &ramdisk) {

        ofstream myfile;
        myfile.open(ramdisk + "/score_crafty");
        myfile << "\nponder off";
        myfile << "\nsetboard " << fen;
        myfile << "\nscore";
        myfile << "\nquit";
        myfile << "\n";
        myfile.close();

        string x = ramdisk + "/crafty <" + ramdisk + "/score_crafty >" + ramdisk + "/out";
        system(x.c_str());
        int score = 0;
        ifstream read(ramdisk + "/out");
        string line;
        std::size_t offset = 0;
        if (read.is_open()) {
            while (std::getline(read, line)) {
                if (line.rfind("total...", 0) == 0) {
                    const string x = line.substr(15, 9);
                    score = std::stod(x, &offset) * 100;
                    //cout << line << "|" << x<<"|"<<score << endl;
                    break;
                }
            }
            read.close();
        } else {
            cout << "\nerr";
            exit(1);
        }
        return score;
    }


    double E(const set<FEN *> &fens) {

        double currentError = 0.0;

        for (auto itr = fens.begin(); itr != fens.end(); itr++) {

            const FEN *fen = *itr;
            searchManager.loadFen(fen->fen);

            const int score = searchManager.getScore(WHITE);
            const int Oscore = fen->score;
            //cout << score << " " << Oscore << " " << fen->fen << endl;
            if (abs(score) > _INFINITE - 1000) {
                cout << "skip mate score " << score << endl;
                continue;
            }
            currentError += pow(score - Oscore, 2);
        }
        return currentError / fens.size();
    }

};

#endif
