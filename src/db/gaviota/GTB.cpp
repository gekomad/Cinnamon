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

#include "GTB.h"

GTB::GTB() {
    load();
}

GTB::~GTB() {
    tbcache_done();
    tb_done();
    paths = tbpaths_done(paths);
}


bool GTB::load() {
    if (path.size() == 0)return false;
    memset(installedPieces, 0, sizeof(installedPieces));
    if (!FileUtil::fileExists(path)) {
        cout << "file not found " << path << endl;
        return false;
    }
    tbstats_reset();
    paths = tbpaths_done(paths);
    paths = tbpaths_init();
    _assert(paths);
    paths = tbpaths_add(paths, path.c_str());
    restart();
    unsigned av = tb_availability();
    if (0 != (av & 2)) {
        setInstalledPieces(3);
        cout << "3-pc TBs complete\n";
    } else if (0 != (av & 1)) {
        cout << "Some 3-pc TBs available\n";
    } else {
        cout << "No 3-pc TBs available\n";
    }
    if (0 != (av & 8)) {
        setInstalledPieces(4);
        cout << "4-pc TBs complete\n";
    } else if (0 != (av & 4)) {
        cout << "Some 4-pc TBs available\n";
    } else {
        cout << "No 4-pc TBs available\n";
    }
    if (0 != (av & 32)) {
        setInstalledPieces(5);
        cout << "5-pc TBs complete\n";
    } else if (0 != (av & 16)) {
        cout << "Some 5-pc TBs available\n";
    } else {
        cout << "No 5-pc TBs available\n";
    }
    cout << endl;
    if (!getAvailable()) {
        return false;
    }
    setCacheSize(cacheSize);
    tb_init(verbosity, scheme, paths);
    tbcache_init(cacheSize * 1024 * 1024, wdl_fraction);
    tbstats_reset();
    return true;
}

int GTB::getCache() const {
    return cacheSize;
}

bool GTB::isInstalledPieces(const int p) const {
    if (p > 5)return false;
    return installedPieces[p];
}

string GTB::getPath() const {
    return path;
}

string GTB::getSchema() const {
    if (scheme == tb_CP1) {
        return "cp1";
    }
    if (scheme == tb_CP2) {
        return "cp2";
    }
    if (scheme == tb_CP3) {
        return "cp3";
    }
    if (scheme == tb_CP4) {
        return "cp4";
    }
    return "tb_UNCOMPRESSED";
}

bool GTB::getAvailable() const {
    for (int i = 3; i < 6; i++)
        if (installedPieces[i]) {
            return true;
        }
    return false;
}

bool GTB::setCacheSize(const int mb) {
    if (mb < 1 || mb > 1024) {
        return false;
    }
    cacheSize = mb;
    tbcache_init(cacheSize * 1024 * 1024, wdl_fraction);
    restart();
    return true;
}

bool GTB::setScheme(const string &s) {
    bool res = false;
    if (s == "cp1") {
        scheme = tb_CP1;
        res = true;
    } else if (s == "cp2") {
        scheme = tb_CP2;
        res = true;
    } else if (s == "cp3") {
        scheme = tb_CP3;
        res = true;
    } else if (s == "cp4") {
        scheme = tb_CP4;
        res = true;
    }
    if (res) {
        restart();
    }
    return res;
}

void GTB::restart() {
    tb_restart(verbosity, scheme, paths);
    tbcache_restart(cacheSize * 1024 * 1024, wdl_fraction);
}

bool GTB::setInstalledPieces(const int n) {
    installedPieces[n] = true;
    return true;
}

bool GTB::setPath(const string &path1) {
    path = path1;
    return load();
}

#include "../syzygy/tbprobe.h"

int GTB::convertToSyzygy(const int stm, const int info) const {
    switch (info) {
        case tb_DRAW :
            return TB_DRAW;
        case tb_WMATE :
            return (stm == tb_WHITE_TO_MOVE) ? TB_WIN : TB_LOSS;
        case tb_BMATE :
            return (stm == tb_BLACK_TO_MOVE) ? TB_WIN : TB_LOSS;
        default:
            return INT_MAX;
    }
}

int GTB::getDtmWdl(const int stm,
                   const int doPrint,
                   const _Tchessboard &chessboard,
                   unsigned *pliestomate,
                   const bool dtm, const uchar RIGHT_CASTLE) const {

    unsigned info = tb_UNKNOWN;    /* default, no tbvalue */

    GTBchessboard gtbChessboard;
    gtbChessboard.fromChessboard(chessboard, RIGHT_CASTLE);
    if (dtm) {
        int tb_available = tb_probe_soft(stm, tb_NOSQUARE, gtbChessboard.tb_castling, gtbChessboard.ws,
                                         gtbChessboard.bs,
                                         gtbChessboard.wp, gtbChessboard.bp, &info, pliestomate);
        if (!tb_available)
            tb_available = tb_probe_hard(stm, tb_NOSQUARE, gtbChessboard.tb_castling, gtbChessboard.ws,
                                         gtbChessboard.bs,
                                         gtbChessboard.wp, gtbChessboard.bp, &info, pliestomate);

        if (tb_available) {
            if (doPrint != 0) {
                const int stm1 = (doPrint == 1) ? stm : X(stm);
                if (info == tb_DRAW) {
                    cout << "Draw";
                } else if (info == tb_WMATE && stm1 == tb_WHITE_TO_MOVE) {
                    cout << "Loss, plies=" << *pliestomate;
                } else if (info == tb_BMATE && stm1 == tb_BLACK_TO_MOVE) {
                    cout << "Loss, plies=" << *pliestomate;
                } else if (info == tb_WMATE && stm1 == tb_BLACK_TO_MOVE) {
                    cout << "Win, plies=" << *pliestomate;
                } else if (info == tb_BMATE && stm1 == tb_WHITE_TO_MOVE) {
                    cout << "Win, plies=" << *pliestomate;
                } else {
                    cout << "none";
                }
                cout << endl;
            }
            assert(info != tb_UNKNOWN);
            return convertToSyzygy(stm, info);
        }
        return INT_MAX;
    } else {

        int tb_available = tb_probe_WDL_soft(stm, tb_NOSQUARE, gtbChessboard.tb_castling, gtbChessboard.ws,
                                             gtbChessboard.bs, gtbChessboard.wp, gtbChessboard.bp, &info);
        if (!tb_available)
            tb_available = tb_probe_WDL_hard(stm, tb_NOSQUARE, gtbChessboard.tb_castling, gtbChessboard.ws,
                                             gtbChessboard.bs, gtbChessboard.wp, gtbChessboard.bp, &info);

        if (tb_available) {
            if (doPrint != 0) {
                const int stm1 = (doPrint == 1) ? stm : X(stm);
                switch (info) {
                    case tb_WMATE :
                        if (stm1 == tb_WHITE_TO_MOVE) cout << "Loss" << endl;
                        else cout << "Win" << endl;
                        break;
                    case tb_BMATE :
                        if (stm1 == tb_BLACK_TO_MOVE) cout << "Loss" << endl;
                        else cout << "Win" << endl;
                        break;
                    case tb_DRAW :
                        cout << "draw" << endl;
                        break;
                    default :
                        cout << "none" << info << endl;
                        break;
                }
            }
            assert(info != tb_UNKNOWN);
            return convertToSyzygy(stm, info);
        }
        if (doPrint != 0) cout << "none" << endl;
        return INT_MAX;
    }

}
