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

u64 Client::TOT = 0;
int Client::N_CLIENT = 0;
int Client::endClient = 0;

void Client::sendMsg(const string &host, int portno, const string &msg) {
    lock_guard<mutex> lock(clientMutex);

    assert(msg.size() < Server::MAX_MSG_SIZE)

    struct sockaddr_in server;
    char server_reply[Server::MAX_MSG_SIZE];

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
    while (!closeSocket) {
        recv(sock, server_reply, Server::MAX_MSG_SIZE, 0);
        cout << "Client::reply from servers: " << server_reply << "\n";
        if (string(server_reply) != "OK") {//aggiungere decorator parser che chiama PerftClient, TOT e N_CLIENT va in PErftClient
            Message message(server_reply);
            if (message.getTot() != 0xffffffffffffffff) {
                cout << "node: " << message.getHost() << " tot: " << message.getTot() << "\n";
                TOT += message.getTot();
                endClient++;
                cout << "TOT " << TOT << " (" << endClient << "/" << N_CLIENT << " nodes)" << endl;
            }
        }
    }
    close(sock);
}

