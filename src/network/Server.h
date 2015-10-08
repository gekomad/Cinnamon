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


#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../blockingThreadPool/Thread.h"
#include "Iparser.h"
#include "../perft/remote/Message.h"

class Server : public Thread {

public:
    static const int MAX_MSG_SIZE = 2048;

    Server(int port, Iparser *parser);

    void run();

    void endRun();

    ~Server();

    void sendMsg(const string &msg);

private:

    int client_sock = -98691;
    int sockfd = -1;
    int port;
    int socket_desc;
    struct sockaddr_in client;

    void dispose();

    int c;
    Iparser *parser;
};
