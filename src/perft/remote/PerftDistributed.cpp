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
#include "../../network/Client.h"

PerftDistributed::~PerftDistributed() {

}

std::vector<tuple<string, int, int, string>> PerftDistributed::getRemoteNodes(string distributedFile) {

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
    cout << nodesSet.size() << " nodes\n";
    return nodesSet;
}

void PerftDistributed::setParam(string fen1, int depth1, string distributedFile, int port1) {

    if (fen1.empty()) {
        fen1 = STARTPOS;
    }
    depth = depth1;
    if (depth <= 0)depth = 1;
    fen = fen1;
    port = port1;

    nodesSet = getRemoteNodes(distributedFile);

}

void PerftDistributed::run() {
//    PerftServer s(port);
//    s.start();
//    usleep(10000);//wait complete startup

    int totMoves = 20;
    // getNmoves();TODO
    int from = 0;

    int totMachine = 0;
    int c = 0;

    for (totMachine = 0; totMachine < nodesSet.size(); totMachine++) {
        c += std::get<1>(nodesSet[totMachine]);
        if (c >= totMoves)break;
    }
    totMachine++;
    int form = 0;
    int to = 0;
    setNthread(totMachine);
    for (int i = 0; i < totMachine; i++) {
        RemoteNode &remoteNode = getNextThread();
        //nodeIp, nodeNcores, nodeHash, nodeDumpfile
        remoteNode.setParam(port, fen, depth, from, to, nodesSet[i]);
    }
    startAll();
    joinAll();

}

void PerftDistributed::endRun() {

}
