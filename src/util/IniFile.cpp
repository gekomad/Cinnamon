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
#include "IniFile.h"

IniFile::IniFile(const string &fileName1) {
    fileName = fileName1;
    inData.open(fileName, std::ofstream::in);
    if (!inData.is_open()) {
        cout << "file not found " << fileName << endl;
        return;
    }

    rgxLine.assign("^(.+?)=(.*)$");
    rgxTag.assign("^\\[.+]$");
    while (true) {
        pair<string, string> *parameters = this->get();
        if (!parameters)break;
        paramMap[parameters->first] = parameters->second;
    }
    inData.close();
}

pair<string, string> *IniFile::get() {
    std::smatch match;
    string line;
    while (!inData.eof()) {
        getline(inData, line);
        trace(line);
        if (line.empty())continue;
        if (line.at(0) == '#' || line.at(0) == ';')continue;

        const string line2 = line;
        if (std::regex_search(line2.begin(), line2.end(), match, rgxTag)) {
            params.first = line;
            params.second = "";
        } else if (std::regex_search(line2.begin(), line2.end(), match, rgxLine)) {
            string x = string(match[1]);
            params.first = String::trim(x);
            if (params.first.empty())continue;
            params.second = match[2];
        } else {
            continue;
        }
        return &params;
    }

    return nullptr;
}
#endif
