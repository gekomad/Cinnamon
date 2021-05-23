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
#include <cstring>

using namespace std;

class String {
private:
    String() {}

public:

//    template<class T>
//    static String(T d, const string tohex = "") {
//        stringstream ss;
//        if (tohex == "int64tohex") {
//            stringstream ss2;
//            ss2 << std::hex << d;
//            ss << "0x";
//            for (unsigned i = 0; i < 16 - ss2.str().length(); i++)
//                ss << "0";
//            ss << std::hex << d << "ULL";
//        } else if (tohex == "int32tohex") {
//            stringstream ss2;
//            ss2 << std::hex << d;
//            ss << "0x";
//            for (unsigned i = 0; i < 8 - ss2.str().length(); i++)
//                ss << "0";
//            ss << std::hex << d;
//        } else {
//            ss << d;
//        }
//        value = ss.str();
//    }

    static bool isNumber(const string &value) {
        for(const char& c : value) {
            if(!isdigit(c))return false;
        }
        return true;
    }

    static const string trim(string &value) {
        string value1 = trimLeft(value);
        return trimRight(value1);
    }

    static const string trimLeft(string &value) {
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t start = value.find_first_not_of(WHITESPACE);
        if (start == std::string::npos) {
            return "";
        } else {
            value = value.substr(start);
            return value;
        }
    }

    static const string trimRight(string &value) {
        const std::string WHITESPACE = " \n\r\t\f\v";
        size_t end = value.find_last_not_of(WHITESPACE);
        if (end == std::string::npos) return "";
        else {
            value = value.substr(0, end + 1);
            return value;
        }
    }

    static const string replace(string &value, const char c1, const char c2) {
        for (unsigned i = 0; i < value.size(); i++) {
            if (value.at(i) == c1) {
                value.at(i) = c2;
            }
        }
        return value;
    }

    static const string replace(string &v, const string &s1, const string &s2) {
        unsigned long a;

        while ((a = v.find(s1)) != string::npos) {
            v.string::replace(a, s1.size(), s2);
        }
        return v;
    }

    static const string toUpper(string &v) {
        transform(v.begin(), v.end(), v.begin(), ::toupper);
        return v;
    }

    static const string toLower(string &v) {
        transform(v.begin(), v.end(), v.begin(), ::tolower);
        return v;
    }

};
