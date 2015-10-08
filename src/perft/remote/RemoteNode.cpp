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

#include "RemoteNode.h"

void RemoteNode::run() {
    ASSERT(message);
    string a = message->getSerializedString();
    info( "send ", a, "to", host, "waiting for result..");
    c.sendMsg(host, port, a);
    mutex mtx;
    unique_lock<std::mutex> lck(mtx);
    cv.wait(lck, [this]() { return end == 1; });
    debug( "end run ", host);
}

void RemoteNode::endRun() {
    debug( "exit remoteNode ", host);
}

void RemoteNode::setRemoteNode(const int port1, const string &fen, const int depth, const int from, const int to, const tuple<string, int, int, string> node) {
    host = get<0>(node);
    port = port1;
    int Ncpu = get<1>(node);
    int hashsize = get<2>(node);
    string dumpFile = get<3>(node);

    message = new Message(fen, depth, hashsize, Ncpu, dumpFile, from, to, -1, -1);
}