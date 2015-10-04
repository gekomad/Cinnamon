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

#include "Server.h"

void Client::sendMsg(string m) {
    assert(msg.size() < Server::MAX_MSG_SIZE)
    msg = m;

}

void Client::run() {


    struct sockaddr_in server;
    char server_reply[ Server::MAX_MSG_SIZE];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock != -1);

    cout<<"Socket created\n";

    server.sin_addr.s_addr = inet_addr(host.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(portno);

    assert(connect(sock, (struct sockaddr *) &server, sizeof(server)) >= 0);

    cout<<"Connected\n";

    assert (send(sock, msg.c_str(), strlen(msg.c_str()) + 1, 0) >=0);


    assert(recv(sock, server_reply,  Server::MAX_MSG_SIZE, 0)  >=0);

    cout<<"Server reply: "<<server_reply<<"\n";

    close(sock);
}

void Client::endRun() {

}
