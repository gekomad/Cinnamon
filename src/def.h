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

static constexpr int MAX_PLY = 96;

typedef unsigned char uchar;
typedef long long unsigned u64;
typedef u64 _Tchessboard[16];

typedef struct {
    u64 allPieces;
    u64 kingAttackers[2];
    u64 allPiecesSide[2];
    u64 allPiecesNoPawns[2];
    u64 posKingBit[2];
    int kingSecurity[2];
    uchar posKing[2];
} _Tboard;


typedef union {
    u64 u;
    struct s {
        char promotionPiece;
        char pieceFrom;
        uchar capturedPiece;
        uchar from;
        uchar to;
        char side;
        uchar type;
    } s;
} _Tmove;

typedef struct {
    _Tmove *moveList;
    int size;
} _TmoveP;

typedef struct {
    int cmove;
    _Tmove argmove[MAX_PLY];
} _TpvLine;
