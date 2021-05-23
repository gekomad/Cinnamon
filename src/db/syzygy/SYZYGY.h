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

#include "../../namespaces/bits.h"
#include "../../ChessBoard.h"
#include "../../util/Singleton.h"
#include "tbprobe.h"
#include "../../GenMoves.h"

#include <map>

using namespace std;


class SYZYGY : public Singleton<SYZYGY> {
    friend class Singleton<SYZYGY>;

public:
    int SZtbProbeWDL(const _Tchessboard &chessboard, const int side, const int tot) const {
        if (getInstalledPieces() >= tot) {
            BENCH_AUTO_CLOSE("syzygyTime")
            return SZtbProbeWDL(chessboard, side);
        }
        return TB_RESULT_FAILED;
    }

    int setPath(const string &path);

    static char getPromotion(const unsigned promotion) {
        if (promotion == 1) return 'q';
        if (promotion == 2) return 'r';
        if (promotion == 3) return 'b';
        if (promotion == 4) return 'n';
        return NO_PROMOTION;
    }

    unsigned SZtbProbeRoot(const u64, const u64, const _Tchessboard &c, const bool turn, unsigned *);

    unsigned SZtbProbeWDL(const _Tchessboard &c, const bool turn) const;

    void setInstalledPieces(const int i) {
        installedPieces = i;
    }

    ~SYZYGY() = default;

    int getInstalledPieces() const {
        return installedPieces;
    }

    bool createSYZYGY(string path) {

        installedPieces = setPath(path);
        if (!installedPieces) {
            cout << "error: unable to initialize syzygy lib; no lib files found" << endl;
            return false;
        }
        cout << "syzygy tb use " << installedPieces << " pieces" << endl;

        return true;
    }

private:
#ifdef BENCH_MODE
    Times *times = &Times::getInstance();
#endif
    int installedPieces = 0;

    SYZYGY();

    static u64 decode(u64 d);
};

