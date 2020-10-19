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
#include "../String.h"
#include "Time.h"
#include <map>
#include <iomanip>

using namespace std;

class Times {
public:
    static Times &getInstance() {
        static Times i;
        return i;
    }

private:
    int latency;

    void calcLatency() {
        start("test");
        stop("test");
    }

    Times() {
        for (int i = 0; i < 100000; i++)calcLatency();
        auto a = avgAndReset("test");
        latency = a.first;
        times.clear();
    }

    map<string, Time *> times;

    inline void add(const string &name) {
        if (nullptr == times[name]) times[name] = new Time(latency);
    }

    ~Times() {
        for (auto it = times.begin(); it != times.end(); ++it) {
            delete it->second;
        }
    }

public:

    void print() {
        for (auto it = times.begin(); it != times.end(); ++it) {
            const auto count1 = it->second->getCount();
            long countP = count1;
            const auto avg = avgAndReset(it->first);
            string m = " ";
            if (count1 > (1000 * 1000)) {
                countP /= (1000 * 1000);
                m = "M";
            } else if (count1 > 1000) {
                countP /= 1000;
                m = "K";
            }
            cout << "info string bench " << it->first << setw(30 - it->first.length())
                 << countP << m << " times\t" << "avg ns:" << "\t"
                 << avg.second << flush;
            if (avg.first != avg.second)
                cout << (avg.second > 1000 ? "\t" : "\t\t") << "without subprocess: " << avg.first << flush;
            int64_t a = count1 * avg.second;
            if (a > (1000 * 1000)) {
                a /= (1000 * 1000);
                m = "M";
            } else if (a > 1000) {
                a /= 1000;
                m = "K";
            }
            if (avg.first != avg.second)
            cout << "\t\tTOT: " << a << m << flush;
            else
            cout << "\t\t\t\t\t\t\t\t\tTOT: " << a << m << flush;
            cout << endl;
        }
    }

    inline void subProcess(const string &name, const string &subName) {
        times[name]->incCount(subName);
    }

    inline void start(const string &name) {
        add(name);
        times[name]->start();
    }

    inline void stop(const string &name) {
        Time *a = times[name];
        if (a == nullptr)return;
        a->stop();
    }

    pair<int64_t, int64_t> avgAndReset(const string &name) {
        Time *a = times[name];
        if (a == nullptr)return pair<int64_t, int64_t>(-1, -1);
        return a->avgWithSubProcessAndReset(times);
    }
};
