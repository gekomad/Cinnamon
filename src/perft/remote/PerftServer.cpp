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

#include "PerftServer.h"
#include "Message.h"

void PerftServer::receive(string msg) {
    Message message(msg);

    cout << "host: " << message.getHost() << "\n";
    cout << "tot: " << message.getTot() << "\n";
    cout << "partial: " << message.getPartial() << "\n";

    if (message.getTot() != -1) {
        for (int i = 0; i < threadPool.size(); i++) {
//            if (threadPool.at(i)->message->getHost() == message.getHost()) {
//                threadPool.at(i)->endWork();
//                break;
//            }
        }
    }


};

