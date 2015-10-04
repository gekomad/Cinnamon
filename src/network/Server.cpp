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
    while (true) {
        struct sockaddr_in cli_addr;

        listen(sockfd, 5);
        socklen_t clilen = sizeof(cli_addr);
        int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            cout << "server ERROR on accept" << endl;
            return;
        }

        bzero(buffer, MAX_MSG_SIZE);
        int n = read(newsockfd, buffer, MAX_MSG_SIZE-1);
        if (n < 0) {
            cout << "server ERROR reading from socket" << endl;
            close(newsockfd);
            return;
        }
        cout << "server - Here is the message: " << buffer << endl;

        n = write(newsockfd, "I got your message", 18);
        if (n < 0) {
            cout << "server ERROR writing to socket" << endl;
            close(newsockfd);
            return;
        }
        close(newsockfd);
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
    struct sockaddr_in serv_addr;
    Server::portno = port;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "server ERROR opening socket" << endl;
        return;
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "server ERROR on binding" << endl;
        dispose();
        exit(1);
    }

}
