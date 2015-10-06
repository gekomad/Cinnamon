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

#include "../../Search.h"
#include <iomanip>
#include <atomic>
#include <fstream>
#include <unistd.h>
#include "../../util/Timer.h"
#include <mutex>

#include <signal.h>
#include <set>
#include "Message.h"

#include "../../network/Server.h"
#include "../../blockingThreadPool/ThreadPool.h"
#include "RemoteNode.h"

class PerftServer : public Server {
public :
    PerftServer(int port) : Server(port) { }

    void registerObservers(function<void(Message message)> f) {
        observers.push_back(f);
    }

    void notifyObservers(Message message) {
        for (auto i = observers.begin(); i != observers.end(); ++i) {
            (*i)(message);
        }
    }

protected:

    virtual void receive(string msg);

private:
    vector<function<void(Message message)>> observers;


};

