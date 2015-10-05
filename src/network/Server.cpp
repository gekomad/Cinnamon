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

    while (1) {

        listen(socket_desc, 3);

        c = sizeof(struct sockaddr_in);

        client_sock = accept(socket_desc, (struct sockaddr *) &client, (socklen_t *) &c);
        assert (client_sock >= 0);

        while ((read_size = recv(client_sock, client_message, Server::MAX_MSG_SIZE, 0)) > 0) {
            strcat(client_message," FROM SERVER");
            cout << "Server::read " << client_message << "\n";
            write(client_sock, client_message, strlen(client_message));
        }

        assert(read_size != -1);
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

Server::Server(int port) {
    portno = port;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    assert (socket_desc != -1);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(portno);

    int on = 1;
    assert(setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) >= 0);

    assert (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) >= 0)


}
