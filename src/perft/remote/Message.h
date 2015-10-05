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
#include "../../namespaces/def.h"
#include "../../network/Server.h"

using namespace _debug;

class Message {

public:

    typedef struct {
        string host;
        string fen;
        string dumpFile;
        int depth;
        int hashsize;
        int from;
        int to;
        unsigned long long partial;
        unsigned long long tot;
    } _Tmessage;

    const static char SEPARATOR = 1;

    bool static compare(_Tmessage &a, _Tmessage &b) {
        if (a.depth != b.depth
            ||a.host != b.host||
            a.dumpFile != a.dumpFile ||
            a.fen != b.fen ||
            a.from != b.from ||
            a.to != b.to ||
            a.hashsize != b.hashsize
            || a.partial != b.partial||
                a.tot != b.tot
                )
            return false;
        return true;
    }

    string static serialize(_Tmessage &m) {

        char a[Server::MAX_MSG_SIZE];
        memset(a, 0, Server::MAX_MSG_SIZE);
        strcat(a, m.host.c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, m.fen.c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.depth).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.hashsize).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.dumpFile).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.from).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.to).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.partial).c_str());
        a[strlen(a)] = SEPARATOR;
        strcat(a, String(m.tot).c_str());

        string b(a);
#ifdef DEBUG_MODE
        _Tmessage x = Message::deserialize(b);
        ASSERT(Message::compare(m, x));
#endif
        return b;
    }

    _Tmessage static deserialize(string m) {
        cout <<m<<endl;
#ifdef DEBUG_MODE
        int c=0;
for(int i=0;i<m.size();i++)if(m.at(i)==1)c++;
assert(c==8);
#endif
        stringstream ss(m);

        _Tmessage tmessage;

        char sep = SEPARATOR;
        string dummy;
        getline(ss, tmessage.host, sep);
        getline(ss, dummy, sep);
        getline(ss, tmessage.fen, sep);
        getline(ss, dummy, sep);
        tmessage.depth = stoi(dummy);
        getline(ss, dummy, sep);
        assert(dummy.size()<10);
        tmessage.hashsize = stoi(dummy);
        getline(ss, tmessage.dumpFile, sep);
        getline(ss, dummy, sep);
        tmessage.from = stoi(dummy);
        getline(ss, dummy, sep);
        tmessage.to = stoi(dummy);

        getline(ss, dummy, sep);
        tmessage.partial = stoull(dummy);
        getline(ss, dummy, sep);
        tmessage.tot = stoull(dummy);
        return tmessage;
    }


};

