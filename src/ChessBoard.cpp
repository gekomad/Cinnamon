/*
    Cinnamon is a UCI chess engine
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
    memset(&structure, 0, sizeof(_Tboard));
    chessboard[SIDETOMOVE_IDX] = loadFen(fenString);
}

ChessBoard::~ChessBoard() {
}

#ifdef DEBUG_MODE

u64 ChessBoard::getBitBoard(int side) {
    return side ? getBitBoard<WHITE>() : getBitBoard<BLACK>();
}

int ChessBoard::getPieceAt(int side, u64 bitmapPos) {
    return side ? getPieceAt<WHITE>(bitmapPos) : getPieceAt<BLACK>(bitmapPos);
}

#endif

uchar ChessBoard::getRightCastle() {
    return (uchar) chessboard[RIGHT_CASTLE_IDX];
}

void ChessBoard::setRightCastle(uchar r) {
    chessboard[RIGHT_CASTLE_IDX] = r;
}

void ChessBoard::makeZobristKey() {
    chessboard[ZOBRISTKEY_IDX] = 0;
    int i = 0;
    for (int u = 0; u < 12; u++) {
        u64 c = chessboard[u];
        while (c) {
            int position = BITScanForward(c);
            updateZobristKey(i, position);
            c &= NOTPOW2[position];
        }
        i++;
    }
    if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
        updateZobristKey(13, chessboard[ENPASSANT_IDX]);
    }
    u64 x2 = chessboard[RIGHT_CASTLE_IDX];
    while (x2) {
        int position = BITScanForward(x2);
        updateZobristKey(14, position);
        x2 &= NOTPOW2[position];
    }
}


string ChessBoard::getFen() {
    return fenString;
}

int ChessBoard::getPieceByChar(char c) {
    for (int i = 0; i < 12; i++)
        if (c == FEN_PIECE[i]) {
            return i;
        }
    return -1;
}


void ChessBoard::display() {
    cout << "\n     a   b   c   d   e   f   g   h";
    for (int t = 0; t <= 63; t++) {
        char x = ' ';
        if (t % 8 == 0) {
            cout << "\n   ----+---+---+---+---+---+---+----\n";
            cout << " " << 8 - RANK_AT[t] << " | ";
        }
        x = (x = (x = FEN_PIECE[getPieceAt<WHITE>(POW2[63 - t])]) != '-' ? x : FEN_PIECE[getPieceAt<BLACK>(POW2[63 - t])]) == '-' ? ' ' : x;
        x != ' ' ? cout << x : POW2[t] & WHITE_SQUARES ? cout << " " : cout << ".";
        cout << " | ";
    };
    cout << "\n   ----+---+---+---+---+---+---+----\n";
    cout << "     a   b   c   d   e   f   g   h\n\n\n" << boardToFen() << "\n" << endl;
#ifdef DEBUG_MODE

    cout << "zobristKey: " << chessboard[ZOBRISTKEY_IDX] << "\n";
    cout << "enpassantPosition: " << chessboard[ENPASSANT_IDX] << "\n";
    cout << "rightCastle: " << (int) chessboard[RIGHT_CASTLE_IDX] << "\n";
    cout << "sideToMove: " << chessboard[SIDETOMOVE_IDX] << "\n";

#endif
}


string ChessBoard::boardToFen() {
    string fen;
    int x, y, l = 0, i = 0, sq;
    char row[8];
    int q;
    for (y = 0; y < 8; y++) {
        i = l = 0;
        strcpy(row, "");
        for (x = 0; x < 8; x++) {
            sq = (y * 8) + x;
            q = getPieceAt<BLACK>(POW2[63 - sq]);
            if (q == SQUARE_FREE) {
                q = getPieceAt<WHITE>(POW2[63 - sq]);
            }
            if (q == SQUARE_FREE) {
                l++;
            } else {
                if (l > 0) {
                    row[i] = (char) (l + 48);
                    i++;
                }
                l = 0;
                row[i] = FEN_PIECE[q];
                i++;
            }
        }
        if (l > 0) {
            row[i] = (char) (l + 48);
            i++;
        }
        fen.append(row, i);
        if (y < 7) {
            fen.append("/");
        }
    }
    if (chessboard[SIDETOMOVE_IDX] == BLACK) {
        fen.append(" b ");
    } else {
        fen.append(" w ");
    }
    int cst = 0;
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK) {
        fen.append("K");
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK) {
        fen.append("Q");
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK) {
        fen.append("k");
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK) {
        fen.append("q");
        cst++;
    }
    if (!cst) {
        fen.append("-");
    }
    if (chessboard[ENPASSANT_IDX] == NO_ENPASSANT) {
        fen.append(" -");
    } else {
        fen.append(" ");
        chessboard[SIDETOMOVE_IDX] ? fen.append(BOARD[chessboard[ENPASSANT_IDX] + 8]) : fen.append(BOARD[chessboard[ENPASSANT_IDX] - 8]);
    }
    return fen;
}

char ChessBoard::decodeBoard(string a) {
    for (int i = 0; i < 64; i++) {
        if (!a.compare(BOARD[i])) {
            return i;
        }
    }
    cout << "\n" << a << endl;
    ASSERT(0);
    return -1;
}

int ChessBoard::loadFen() {
    return loadFen(fenString);
}

int ChessBoard::loadFen(string fen) {
    if (fen.empty()) {
        return loadFen();
    }
    memset(chessboard, 0, sizeof(chessboard));
    istringstream iss(fen);
    string pos, castle, enpassant, side;
    iss >> pos;
    iss >> side;
    iss >> castle;
    iss >> enpassant;
    int ix = 0;
    int s[64];
    for (unsigned ii = 0; ii < pos.length(); ii++) {
        uchar ch = pos.at(ii);
        if (ch != '/') {
            if (INV_FEN[ch] != 0xFF) {
                s[ix++] = INV_FEN[ch];
            } else if (ch > 47 && ch < 58) {
                for (int t = 0; t < ch - 48; t++) {
                    s[ix++] = SQUARE_FREE;
                }
            } else {
                cout << "Bad FEN position format (" << (char) ch << ") " << fen << endl;
                return chessboard[SIDETOMOVE_IDX];//TODO retun -1
            };
        }
    }
    if (ix != 64) {
        cout << "Bad FEN position format " << fen << endl;
        return chessboard[SIDETOMOVE_IDX];//TODO retun -1
    }
    if (side == "b") {
        chessboard[SIDETOMOVE_IDX] = BLACK;
    } else if (side == "w") {
        chessboard[SIDETOMOVE_IDX] = WHITE;
    } else {
        cout << "Bad FEN position format " << fen << endl;
        return chessboard[SIDETOMOVE_IDX];//TODO retun -1
    }

    for (int i = 0; i < 64; i++) {
        int p = s[63 - i];
        if (p != SQUARE_FREE) {
            chessboard[p] |= POW2[i];
        } else {
            chessboard[p] &= NOTPOW2[i];
        }
    };
    chessboard[RIGHT_CASTLE_IDX] = 0;
    for (unsigned e = 0; e < castle.length(); e++) {
        switch (castle.at(e)) {
            case 'K':
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_WHITE_MASK;
                break;
            case 'k':
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
                break;
            case 'Q':
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
                break;
            case 'q':
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
                break;
            default:;
        };
    };
    chessboard[ENPASSANT_IDX] = NO_ENPASSANT;
    for (int i = 0; i < 64; i++) {
        if (enpassant == BOARD[i]) {
            chessboard[ENPASSANT_IDX] = i;
            if (chessboard[SIDETOMOVE_IDX]) {
                chessboard[ENPASSANT_IDX] -= 8;
            } else {
                chessboard[ENPASSANT_IDX] += 8;
            }
            break;
        }
    }
    makeZobristKey();
    return chessboard[SIDETOMOVE_IDX];
}
