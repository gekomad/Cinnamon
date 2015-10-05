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

#include "../../namespaces/def.h"

using namespace _debug;

class Message {

public:
    typedef struct {
        string fen;
        string dumpFile;
        int depth;
        int hashsize;
        int from;
        int to;
    } _Tmessage;

    const static char SEPARATOR = 1;

    bool static compare(_Tmessage &a, _Tmessage &b) {
        if (a.depth != b.depth ||
            a.dumpFile != a.dumpFile ||
            a.fen != b.fen ||
            a.from != b.from ||
            a.to != b.to ||
            a.hashsize != b.hashsize
                )
            return false;
        return true;
    }

    string static serialize(_Tmessage &m) {

        char a[1024];//TODO Server::MAX_MSG_SIZE];

        memset(a, 0, 1024);//TODO Server::MAX_MSG_SIZE];
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

        string b(a);
#ifdef DEBUG_MODE
        _Tmessage x = Message::deserialize(b);
        ASSERT(Message::compare(m, x));
#endif
        return b;
    }

    _Tmessage static deserialize(string m) {

        stringstream ss(m);

        _Tmessage tmessage;

        char sep = SEPARATOR;
        string dummy;
        getline(ss, tmessage.fen, sep);
        getline(ss, dummy, sep);
        tmessage.depth = stoi(dummy);
        getline(ss, dummy, sep);
        tmessage.hashsize = stoi(dummy);
        getline(ss, tmessage.dumpFile, sep);
        getline(ss, dummy, sep);
        tmessage.from = stoi(dummy);
        getline(ss, dummy, sep);
        tmessage.to = stoi(dummy);

        return tmessage;
    }


};

