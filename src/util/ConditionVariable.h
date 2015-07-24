/*
    Cinnamon is a UCI chess engine
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

using namespace std;

class ConditionVariable : public condition_variable {
public:

    void wait() {
        waiting = true;
        unique_lock<mutex> lck(mtx);
        condition_variable::wait(lck);
    }

    void notifyOne() {
        checkLock();
        condition_variable::notify_one();
        waiting = false;
    }

    void notifyAll() {
        checkLock();
        condition_variable::notify_all();
        waiting = false;
    }

private:
    void checkLock() {
        while (!waiting) {
#ifdef DEBUG_MODE
            CoutSync() << "ConditionVariable::checkLock waiting==true  ";
#endif
            ;
        }
    }

    mutex mtx;
    volatile bool waiting = false;
};
