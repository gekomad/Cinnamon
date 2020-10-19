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

SYZYGY::SYZYGY() = default;

int SYZYGY::setPath(const string &path) {
    sz_tb_init(path.c_str());
    setInstalledPieces(TB_LARGEST);
    return TB_LARGEST;
}

unsigned SYZYGY::SZtbProbeRoot(const u64 white,const u64 black,const _Tchessboard &c, const bool side, unsigned *results) {

    return tb_probe_root(decode(white),
                                   decode(black),
                                   decode(c[KING_BLACK] | c[KING_WHITE]),
                                   decode(c[QUEEN_BLACK] | c[QUEEN_WHITE]),
                                   decode(c[ROOK_BLACK] | c[ROOK_WHITE]),
                                   decode(c[BISHOP_BLACK] | c[BISHOP_WHITE]),
                                   decode(c[KNIGHT_BLACK] | c[KNIGHT_WHITE]),
                                   decode(c[PAWN_BLACK] | c[PAWN_WHITE]),
                                   0, 0,
                                   0,
                                   side, results);
}

unsigned SYZYGY::SZtbProbeWDL(const _Tchessboard &c, const bool turn) const {

    return tb_probe_wdl(decode(board::getBitmap<WHITE>(c)),
                        decode(board::getBitmap<BLACK>(c)),
                        decode(c[KING_BLACK] | c[KING_WHITE]),
                        decode(c[QUEEN_BLACK] | c[QUEEN_WHITE]),
                        decode(c[ROOK_BLACK] | c[ROOK_WHITE]),
                        decode(c[BISHOP_BLACK] | c[BISHOP_WHITE]),
                        decode(c[KNIGHT_BLACK] | c[KNIGHT_WHITE]),
                        decode(c[PAWN_BLACK] | c[PAWN_WHITE]),
                        0,
                        0, 0,
                        turn);
}

//Little-Endian Rank-File Mapping
u64 SYZYGY::decode(u64 c) {

    static constexpr array<u64, 64> _decode = {
            0x80ull, 0x40ull, 0x20ull, 0x10ull, 0x8ull, 0x4ull, 0x2ull, 0x1ull, 0x8000ull, 0x4000ull, 0x2000ull,
            0x1000ull, 0x800ull, 0x400ull, 0x200ull, 0x100ull, 0x800000ull, 0x400000ull, 0x200000ull,
            0x100000ull, 0x80000ull, 0x40000ull, 0x20000ull, 0x10000ull, 0x80000000ull, 0x40000000ull, 0x20000000ull,
            0x10000000ull, 0x8000000ull, 0x4000000ull, 0x2000000ull, 0x1000000ull,
            0x8000000000ull, 0x4000000000ull, 0x2000000000ull, 0x1000000000ull, 0x800000000ull, 0x400000000ull,
            0x200000000ull, 0x100000000ull, 0x800000000000ull, 0x400000000000ull,
            0x200000000000ull, 0x100000000000ull, 0x80000000000ull, 0x40000000000ull, 0x20000000000ull,
            0x10000000000ull, 0x80000000000000ull, 0x40000000000000ull, 0x20000000000000ull,
            0x10000000000000ull, 0x8000000000000ull, 0x4000000000000ull, 0x2000000000000ull, 0x1000000000000ull,
            0x8000000000000000ull, 0x4000000000000000ull, 0x2000000000000000ull,
            0x1000000000000000ull, 0x800000000000000ull, 0x400000000000000ull, 0x200000000000000ull,
            0x100000000000000ull
    };

    u64 res = 0;
    for (; c; RESET_LSB(c)) {
        int position = BITScanForward(c);
        res |= _decode[position];
    }
    return res;
}

