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
    memset(&structure, 0, sizeof(_Tboard));
    if ((chessboard[SIDETOMOVE_IDX] = loadFen(fenString)) == 2) {
        fatal("Bad FEN position format ", fenString);
        std::_Exit(1);
    }
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

void ChessBoard::makeZobristKey() {
    chessboard[ZOBRISTKEY_IDX] = 0;
    for (int u = 0; u < 12; u++) {
        u64 c = chessboard[u];
        while (c) {
            int position = Bits::BITScanForward(c);
            updateZobristKey(u, position);
            RESET_LSB(c);
        }
    }
    if (chessboard[ENPASSANT_IDX] != NO_ENPASSANT) {
        updateZobristKey(ENPASSANT_IDX, chessboard[ENPASSANT_IDX]);
    }
    u64 x2 = chessboard[RIGHT_CASTLE_IDX];
    while (x2) {
        int position = Bits::BITScanForward(x2);
        updateZobristKey(14, position);
        RESET_LSB(x2);
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

string ChessBoard::boardToFen() const {
    string fen;
    for (int y = 0; y < 8; y++) {
        int l = 0;
        string row;
        for (int x = 0; x < 8; x++) {
            int q = getPieceAt<BLACK>(POW2[63 - ((y * 8) + x)]);
            if (q == SQUARE_FREE) {
                q = getPieceAt<WHITE>(POW2[63 - ((y * 8) + x)]);
            }
            if (q == SQUARE_FREE) {
                l++;
            } else {
                if (l > 0) {
                    row.append(1, (char) (l + 48));
                }
                l = 0;
                row.append(1, FEN_PIECE[q]);
            }
        }
        if (l > 0) {
            row.append(1, (char) (l + 48));
        }
        fen.append(row.c_str());
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
    fen.append(" 0 1");
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
                    s[ix++] = SQUARE_FREE;
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
        if (p != SQUARE_FREE) {
            updateZobristKey(p, i);
            chessboard[p] |= POW2[i];
        } else {
            chessboard[p] &= NOTPOW2[i];
        }
    };

    for (unsigned e = 0; e < castle.length(); e++) {
        switch (castle.at(e)) {
            case 'K':
                updateZobristKey(RIGHT_CASTLE_IDX, 4);
                ASSERT(Bits::BITScanForward(4 == RIGHT_KING_CASTLE_WHITE_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_WHITE_MASK;
                break;
            case 'k':
                updateZobristKey(RIGHT_CASTLE_IDX, 6);
                ASSERT(Bits::BITScanForward(6 == RIGHT_KING_CASTLE_BLACK_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
                break;
            case 'Q':
                updateZobristKey(RIGHT_CASTLE_IDX, 5);
                ASSERT(Bits::BITScanForward(5 == RIGHT_QUEEN_CASTLE_WHITE_MASK));
                chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
                break;
            case 'q':
                updateZobristKey(RIGHT_CASTLE_IDX, 7);
                ASSERT(Bits::BITScanForward(7 == RIGHT_QUEEN_CASTLE_BLACK_MASK));
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
            updateZobristKey(ENPASSANT_IDX, chessboard[ENPASSANT_IDX]);
            break;
        }
    }
    return chessboard[SIDETOMOVE_IDX];
}

#ifdef DEBUG_MODE

bool ChessBoard::checkNPieces(std::unordered_map<int, int> pieces) {
    int a = 0;
    for (int i = 0; i < 13; i++) {
        a += Bits::bitCount(chessboard[i]) == pieces[i];
    }
    return a == 13;
}

#endif