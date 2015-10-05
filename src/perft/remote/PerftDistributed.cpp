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
#include "../remote/PerftServer.h"

PerftDistributed::~PerftDistributed() {

}

std::set<tuple<string, int, int, string>> PerftDistributed::getRemoteNodes(string distributedFile) {


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
                nodesSet.insert(make_tuple(nodeIp, nodeNcores, nodeHash, nodeDumpfile));
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
        nodesSet.insert(make_tuple(nodeIp, nodeNcores, nodeHash, nodeDumpfile));
    }
    cout << nodesSet.size() << " nodes\n";
    return nodesSet;
}

void PerftDistributed::setParam(string fen1, int depth1, string distributedFile, int port1) {
    PerftServer s(port1);
    s.start();
    usleep(10000);//wait complete startup
    if (fen1.empty()) {
        fen1 = STARTPOS;
    }

    depth = depth1;
    if (depth <= 0)depth = 1;
//    perftRes.depth = depth1;
    fen = fen1;
//    count = 0;
//    dumping = false;
    port = port1;
    nodesSet = getRemoteNodes(distributedFile);

}

void PerftDistributed::run() {
    int tot = 20;// getNmoves();
    int from = 0;
    ThreadPool<RemoteNode> threadPool(nodesSet.size());

    Client c;

    for (auto node:nodesSet) {
        string host = get<0>(node);
        int Ncpu = get<1>(node);
        int hashsize = get<2>(node);
        string dumpFile1 = get<3>(node);
        String f(fen);
        f = f.replace(' ', 1);
        String dumpFile(dumpFile1);
        dumpFile = dumpFile.replace(' ', 1);

        int to = from + Ncpu;
        if (to >= tot) {
            to = tot - 1;
        }

        string a(f + " " + String(depth) + " " + String(hashsize) + " " + dumpFile, from, to);
        from += to;
        c.sendMsg(host, port, a);
    }
}

void PerftDistributed::endRun() {

}
