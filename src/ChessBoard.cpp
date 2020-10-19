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

#include "ChessBoard.h"

ChessBoard::ChessBoard() {
    fenString = string(STARTPOS);
    memset(&structureEval, 0, sizeof(_Tboard));
    if ((chessboard[SIDETOMOVE_IDX] = loadFen(fenString)) == 2) {
        fatal("Bad FEN position format ", fenString);
        std::exit(1);
    }
}

ChessBoard::~ChessBoard() {

}

void ChessBoard::makeZobristKey() {
    chessboard[ZOBRISTKEY_IDX] = 0;

    for (int u = 0; u < 12; u++) {
        for (u64 c = chessboard[u]; c; RESET_LSB(c)) {
            const int position = BITScanForward(c);
            updateZobristKey(u, position);
        }
    }


    for (u64 x2 = chessboard[RIGHT_CASTLE_IDX]; x2; RESET_LSB(x2)) {
        const int position = BITScanForward(x2);
        updateZobristKey(RIGHT_CASTLE_IDX, position);//12
    }

    if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
        updateZobristKey(ENPASSANT_IDX, chessboard[ENPASSANT_IDX]);//13
    }

    updateZobristKey(SIDETOMOVE_IDX, chessboard[SIDETOMOVE_IDX]); //14

}

const string ChessBoard::getFen() const {
    return fenString;
}

int ChessBoard::loadFen() {
    return loadFen(fenString);
}

int ChessBoard::loadFen(const string& fen) {
    if (fen.empty()) {
        return loadFen();
    }
    fenString = fen;
    memset(chessboard, 0, sizeof(_Tchessboard));
    istringstream iss(fen);
    string pos, castle, enpassant, side;
    iss >> pos;
    iss >> side;
    iss >> castle;
    iss >> enpassant;
    int ix = 0;
    array<int, 64> s;
    for (unsigned ii = 0; ii < pos.length(); ii++) {
        uchar ch = pos.at(ii);
        if (ch != '/') {
            if (INV_FEN[ch] != 0xFF) {
                s[ix++] = INV_FEN[ch];
            } else if (ch > 47 && ch < 58) {
                for (int t = 0; t < ch - 48; t++) {
                    s[ix++] = SQUARE_EMPTY;
                }
            } else {
                return 2;
            };
        }
    }
    if (ix != 64) {
        return 2;
    }
    if (side == "b") {
        chessboard[SIDETOMOVE_IDX] = BLACK;
    } else if (side == "w") {
        chessboard[SIDETOMOVE_IDX] = WHITE;
    } else {
        return 2;
    }

    for (int i = 0; i < 64; i++) {
        int p = s[63 - i];
        if (p != SQUARE_EMPTY) {
            updateZobristKey(p, i);
            chessboard[p] |= POW2[i];
        } else {
            chessboard[p] &= NOTPOW2[i];
        }
    };

    for (unsigned e = 0; e < castle.length(); e++) {
        const char c = castle.at(e);
        switch (c) {
            case 'K':
                startPosWhiteKing = BITScanForward(chessboard[KING_WHITE]);
                startPosWhiteRookKingSide = BITScanForward(chessboard[ROOK_WHITE] & 0xffULL);
                updateZobristKey(RIGHT_CASTLE_IDX, 4);
                ASSERT(4 == BITScanForward(RIGHT_KING_CASTLE_WHITE_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_WHITE_MASK;
                break;
            case 'k':
                startPosBlackKing = BITScanForward(chessboard[KING_BLACK]);
                startPosBlackRookKingSide = BITScanForward(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
                updateZobristKey(RIGHT_CASTLE_IDX, 6);
                ASSERT(6 == BITScanForward(RIGHT_KING_CASTLE_BLACK_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
                break;
            case 'Q':
                startPosWhiteKing = BITScanForward(chessboard[KING_WHITE]);
                startPosWhiteRookQueenSide = BITScanReverse(chessboard[ROOK_WHITE] & 0xffULL);
                updateZobristKey(RIGHT_CASTLE_IDX, 5);
                ASSERT(5 == BITScanForward(RIGHT_QUEEN_CASTLE_WHITE_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
                break;
            case 'q':
                startPosBlackKing = BITScanForward(chessboard[KING_BLACK]);
                startPosBlackRookQueenSide = BITScanReverse(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
                updateZobristKey(RIGHT_CASTLE_IDX, 7);
                ASSERT(7 == BITScanForward(RIGHT_QUEEN_CASTLE_BLACK_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
                break;
            case '-':
                break;
            default:
                //x-fen
                const int wKing = FILE_AT[BITScanForward(chessboard[KING_WHITE])];
                const int bKing = FILE_AT[BITScanForward(chessboard[KING_BLACK])];

                if (isupper(c)) {
                    startPosWhiteKing = BITScanForward(chessboard[KING_WHITE]);
                    if (board::getFile(c) < wKing) {
                        startPosWhiteRookKingSide = BITScanForward(chessboard[ROOK_WHITE] & 0xffULL);
                        updateZobristKey(RIGHT_CASTLE_IDX, 4);
                        ASSERT(4 == BITScanForward(RIGHT_KING_CASTLE_WHITE_MASK));
                        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_WHITE_MASK;
                        break;
                    } else {
                        startPosWhiteRookQueenSide = BITScanReverse(chessboard[ROOK_WHITE] & 0xffULL);
                        updateZobristKey(RIGHT_CASTLE_IDX, 5);
                        ASSERT(5 == BITScanForward(RIGHT_QUEEN_CASTLE_WHITE_MASK));
                        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
                        break;
                    }
                } else {
                    startPosBlackKing = BITScanForward(chessboard[KING_BLACK]);
                    if (board::getFile(c) < bKing) {
                        startPosBlackRookKingSide = BITScanForward(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
                        updateZobristKey(RIGHT_CASTLE_IDX, 6);
                        ASSERT(6 == BITScanForward(RIGHT_KING_CASTLE_BLACK_MASK));
                        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
                        break;
                    } else {
                        startPosBlackRookQueenSide = BITScanReverse(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
                        updateZobristKey(RIGHT_CASTLE_IDX, 7);
                        ASSERT(7 == BITScanForward(RIGHT_QUEEN_CASTLE_BLACK_MASK));
                        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
                        break;
                    }
                }

        }
    }
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    for (int i = 0; i < 64; i++) {
        if (enpassant == BOARD[i]) {
            chessboard[ENPASSANT_IDX] = i;
            if (chessboard[SIDETOMOVE_IDX]) {
                chessboard[ENPASSANT_IDX] -= 8;
            } else {
                chessboard[ENPASSANT_IDX] += 8;
            }
            updateZobristKey(ENPASSANT_IDX, chessboard[ENPASSANT_IDX]);
            break;
        }
    }

    return chessboard[SIDETOMOVE_IDX];
}
