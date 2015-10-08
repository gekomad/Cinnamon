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


#include "PerftDistributed.h"
#include "../locale/PerftThread.h"

PerftDistributed::~PerftDistributed() {
    debug( "~PerftDistributed()");
}

std::vector<tuple<string, int, int, string>> PerftDistributed::getRemoteNodes(const string &distributedFile) {

    IniFile iniFile(distributedFile);
    string nodeIp;
    int nodeNcores;
    int nodeHash;
    string nodeDumpfile;

    bool newNode = false;
    while (true) {
        pair<string, string> *parameters = iniFile.get();
        if (!parameters) {
            break;
        }
        if (parameters->first == "[node]") {
            if (newNode) {
                assert(nodeIp != "" && nodeNcores != -1 && nodeHash != -1 && nodeDumpfile != "*");
                nodesSet.push_back(make_tuple(nodeIp, nodeNcores, nodeHash, nodeDumpfile));
                nodeIp = "";
                nodeNcores = -1;
                nodeHash = -1;
                nodeDumpfile = "*";
            }
            newNode = true;
        } else if (parameters->first == "ip") {
            nodeIp = parameters->second;
            assert(nodeIp.size() > 0);
        } else if (parameters->first == "core") {
            nodeNcores = stoi(parameters->second);
            assert(nodeNcores > 0);
        } else if (parameters->first == "hash") {
            nodeHash = 0;
            if (parameters->second.size()) {
                nodeHash = stoi(parameters->second);
            }
        } else if (parameters->first == "dump_file") {
            nodeDumpfile = parameters->second;

        }

    }
    if (newNode) {
        assert(nodeIp != "" && nodeNcores != -1 && nodeHash != -1 && nodeDumpfile != "*");
        nodesSet.push_back(make_tuple(nodeIp, nodeNcores, nodeHash, nodeDumpfile));
    }

    info( nodesSet.size(), "nodes");
    return nodesSet;
}

void PerftDistributed::setServer(int port1) {
    debug( "SERVER MODE on port", port1);
    serverMode = true;
    port = port1;
}

void PerftDistributed::setParam(const string &fen1, int depth1, const string &distributedFile, int port1) {
    debug( "setParam");
    serverMode = false;
    if (fen1.empty()) {
        fen = STARTPOS;
    } else {
        fen = fen1;
    }
    depth = depth1;
    if (depth <= 0)depth = 1;

    port = port1;
    nodesSet = getRemoteNodes(distributedFile);
}

void PerftDistributed::run() {
    debug( "run");

    if (serverMode) {
        server = new Server(port, new PerftParser());//TODO delete PerftParser deleteserver??
        server->start();
        usleep(10000);//wait complete startup
    } else {
        callRemoteNode();
    }
}

void PerftDistributed::endRun() {
    debug( "endRun");

}

void PerftDistributed::receiveMsg(const Message &message) {
    info( "PerftServer:: receive msg from host: ", message.getHost(), message.getSerializedString());

    if (message.getTot() != 0xffffffffffffffff)info( "PerftServer::tot:", message.getTot());

    if (message.getPartial() != 0xffffffffffffffff) info( "PerftServer::partial:", message.getPartial());

    if (message.getTot() != 0xffffffffffffffff) {
        for (unsigned i = 0; i < threadPool.size(); i++) {
            if (threadPool.at(i)->getHost() == message.getHost()) {
                threadPool.at(i)->endWork();
                break;
            }
        }
    }
}

int PerftDistributed::getTotMoves(const string &fen1) {
    PerftThread p;

    p.loadFen(fen1);

    p.setPerft(true);
    int side = p.getSide() ? 1 : 0;

    p.incListId();
    u64 friends = side ? p.getBitBoard<WHITE>() : p.getBitBoard<BLACK>();
    u64 enemies = side ? p.getBitBoard<BLACK>() : p.getBitBoard<WHITE>();
    p.generateCaptures(side, enemies, friends);
    p.generateMoves(side, friends | enemies);
    return p.getListSize();

}

void PerftDistributed::callRemoteNode() {
    debug( "callRemoteNode");
    assert(nodesSet.size());
    int totMoves = getTotMoves(fen);

    unsigned totMachine = 0;
    int c = 0;

    for (totMachine = 0; totMachine < nodesSet.size(); totMachine++) {
        c += std::get<1>(nodesSet[totMachine]);
        if (c >= totMoves)break;
    }

    int from = 0;
    int to = 0;
    setNthread(totMachine);
    int block = totMoves / totMachine;
    int lastBlock = totMoves % totMachine;
    for (unsigned i = 0; i < totMachine; i++) {
        RemoteNode &remoteNode = getNextThread();
        to += block;
        if (i == totMachine - 1)to += lastBlock;
        debug(from+" "+to);
        cout << from << " " << to << endl;
        remoteNode.setRemoteNode(port, fen, depth, from, to - 1, nodesSet[i]);
        from = to;
    }
    startAll();
    joinAll();
}
