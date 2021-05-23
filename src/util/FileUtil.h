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
#include "../namespaces/String.h"
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

class FileUtil {
public:
    static bool fileExists(const string &filename) {
        struct stat info;
        return stat(filename.c_str(), &info) == 0;
    }

    static int fileSize(const string &filename) {
        std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
        return in.tellg();
    }

    static string getFileName(const string &path) {
        string pp = path;
        auto p = String::replace(pp, '\\', '/');
        istringstream iss(p);
        string token;
        while (getline(iss, token, '/'));
        return token;
    }
};

