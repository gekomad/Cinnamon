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

#include <fstream>
#include <algorithm>
#include <regex>

using namespace std;

class IniFile {
public:

    IniFile(const string &fileName) {
        endFile = true;
        inData.open(fileName);
        if (inData.is_open()) {
            endFile = false;
        }
    }

    ~IniFile() {
        if (endFile) {
            inData.close();
        }
    }

    pair<string, string> *get() {

        std::regex rgx("^(\\w*)=(.*)$");
        std::smatch match;
        string line;

        while (!endFile) {
            if (inData.eof()) {
                endFile = true;
                return nullptr;
            }
            getline(inData, line);
            if (!line.size)continue;
            if (line.at(0) == '#')continue;

            const string line2 = line;
            if (std::regex_search(line2.begin(), line2.end(), match, rgx)) {
                params.first = match[1];
                params.second = match[2];
            }
            return &params;
        }

        return nullptr;
    };

private:

    bool endFile = true;
    ifstream inData;
    pair<string, string> params;
};

