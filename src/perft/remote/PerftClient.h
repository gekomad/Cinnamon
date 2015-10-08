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

#include "Message.h"
#include "../../network/Client.h"

class PerftClient : public Client {
public :
    ~PerftClient() { N_CLIENT--; }

    PerftClient() { N_CLIENT++; }

    void sendMsg(const string &host, int portno, const Message &msg) {
        Client::sendMsg(host, portno, msg.getSerializedString());
    }

};

