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

#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include "../blockingThreadPool/Thread.h"
#include "Server.h"
#include<arpa/inet.h>

using namespace std;

class Client {
public:

    void sendMsg(const string &host, int portno, const string &msg);

    virtual ~Client() { closeSocket = true; }

protected:
    static int N_CLIENT;
private:
    mutex clientMutex;
    bool closeSocket = false;
    static u64 TOT;

    static int endClient;
};
