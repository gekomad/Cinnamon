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

#ifdef JS_MODE

GTB::GTB() { }

GTB::~GTB() { }

bool GTB::getAvailable() const { return false; }

int GTB::getCache() const { return 0; }

string GTB::getPath() const { return ""; }

string GTB::getSchema() const { return ""; }

bool GTB::setCacheSize(const int mb) { return false; }

bool GTB::setPath(const string &path) { return false; }

bool GTB::setScheme(const string &s) { return false; }

void GTB::restart() { }

bool GTB::setProbeDepth(const int d) { return false; }

bool GTB::setInstalledPieces(const int n) { return false; }

bool GTB::isInstalledPieces(const int p) const { return false; }

int GTB::getProbeDepth() const { return 0; }

void GTB::print(const unsigned stm1, const unsigned info1, const unsigned pliestomate1) const { }

int GTB::extractDtm(const unsigned stm1, const bool doPrint, const int tb_available1, const unsigned info1, const unsigned pliestomate1) const { return 0; }

int GTB::getDtm(const int side, const bool doPrint, const _Tchessboard &chessboard, const uchar rightCastle, const int depth) const { return 0; };

#else

GTB::GTB() {
    load();
}

GTB::~GTB() {
    tbcache_done();
    tb_done();
    paths = tbpaths_done(paths);
}

int GTB::getProbeDepth() const {
    return probeDepth;
}

bool GTB::load() {
    memset(installedPieces, 0, sizeof(installedPieces));
    if (!FileUtil::fileExists(path)) {
        debug("file not exists ", path);
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
    ASSERT(p < 33);
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

void GTB::print(const unsigned stm1, const unsigned info1, const unsigned pliestomate1) const {
    if (info1 == tb_DRAW) {
        cout << "Draw";
    } else if (info1 == tb_WMATE && stm1 == tb_WHITE_TO_MOVE) {
        cout << "White mates, plies=" << pliestomate1;
    } else if (info1 == tb_BMATE && stm1 == tb_BLACK_TO_MOVE) {
        cout << "Black mates, plies=" << pliestomate1;
    } else if (info1 == tb_WMATE && stm1 == tb_BLACK_TO_MOVE) {
        cout << "Black is mated, plies=" << pliestomate1;
    } else if (info1 == tb_BMATE && stm1 == tb_WHITE_TO_MOVE) {
        cout << "White is mated, plies=" << pliestomate1;
    } else {
        cout << "none";
    }
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

bool GTB::setProbeDepth(const int d) {
    if (d < 0 || d > 5) {
        return false;
    }
    probeDepth = d;
    return true;
}

bool GTB::setInstalledPieces(const int n) {
    if (n < 3 || n > 5) {
        return false;
    }
    installedPieces[n] = true;
    return true;
}

bool GTB::setPath(const string &path1) {
    path = path1;
    return load();
}

int GTB::extractDtm(const unsigned stm1, const bool doPrint, const int tb_available1, const unsigned info1, const unsigned pliestomate1) const {
    if (doPrint) {
        print(stm1, info1, pliestomate1);
    }
    if (tb_available1) {
        if (info1 == tb_DRAW) {
            return 0;
        }
        if (info1 == tb_WMATE && stm1 == tb_WHITE_TO_MOVE) {
            return pliestomate1;
        }
        if (info1 == tb_BMATE && stm1 == tb_BLACK_TO_MOVE) {
            return pliestomate1;
        }
        if (info1 == tb_WMATE && stm1 == tb_BLACK_TO_MOVE) {
            return -pliestomate1;
        }
        if (info1 == tb_BMATE && stm1 == tb_WHITE_TO_MOVE) {
            return -pliestomate1;
        }
    }
    return INT_MAX;
}

int GTB::getDtm(const int side, const bool doPrint, const _Tchessboard &chessboard, const uchar rightCastle, const int depth) const {
    unsigned int ws[17];    /* list of squares for white */
    unsigned int bs[17];    /* list of squares for black */
    unsigned char wp[17];    /* what white pieces are on those squares */
    unsigned char bp[17];    /* what black pieces are on those squares */
    unsigned info = tb_UNKNOWN;    /* default, no tbvalue */
    unsigned pliestomate = 0;
    int count = 0;
    //white
    for (int piece = 1; piece < 12; piece += 2) {
        u64 b = chessboard.bit[piece];
        while (b) {
            int position = BITScanForward(b);
            ws[count] = DECODE_POSITION[position];
            wp[count] = DECODE_PIECE[piece];
            count++;
            RESET_LSB(b);
        }
    }
    ws[count] = tb_NOSQUARE;    /* it marks the end of list */
    wp[count] = tb_NOPIECE;    /* it marks the end of list */
    //black
    count = 0;
    for (int piece = 0; piece < 12; piece += 2) {
        u64 b = chessboard.bit[piece];
        while (b) {
            int position = BITScanForward(b);
            bs[count] = DECODE_POSITION[position];
            bp[count] = DECODE_PIECE[piece];
            count++;
            RESET_LSB(b);
        }
    }
    bs[count] = tb_NOSQUARE;
    bp[count] = tb_NOPIECE;
    unsigned int tb_castling = 0;
    tb_castling = rightCastle & ChessBoard::RIGHT_QUEEN_CASTLE_WHITE_MASK ? tb_WOOO : 0;
    tb_castling |= rightCastle & ChessBoard::RIGHT_KING_CASTLE_WHITE_MASK ? tb_WOO : 0;
    tb_castling |= rightCastle & ChessBoard::RIGHT_KING_CASTLE_BLACK_MASK ? tb_BOO : 0;
    tb_castling |= rightCastle & ChessBoard::RIGHT_QUEEN_CASTLE_BLACK_MASK ? tb_BOOO : 0;
    int tb_available = 0;
    if (depth > 8) {
        tb_available = tb_probe_hard(side ^ 1, tb_NOSQUARE, tb_castling, ws, bs, wp, bp, &info, &pliestomate);
    } else if (depth >= probeDepth) {
        tb_available = tb_probe_soft(side ^ 1, tb_NOSQUARE, tb_castling, ws, bs, wp, bp, &info, &pliestomate);
    }
    return extractDtm(side ^ 1, doPrint, tb_available, info, pliestomate);
}

#endif
