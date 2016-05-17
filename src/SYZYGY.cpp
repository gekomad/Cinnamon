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

#include "SYZYGY.h"
#include "syzygy/tbprobe.h"

#ifdef JS_MODE
bool SYZYGY::getAvailable() const{return false;}

string SYZYGY::getPath() const{return "";}

bool SYZYGY::setPath(const string &path){return false;}

void SYZYGY::restart(){}

int SYZYGY::getDtm() const{return INT_MAX;}

string SYZYGY::getBestmove(){return false;}
#else

SYZYGY::SYZYGY() {

}

SYZYGY::~SYZYGY() {

}

bool SYZYGY::getAvailable() const { return false; }

string SYZYGY::getPath() const { return ""; }

bool SYZYGY::setPath(const string &path) {
    SYZYGY::path = path;
    tb_init_syzygy(path.c_str());
    return TB_LARGEST;
}

void SYZYGY::restart() { }

int SYZYGY::getDtm(const _Tchessboard &c) const {

    unsigned res;//= tb_probe_root(pos->white, pos->black, pos->kings,
    //             pos->queens, pos->rooks, pos->bishops, pos->knights, pos->pawns,
    //           pos->rule50, pos->castling, pos->ep, pos->turn, results);
    _assert (res != TB_RESULT_FAILED);
    TB_GET_DTZ(res);
    return INT_MAX;
}

string SYZYGY::getBestmove() {

    return "";
}

#endif
