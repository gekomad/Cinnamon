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

#include <mutex>

#include "../../blockingThreadPool/ThreadPool.h"

#include <set>

#include "../remote/RemoteNode.h"
#include "../../util/IniFile.h"
#include "PerftParser.h"
#include "PerftDistributed.h"
#include "../../network/Client.h"


class PerftDistributed : public Thread, public ThreadPool<RemoteNode>, public Singleton<PerftDistributed> {
    friend class Singleton<PerftDistributed>;

public:


    void setParam(const string &fen1, int depth1, const string &distributedFile, int port);

    void setServer(int port1);

    ~PerftDistributed();

    virtual void run();

    virtual void endRun();


private:

    void receiveMsg(const Message &message);

    std::vector<tuple<string, int, int, string>> nodesSet;

    PerftDistributed() : ThreadPool(1) { };

    std::vector<tuple<string, int, int, string>> getRemoteNodes(const string &distributedFile);

    int depth;
    string fen;
    int port;

    void callRemoteNode();

    int getTotMoves(const string &fen);

    bool serverMode = false;
    Server *server = nullptr;
public:
    Server *getServer() const {
        return server;
    }
};

