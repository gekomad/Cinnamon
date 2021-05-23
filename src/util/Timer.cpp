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

#include "Timer.h"

Timer::Timer(const int seconds1) {
    seconds = seconds1;
}

void Timer::endRun() { }

void Timer::run() {
    unique_lock<mutex> lck(mtx);
    while (seconds) {
        cv.wait_for(lck, chrono::seconds(seconds));
        if (seconds) {
            notifyObservers();
        }
    }
}

void Timer::registerObservers(const function<void(void)>& f) {
    observers.push_back(f);
}

void Timer::notifyObservers() {
    for (auto & observer : observers) {
        observer();
    }
}

Timer::~Timer() {
    seconds = 0;
    cv.notify_all();
    join();
}
