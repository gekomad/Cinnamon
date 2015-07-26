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
#ifdef DEBUG_MODE

#include <iostream>

using namespace std;
using namespace std::chrono;
static mutex _CoutSyncMutex;

struct CoutSync {
    stringstream s;

    template<typename T>
    CoutSync &operator<<(const T &x) {
        s << x;
        return *this;
    }

    ~CoutSync() {
        lock_guard<mutex> lock1(_CoutSyncMutex);
        nanoseconds ms = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
        cout << "info string TIME: " << ms.count() << " " << s.str() << "\n";
    }
};

#endif