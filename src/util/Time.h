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

#include <chrono>
#include <iostream>
#include "String.h"

using namespace std;
using namespace chrono;

class Time {
public:
    static const int HOUR_IN_SECONDS = 60 * 60;
    static const int HOUR_IN_MINUTES = 60;


    static int diffTime(high_resolution_clock::time_point t1, high_resolution_clock::time_point t2) {
        std::chrono::duration<double, std::milli> elapsed = t1 - t2;
        return elapsed.count();
    }

    static string getLocalTime() {
        time_t current = chrono::system_clock::to_time_t(chrono::system_clock::now());
        String gg(ctime(&current));
        return gg.trimRight();
    }

    static int getYear() {
        time_t t = time(NULL);
        tm *timePtr = localtime(&t);
        return 1900 + timePtr->tm_year;
    }

    static int getMonth() {
        time_t t = time(NULL);
        tm *timePtr = localtime(&t);
        return 1 + timePtr->tm_mon;
    }

    static int getDay() {
        time_t t = time(NULL);
        tm *timePtr = localtime(&t);
        return timePtr->tm_mday;
    }
};
