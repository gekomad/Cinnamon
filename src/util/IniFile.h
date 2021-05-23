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

#include <fstream>
#include <regex>
#include "logger.h"
#include "FileUtil.h"
#include <map>

using namespace std;
using namespace _logger;

class IniFile {
public:

    IniFile(const string &fileName1);

    map<string, string> paramMap;
private:
    std::regex rgxLine;
    std::regex rgxTag;
    ifstream inData;
    string fileName;
    pair<string, string> params;

    pair<string, string> *get();

};
#endif
