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
#include "../namespaces/debug.h"

using namespace std;
using namespace _debug;

class IniFile {
public:

    IniFile(const string &fileName) {
        endFile = true;
        inData.open(fileName);
        if (inData.is_open()) {
            endFile = false;
        } else {
            warn("file not found: ", fileName);
        }
        rgxLine.assign("^(\\w*)=(.*)$");
        rgxTag.assign("^\\[.+]$");
    }

    ~IniFile() {
        if (endFile) {
            inData.close();
        }
    }

    string getValue(const string &value) {
        while (true) {
            pair<string, string> *parameters = get();
            if (!parameters)return "";
            if (parameters->first == value) {
                return parameters->second;
            }
        }
    }

    pair<string, string> *get() {

        std::smatch match;
        string line;

        while (!endFile) {
            if (inData.eof()) {
                endFile = true;
                return nullptr;
            }
            getline(inData, line);
            trace(line);
            if (!line.size())continue;
            if (line.at(0) == '#')continue;

            const string line2 = line;
            if (std::regex_search(line2.begin(), line2.end(), match, rgxTag)) {
                params.first = line;
                params.second = "";
            } else if (std::regex_search(line2.begin(), line2.end(), match, rgxLine)) {
                params.first = match[1];
                params.second = match[2];
            }
            return &params;
        }

        return nullptr;
    };

private:
    std::regex rgxLine;
    std::regex rgxTag;
    bool endFile = true;
    ifstream inData;
    pair<string, string> params;
};

