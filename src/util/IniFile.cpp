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

#include "IniFile.h"
#include "String.h"

IniFile::IniFile(const string &fileName) {
    endFile = true;
    inData.open(fileName);
    if (inData.is_open()) {
        endFile = false;
    }
}

IniFile::~IniFile() {
    if (endFile) {
        inData.close();
    }
}

pair<string, string> *IniFile::get() {
    string svalue, line2;
    string param;
    while (!endFile) {
        if (inData.eof()) {
            endFile = true;
            return nullptr;
        }
        getline(inData, line2);
        if (line2.size() == 0)continue;
        String line(line2);
        line.replace('\t', ' ');
        line.replace('=', ' ');
        line.trim();
        if (line.at(0) == '#')continue;
        stringstream ss(line);
        ss >> param;
        ss >> svalue;
        params.first = param;
        params.second = svalue;
        return &params;
    }
    return nullptr;
}
