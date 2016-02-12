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

#include "String.h"

bool String::endsWith(const string &ending) {
    if (ending.size() > this->size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), this->rbegin());
}

String &String::trim() {
    trimLeft();
    trimRight();
    return *this;
}

String &String::trimLeft() {
    this->erase(this->begin(), std::find_if(this->begin(), this->end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return *this;
}

String &String::trimRight() {
    this->erase(std::find_if(this->rbegin(), this->rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), this->end());
    return *this;
}

String &String::replace(const char c1, const char c2) {
    for (unsigned i = 0; i < size(); i++) {
        if (at(i) == c1) {
            at(i) = c2;
        }
    }
    return *this;
}

String &String::replace(const string &s1, const string &s2) {
    unsigned long a;
    while ((a = find(s1)) != string::npos) {
        string::replace(a, s1.size(), s2);
    }
    return *this;
}

String &String::toUpper() {
    transform(begin(), end(), begin(), ::toupper);
    return *this;
}

String &String::toLower() {
    transform(begin(), end(), begin(), ::tolower);
    return *this;
}