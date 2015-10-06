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


#include "Client.h"

mutex Client::clientMutex;

void Client::sendMsg(string host, int portno, string msg) {
    lock_guard<mutex> lock(clientMutex);
    assert(msg.size() < Server::MAX_MSG_SIZE)

    struct sockaddr_in server;
    char server_reply[_def::OK.size() + 1];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != -1);

    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(portno);

    assert(connect(sock, (struct sockaddr *) &server, sizeof(server)) >= 0);

    assert (send(sock, msg.c_str(), strlen(msg.c_str()) + 1, 0) >= 0);

    assert(recv(sock, server_reply, _def::OK.size() + 1, 0) >= 0);
    cout << "Client::reply from servers: " << server_reply << "\n";
    assert(server_reply == _def::OK);
    close(sock);
}
