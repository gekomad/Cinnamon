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

#include "board.h"

u64 board::colors(const int pos) {
    if (POW2[pos] & 0x55aa55aa55aa55aaULL)return 0x55aa55aa55aa55aaULL;
    return 0xaa55aa55aa55aa55ULL;
}


#ifdef DEBUG_MODE

u64 board::getBitmap(const int side, const _Tchessboard &chessboard) {
    return side ? getBitmap<WHITE>(chessboard) : getBitmap<BLACK>(chessboard);
}

int board::getPieceAt(int side, const u64 bitmapPos, const _Tchessboard &chessboard) {
    return side == WHITE ? getPieceAt<WHITE>(bitmapPos, chessboard) : getPieceAt<BLACK>
            (bitmapPos, chessboard);
}

#endif

int board::getSide(const _Tchessboard &chessboard) {
    return chessboard[SIDETOMOVE_IDX];
}

u64 board::performRankFileCaptureAndShift(const int position, const u64 enemies, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63)
    const u64 rankFile = Bitboard::getRankFile(position, allpieces);
    return (rankFile & enemies) | (rankFile & ~allpieces);
}


int board::getDiagShiftCount(const int position, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63);
    return bitCount(Bitboard::getDiagonalAntiDiagonal(position, allpieces) & ~allpieces);
}

bool board::checkInsufficientMaterial(const int nPieces, const _Tchessboard &chessboard) {
    //regexp: KN?B*KB*
    switch (nPieces) {
        case 2 :
            //KK
            return true;
        case 3:
            //KBK
            if (chessboard[BISHOP_BLACK] || chessboard[BISHOP_WHITE])return true;
            //KNK
            if (chessboard[KNIGHT_BLACK] || chessboard[KNIGHT_WHITE])return true;
            break;
        case 4 :
            //KBKB
            if (chessboard[BISHOP_BLACK] && chessboard[BISHOP_WHITE])return true;
            //KNKN
            if (chessboard[KNIGHT_BLACK] && chessboard[KNIGHT_WHITE])return true;
            //KBKN
            if (chessboard[BISHOP_BLACK] && chessboard[KNIGHT_WHITE])return true;
            if (chessboard[BISHOP_WHITE] && chessboard[KNIGHT_BLACK])return true;
            //KNNK
            if (bitCount(chessboard[KNIGHT_BLACK]) == 2)return true;
            if (bitCount(chessboard[KNIGHT_WHITE]) == 2)return true;
            break;
        default:
            return false;
    }
    return false;
}

u64 board::getDiagShiftAndCapture(const int position, const u64 enemies, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63);
    u64 nuovo = Bitboard::getDiagonalAntiDiagonal(position, allpieces);
    return (nuovo & enemies) | (nuovo & ~allpieces);
}

u64 board::getMobilityQueen(const int position, const u64 enemies, const u64 allpieces) {
    ASSERT_RANGE(position, 0, 63)
    return performRankFileCaptureAndShift(position, enemies, allpieces) +
           getDiagShiftAndCapture(position, enemies, allpieces);
}

u64 board::getMobilityRook(const int position, const u64 enemies, const u64 friends) {
    ASSERT_RANGE(position, 0, 63)
    return performRankFileCaptureAndShift(position, enemies, enemies | friends);
}

bool board::isCastleRight_WhiteKing(const _Tchessboard &chessboard) {
    return chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK;
}

bool board::isCastleRight_BlackKing(const _Tchessboard &chessboard) {
    return chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK;
}

bool board::isCastleRight_WhiteQueen(const _Tchessboard &chessboard) {
    return chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK;
}

bool board::isCastleRight_BlackQueen(const _Tchessboard &chessboard) {
    return chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK;
}

char board::decodeBoard(string a) {
    for (int i = 0; i < 64; i++) {
        if (!a.compare(BOARD[i])) return i;
    }
    error(a);
    ASSERT(0);
    return -1;
}

int board::getFile(const char cc) {
    return 104 - tolower(cc);
}

bool board::isOccupied(const uchar pos, const u64 allpieces) {
    return allpieces & POW2[pos];
}


string board::getCell(const int file, const int rank) {
    return BOARD[FILE_AT[file] * 8 + rank];
}

bool board::isPieceAt(const uchar pieces, const uchar pos, const _Tchessboard &chessboard) {
    return chessboard[pieces] & POW2[pos];
}
