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
#include "../namespaces/def.h"
#include "Server.h"

void Client::sendMsg(string msg) {
    assert(msg.size() < Server::MAX_MSG_SIZE)
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "client ERROR opening socket" << endl;
        return;
    }
    server = gethostbyname(host.c_str());

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "client ERROR connecting" << endl;
        return;
    }

    n = write(sockfd, msg.c_str(), strlen(msg.c_str()));
    if (n < 0) {
        cout << "client ERROR writing to socket" << endl;
        return;
    }
    char buffer[256];
    bzero(buffer, 256);
    n = read(sockfd, buffer, 255);
    if (n < 0) {
        cout << "client ERROR reading from socket" << endl;
        return;
    }

    cout << "client msg ricevuto: " << buffer << endl;
    close(sockfd);

}
