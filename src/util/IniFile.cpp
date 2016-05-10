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

IniFile::IniFile(const string &fileName1) {
    fileName = fileName1;
    if (FileUtil::fileSize(fileName) <= 0)return;
    endFile = true;
    inData.open(fileName, std::ofstream::in);
    if (inData.is_open()) {
        endFile = false;
    } else {
        warn("file not found: ", fileName);
    }
    rgxLine.assign("^(.+?)=(.*)$");
    rgxTag.assign("^\\[.+]$");
}

IniFile::~IniFile() {
    if (endFile) {
        inData.close();
    }
}

string IniFile::getValue(const string &value) {
    IniFile file(fileName);
    while (true) {
        const pair<string, string> *parameters = file.get();
        if (!parameters)return "";
        if (parameters->first == value) {
            return parameters->second;
        }
    }
}

const pair<string, string> *IniFile::get() {
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
        if (line.at(0) == '#' || line.at(0) == ';')continue;

        const string line2 = line;
        if (std::regex_search(line2.begin(), line2.end(), match, rgxTag)) {
            params.first = line;
            params.second = "";
        } else if (std::regex_search(line2.begin(), line2.end(), match, rgxLine)) {
            params.first = String(match[1]).trim();
            if (!params.first.size())continue;
            params.second = match[2];
        } else {
            continue;
        }
        return &params;
    }

    return nullptr;
}