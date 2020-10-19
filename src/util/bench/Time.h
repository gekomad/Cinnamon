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
#include "../String.h"
#include <map>

using namespace std;
using namespace chrono;

class Time {

private:
    int _count = 0;
    std::chrono::time_point<std::chrono::system_clock> _start;
    int64_t _totTime = 0;
    map<string, int64_t> subName;
    int latency = 0;
public:

    Time() {}

    Time(int latency) {
        this->latency = latency;
    }

    static constexpr int HOUR_IN_SECONDS = 60 * 60;
    static constexpr int HOUR_IN_MINUTES = 60;

    int64_t getCount() const { return _count; }

    void resetAndStart() {
        reset();
        start();
    }

    inline void incCount(const string &subName) {
        this->subName[subName]++;
    }

    inline void start() {
        _count++;
        _start = std::chrono::system_clock::now();
    }

    inline void stop() {
        _totTime += std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now() - _start).count();
        _totTime -= latency;
    }

    int64_t avg() const {
        return _count == 0 ? 0 : _totTime / _count;
    }

    pair<int64_t, int64_t> avgWithSubProcessAndReset(const map<string, Time *> times1) {
        int64_t totTimeSubprocess = 0;
        for (auto & it : subName) {
            auto name1 = it.first;
            auto count1 = it.second;
            auto avg1 = times1.at(name1)->avg();
            totTimeSubprocess += avg1 * count1;
        }
        auto avgs = pair<int64_t, int64_t>(0, 0);
        if (_count)
            avgs = pair<int64_t, int64_t>((_totTime - totTimeSubprocess) / _count, _totTime / _count);
        reset();
        return avgs;
    }

    unsigned long getMill() const {
        return _totTime / 1000000;
    }

    void reset() {
        _totTime = _count = 0;
        for (auto & it : subName) {
            it.second = 0;
        }
    }

    static int diffTime(const high_resolution_clock::time_point t1, const high_resolution_clock::time_point t2) {
        const std::chrono::duration<double, std::milli> elapsed = t1 - t2;
        return elapsed.count();
    }

    static string
    diffTimeToString(const high_resolution_clock::time_point start, const high_resolution_clock::time_point stop) {
        string res;
        unsigned t = Time::diffTime(stop, start) / 1000;
        unsigned days = t / 60 / 60 / 24;
        int hours = (t / 60 / 60) % 24;
        int minutes = (t / 60) % 60;
        int seconds = t % 60;
        int millsec = Time::diffTime(stop, start) % 1000;

        if (days) {
            res.append(String(days)).append(" days ");
        }
        if (days || hours) {
            res.append(String(hours)).append(" hours ");
        }
        if (days || hours || minutes) {
            res.append(String(minutes)).append(" minutes ");
        }
        if (!days) {
            res.append(String(seconds)).append(" seconds ");
        }
        if (!days && !hours) {
            res.append(String(millsec)).append(" millsec");
        }
        return res;
    }

    static string getLocalTime() {
        time_t current = chrono::system_clock::to_time_t(chrono::system_clock::now());
        auto a = string(ctime(&current));
        return a.substr(0, a.size() - 1);
    }

    static string getLocalTimeNs() {
        unsigned long ns = (unsigned long) (std::chrono::steady_clock::now().time_since_epoch().count());
        return getLocalTime() + " ns: " + to_string(ns);
    }

    static int getYear() {
        time_t t = time(nullptr);
        tm *timePtr = localtime(&t);
        return 1900 + timePtr->tm_year;
    }

    static int getMonth() {
        time_t t = time(nullptr);
        tm *timePtr = localtime(&t);
        return 1 + timePtr->tm_mon;
    }

    static int getDay() {
        time_t t = time(nullptr);
        tm *timePtr = localtime(&t);
        return timePtr->tm_mday;
    }
};

