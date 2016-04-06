/*
    https://github.com/gekomad/ThreadPool
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

#include <thread>
#include <mutex>
#include "ObserverThread.h"
#include "../namespaces/def.h"
#include <condition_variable>

using namespace std;

template<typename T>
class Thread {

private:
    bool running = true;
    int threadID;
    ObserverThread *observer = nullptr;
    condition_variable cv;
    thread theThread;

    void _run() {
        static_cast<T *>(this)->run();
        static_cast<T *>(this)->endRun();
        if (observer != nullptr) {
            observer->observerEndThread(threadID);
        }
    }

public:
    template<typename O, typename = typename std::enable_if<std::is_base_of<ObserverThread, O>::value, O>::type>
    void registerObserverThread(ObserverThread *obs) {
        observer = static_cast<O *>(obs);
    }

    virtual ~Thread() {
        join();
    }

    void checkWait() {
        while (!running) {
            mutex mtx;
            unique_lock<mutex> lck(mtx);
            cv.wait(lck);
        }
    }

    void notify() {
        cv.notify_all();
    }

    void start() {
        ASSERT(!isJoinable());
        theThread = thread(&Thread::_run, this);
    }

    void join() {
        if (theThread.joinable()) {
            theThread.join();
        }
    }

    void detach() {
        theThread.detach();
    }

    int getId() const {
        return threadID;
    }

    void setId(int id) {
        threadID = id;
    }

    void threadSleep(bool b) {
        running = !b;
    }

    bool isJoinable() {
        return theThread.joinable();
    }

    void setSleep(bool b) {
        running = !b;
    }

};
