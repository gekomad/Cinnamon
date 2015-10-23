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

#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <string.h>

using namespace std;

class String : public string {
public:
    String();

    String(const string &s) : string(s) { };

    static string toString(const __int128_t value) {
        __uint128_t tmp = value < 0 ? -value : value;
        char buffer[128];
        int p = 0;
        char *d = std::end(buffer);
        do {
            p++;
            --d;
            *d = "0123456789"[tmp % 10];
            tmp /= 10;
        } while (tmp != 0);
        if (value < 0) {
            p++;
            --d;
            *d = '-';
        }
        d[p] = 0;
        return d;
    }

    String(const char *s) : string(s) { };

    template<class T>
    String(T d) {
        stringstream ss;
        ss << d;
        assign(ss.str());
    }

    static int stoi(const string &s) {
        if (s.size() == 0)return 0;
        return std::stoi(s);
    }

    String &trim();

    String &trimLeft();

    String &trimRight();

    String &replace(const string &s1, const string &s2);

    String &replace(char c1, char c2);

    String &toUpper();

    String &toLower();
};
