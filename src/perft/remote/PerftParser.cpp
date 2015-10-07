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

#include "PerftParser.h"
#include "../locale/Perft.h"

void PerftParser::parser(const string &msg) {
    debug<LOG_LEVEL::INFO, false>(LINE_INFO, "receive");
    Message message(msg);
#ifdef DEBUG_MODE
    message.print();
#endif
//    if(......)

    Perft &perft = Perft::getInstance();
    perft.setParam(message.getFen(), message.getDepth(), 1, message.getHashsize(), message.getDumpFile(), false);
    perft.setCallbackResult(new PerftResultCallback());
    perft.start();

//    PerftClient c;
//    sleep(10);
//    Message b(message);
//    b.setPartial(10);
////    sendMsg(b.getSerializedString());
//    c.sendMsg("10.0.3.1", SOCK_PORT, b);
//    sleep(5);
//    b.setPartial(30);
//    c.sendMsg("10.0.3.1", SOCK_PORT, b);
//    sleep(5);
//    b.setTot(100);
//    c.sendMsg("10.0.3.1", SOCK_PORT, b);

};
