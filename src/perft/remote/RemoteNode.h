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


class RemoteNode : public Thread {

public:
    void setParam(int port, string fen, int depth, int from, int to, tuple<string, int, int, string> node) {
        string host = get<0>(node);
        int Ncpu = get<1>(node);
        int hashsize = get<2>(node);
        string dumpFile = get<3>(node);
        Message::_Tmessage m;
        m.fen = fen;
        m.depth = depth;
        m.dumpFile = dumpFile;
        m.hashsize = hashsize;
        m.from = from;
        m.to = to;
        string a = Message::serialize(m);


        Client c;
        c.sendMsg(host, port, a);
    }

    virtual void run();

    virtual void endRun();

};

