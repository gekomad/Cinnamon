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

#include "../../blockingThreadPool/Thread.h"
#include "../../network/Client.h"
#include "Message.h"
#include "PerftClient.h"


class RemoteNode : public Thread {

public:

    virtual ~RemoteNode() {
        debug( "~RemoteNode()");
        if (message)delete message;
        message = nullptr;
    }

    void setRemoteNode(const int port, const string &fen, const int depth, const int from, const int to, const tuple<string, int, int, string> node);

    virtual void run();

    virtual void endRun();

    void endWork() {
        end = 1;
        cv.notify_all();
    }

    const string &getHost() const {
        return host;
    }

private:
    string host;
    PerftClient c;
    Message *message = nullptr;
    int port;
    condition_variable cv;
    int end = 0;
};

