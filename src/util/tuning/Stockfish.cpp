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
#ifdef TUNING
#include "Stockfish.h"

using namespace std;

void Stockfish::loadEPD(const string &path1) {
    load(path1);
}

void Stockfish::init1(const string &path1){
    this->path=path1;
    loadParams(path1+"/"+iniFile);
}

// send parameters to others threads
void Stockfish::shareParameters(const double error, const map<string, int> & newParams) const {
    _ShareParameterSpinlock.lock();
    for (int i = 0; i < stockfishPool::stockfishPool.getNthread() ; i++) {
        Stockfish &p = stockfishPool::stockfishPool.getThread(i);
        if (p.getId() == getId())continue;
        p.stopFlag = true;
    }
    usleep(1000);
    for (int i = 0; i < stockfishPool::stockfishPool.getNthread() ; i++) {
        Stockfish &p = stockfishPool::stockfishPool.getThread(i);
        if (p.getId() == getId())continue;
        p.sendParameters(error, newParams);
        p.stopFlag = false;
    }
    _ShareParameterSpinlock.unlock();
}

// each thread receive new params
void Stockfish::sendParameters(const double error, const map<string, int> & newParams) {

    printf("\nThread #%d update parameters...", getId());
    this->currentError=error;
    this->bestError=error;
    int chk=0;
    for (auto &param : search.eval.PARAMS) {
        *param.second.ref = newParams.at(param.first);
        chk+= *param.second.ref;
    }
    printParams(cycle, getId(),currentError,bestError, search.eval.PARAMS);
    printf("\nThread #%d update parameters OK chk: %d\n",getId(), chk);

}

void Stockfish::run() {

    search.setMaxTimeMillsec(2500);
    cout.precision(17);
    const auto params= search.eval.PARAMS;
    const int threadId=getId();
    map<string,int> oldParams;
    for (auto &param: params) oldParams[param.first] = *param.second.ref;
    const double startError = E();
    // printParams(cycle, threadId,startError,startError, params);
    bestError = startError;

    while (true) {
        cycle++;
        for (const auto& param: params) {
            const auto oldValue = *param.second.ref;
            const int newValue = randomPercent(oldValue==0?1:oldValue , 10);
            //cout << oldValue<< " "<< newValue << endl;
            *param.second.ref= newValue;
        }
        currentError = E();
        if (stopFlag){
            while (stopFlag) {
                printf("threadId #%d wait..\n",getId());
                usleep(100);
            }
            continue;
        }
        // printParams(cycle, threadId,currentError,bestError, params);

        if (currentError < bestError) {
            printf("\nThread #%d improved old best error: %.17f new best error: %.17f (Cycle #%d)\n", threadId, bestError, currentError, cycle);
            bestError = currentError;
            saveParams(iniFile, params);
            for (auto &param: params) oldParams[param.first] = *param.second.ref;
            shareParameters(currentError , oldParams);
        } else {
            if (currentError > bestError) cout <<"worse "; else cout <<"same ";
            for (auto &param : params) *param.second.ref = oldParams[param.first];
        }
    }
}
#endif
