/*
    Cinnamon UCI chess engine
    Copyright (C) GiuSEPARATORpe Cannella

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


#include "Message.h"


bool Message::compare(Message &b) {
    if (depth != b.depth) return false;
    if (host != b.host)return false;
    if (dumpFile != dumpFile)return false;
    if (fen != b.fen)return false;
    if (from != b.from)return false;
    if (to != b.to)return false;
    if (hashsize != b.hashsize)return false;
    if (partial != b.partial)return false;
    if (tot != b.tot) return false;
    return true;
}

Message::Message(const string host1, const string fen1, const int depth1, const int hashsize1, const string dumpFile1, const int from1, const int to1, const u64 partial1, const u64 tot1) {
    assert(host1.size()>2);
    assert(fen1.size() > 10);
    assert(depth1 > 0);
    assert(from1 >= to1);
    char a[Server::MAX_MSG_SIZE];
    memset(a, 0, Server::MAX_MSG_SIZE);
    strcat(a, host1.c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, fen1.c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(depth1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(hashsize1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(dumpFile1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(from1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(to1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(partial1).c_str());
    a[strlen(a)] = SEPARATOR;
    strcat(a, String(tot1).c_str());

    string b(a);
    serializedString.assign(b);

    host = host1;
    fen = fen1;
    dumpFile = dumpFile1;
    depth = depth1;
    hashsize = hashsize1;
    from = from1;
    to = to1;
    partial = partial1;
    tot = tot1;
#ifdef DEBUG_MODE
    Message x(serializedString);
    ASSERT(compare(x));
#endif
}

Message::Message(string m) {
#ifdef DEBUG_MODE
    int c = 0;
    for (int i = 0; i < m.size(); i++)if (m.at(i) == 1)c++;
    assert(c == 8);
#endif
    stringstream ss(m);

    string dummy;
    getline(ss, host, SEPARATOR);
    getline(ss, fen, SEPARATOR);
    getline(ss, dummy, SEPARATOR);
    depth = stoi(dummy);
    getline(ss, dummy, SEPARATOR);
    hashsize = stoi(dummy);
    getline(ss, dumpFile, SEPARATOR);
    getline(ss, dummy, SEPARATOR);
    from = stoi(dummy);
    getline(ss, dummy, SEPARATOR);
    to = stoi(dummy);
    getline(ss, dummy, SEPARATOR);
    partial = stoull(dummy);
    getline(ss, dummy, SEPARATOR);
    tot = stoull(dummy);
    serializedString = m;
}

