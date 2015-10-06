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


#include "Server.h"

void Server::run() {
    int read_size;
    struct sockaddr_in client;
    char client_message[MAX_MSG_SIZE];
    while (1) {
        listen(socket_desc, 3);
        int c = sizeof(struct sockaddr_in);
        int client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c);
        assert (client_sock >= 0);
        while ((read_size = recv(client_sock, client_message, Server::MAX_MSG_SIZE, 0)) > 0) {
            cout << "Server::read " << client_message << "\n";
            write(client_sock, _def::OK.c_str(), strlen(_def::OK.c_str()) + 1);
            cout <<"aaaaaaaaaaaa"<<endl;
            receive(string(client_message));
            cout <<"bbbbbbbbbbbb"<<endl;
        }
    }
}

void Server::dispose() {
    if (sockfd >= 0)close(sockfd);
}

void Server::endRun() {
    dispose();
}

Server::~Server() {
    dispose();
}

Server::Server(int portno) {
    struct sockaddr_in server;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    assert (socket_desc != -1);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portno);
    int on = 1;
    assert(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) >= 0);
    assert (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) >= 0);
}
