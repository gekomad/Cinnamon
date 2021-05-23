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

#include "../threadPool/Thread.h"
#include <vector>
#include <functional>

class Timer: public Thread<Timer> {
public:

    Timer(const int seconds1);

    void endRun();

    void run();

    void registerObservers(const function<void(void)>& f);

    void notifyObservers(void);

    virtual ~Timer();

private:
    int seconds;
    condition_variable cv;
    mutex mtx;
    vector<function<void(void)>> observers;
};
