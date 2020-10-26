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

string ChessBoard::boardToFen() const {
    string fen;
    for (int y = 0; y < 8; y++) {
        int l = 0;
        string row;
        for (int x = 0; x < 8; x++) {
            int q = board::getPieceAt<BLACK>(POW2[63 - ((y * 8) + x)], chessboard);
            if (q == SQUARE_EMPTY) {
                q = board::getPieceAt<WHITE>(POW2[63 - ((y * 8) + x)], chessboard);
            }
            if (q == SQUARE_EMPTY) {
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
        fen += whiteRookKingSideCastle;
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK) {
        fen += whiteRookQueenSideCastle;
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK) {
        fen += blackRookKingSideCastle;
        cst++;
    }
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK) {
        fen += blackRookQueenSideCastle;
        cst++;
    }
    if (!cst) {
        fen.append("-");
    }
    if (chessboard[ENPASSANT_IDX] == NO_ENPASSANT) {
        fen.append(" -");
    } else {
        fen.append(" ");
        chessboard[SIDETOMOVE_IDX] ? fen.append(BOARD[chessboard[ENPASSANT_IDX] + 8]) : fen.append(
                BOARD[chessboard[ENPASSANT_IDX] - 8]);
    }
    fen.append(" 0 ");
    fen.append(to_string(movesCount));
    return fen;
}

void ChessBoard::display() const {
    cout << endl << "     a   b   c   d   e   f   g   h";
    for (int t = 0; t <= 63; t++) {
        char x = ' ';
        if (t % 8 == 0) {
            cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
            cout << " " << 8 - RANK_AT[t] << " | ";
        }
        x = (x = (x = FEN_PIECE[board::getPieceAt<WHITE>(POW2[63 - t], chessboard)]) != '-' ? x
                                                                                            : FEN_PIECE[board::getPieceAt<BLACK>(
                        POW2[63 - t], chessboard)]) == '-' ? ' ' : x;
        x != ' ' ? cout << x : POW2[t] & WHITE_SQUARES ? cout << " " : cout << ".";
        cout << " | ";
    };
    cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
    cout << "     a   b   c   d   e   f   g   h" << endl << endl << "fen:\t\t" << boardToFen() << endl;

    cout << "side:\t\t" << (chessboard[SIDETOMOVE_IDX] ? "White" : "Black") << endl;
    cout << "castle:\t\t";
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK) cout << whiteRookKingSideCastle;
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK) cout << whiteRookQueenSideCastle;
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK) cout << blackRookKingSideCastle;
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK) cout << blackRookQueenSideCastle;
    cout << endl;

    cout << "ep:\t\t\t"
         << (chessboard[ENPASSANT_IDX] == 100 ? "" : (chessboard[SIDETOMOVE_IDX] ? BOARD[chessboard[ENPASSANT_IDX] + 8]
                                                                                 : BOARD[chessboard[ENPASSANT_IDX] -
                                                                                         8])) << endl;
    cout << "Chess960:\t" << (chess960 ? "true" : "false") << endl;
#ifdef DEBUG_MODE
    cout << "zobristKey:\t0x" << hex << chessboard[ZOBRISTKEY_IDX] << "ull" << dec << endl;
#endif
    cout << endl;
}

string ChessBoard::moveToString(const _Tmove *move) {
    string a = decodeBoardinv(move->s.type, move->s.from, move->s.side);
    if (move->s.type & 0xc) return a;
    string b = decodeBoardinv(move->s.type, move->s.to, move->s.side);
    if (move->s.promotionPiece != -1) (a + b) += (char) tolower(FEN_PIECE[move->s.promotionPiece]);
    return a + b;
}

void ChessBoard::print(const _Tmove *move, const _Tchessboard &chessboard) {
    cout << moveToString(move) << " " << flush;
}

string ChessBoard::decodeBoardinv(const uchar type, const int a, const int side) {
    if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
        return isChess960() ? BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookQueenSide] : "e1c1";
    }
    if (type & KING_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
        return isChess960() ? BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookKingSide] : "e1g1";
    }
    if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
        return isChess960() ? BOARD[startPosBlackKing] + BOARD[startPosBlackRookQueenSide] : "e8c8";
    }
    if (type & KING_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
        return isChess960() ? BOARD[startPosBlackKing] + BOARD[startPosBlackRookKingSide] : "e8g8";
    }
    ASSERT(!(type & 0xC));
    if (a >= 0 && a < 64) {
        return BOARD[a];
    }
    _assert(0);
}

int ChessBoard::loadFen(const string &fen) {
    if (fen.empty()) {
        return loadFen();
    }
    startPosWhiteKing = -1;
    startPosWhiteRookKingSide = -1;
    startPosWhiteRookQueenSide = -1;

    startPosBlackKing = -1;
    startPosBlackRookKingSide = -1;
    startPosBlackRookQueenSide = -1;
    MATCH_QUEENSIDE = "";
    MATCH_QUEENSIDE_WHITE = "O-O-O ";
    MATCH_KINGSIDE_WHITE = "O-O ";
    MATCH_QUEENSIDE_BLACK = "O-O-O ";
    MATCH_KINGSIDE_BLACK = "O-O ";
    fenString = fen;
    memset(chessboard, 0, sizeof(_Tchessboard));
    istringstream iss(fen);
    string pos, castle, enpassant, side, a1, a2;
    iss >> pos;
    iss >> side;
    iss >> castle;
    iss >> enpassant;
    iss >> a1;
    iss >> a2;
    a2 += " 1";
    movesCount = stoi(a2);

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
    }
    startPosWhiteKing = BITScanForward(chessboard[KING_WHITE]);
    startPosBlackKing = BITScanForward(chessboard[KING_BLACK]);
    auto whiteRookKingSide = [&](const char c) {
        startPosWhiteRookKingSide = BITScanForward(chessboard[ROOK_WHITE] & 0xffULL);
        updateZobristKey(RIGHT_CASTLE_IDX, 4);
        ASSERT(4 == BITScanForward(RIGHT_KING_CASTLE_WHITE_MASK));
        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_WHITE_MASK;
        whiteRookKingSideCastle = c;
    };
    auto blackRookKingSide = [&](const char c) {
        startPosBlackRookKingSide = BITScanForward(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
        updateZobristKey(RIGHT_CASTLE_IDX, 6);
        ASSERT(6 == BITScanForward(RIGHT_KING_CASTLE_BLACK_MASK));
        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_KING_CASTLE_BLACK_MASK;
        blackRookKingSideCastle = c;
    };
    auto whiteRookQueenSide = [&](const char c) {
        startPosWhiteRookQueenSide = BITScanReverse(chessboard[ROOK_WHITE] & 0xffULL);
        updateZobristKey(RIGHT_CASTLE_IDX, 5);
        ASSERT(5 == BITScanForward(RIGHT_QUEEN_CASTLE_WHITE_MASK));
        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
        whiteRookQueenSideCastle = c;
    };
    auto blackRookQueenSide = [&](const char c) {
        startPosBlackRookQueenSide = BITScanReverse(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
        updateZobristKey(RIGHT_CASTLE_IDX, 7);
        ASSERT(7 == BITScanForward(RIGHT_QUEEN_CASTLE_BLACK_MASK));
        chessboard[RIGHT_CASTLE_IDX] |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
        blackRookQueenSideCastle = c;
    };
    for (unsigned e = 0; e < castle.length(); e++) {
        const char c = castle.at(e);
        switch (c) {
            case 'K':
                whiteRookKingSide(c);
                break;
            case 'k':
                blackRookKingSide(c);
                break;
            case 'Q':
                whiteRookQueenSide(c);
                break;
            case 'q':
                blackRookQueenSide(c);
                break;
            case '-':
                break;
            default:
                //x-fen
                setChess960(true);
                const int wKing = FILE_AT[BITScanForward(chessboard[KING_WHITE])];
                const int bKing = FILE_AT[BITScanForward(chessboard[KING_BLACK])];

                if (isupper(c)) {
                    if (board::getFile(c) < wKing) {
                        whiteRookKingSide(c);
                        break;
                    } else {
                        whiteRookQueenSide(c);
                        break;
                    }
                } else {

                    if (board::getFile(c) < bKing) {
                        blackRookKingSide(c);
                        break;
                    } else {
                        blackRookQueenSide(c);
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

    if (isChess960()) {
        if (startPosWhiteRookKingSide != -1)
            MATCH_KINGSIDE_WHITE += BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookKingSide];
        if (startPosBlackRookQueenSide != -1)
            MATCH_QUEENSIDE_BLACK += BOARD[startPosBlackKing] + BOARD[startPosBlackRookQueenSide];
        if (startPosWhiteRookQueenSide != -1)
            MATCH_QUEENSIDE_WHITE += BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookQueenSide];
        if (startPosBlackRookKingSide != -1)
            MATCH_KINGSIDE_BLACK += BOARD[startPosBlackKing] + BOARD[startPosBlackRookKingSide];
        if (startPosBlackRookQueenSide != -1)
            MATCH_QUEENSIDE +=
                    MATCH_QUEENSIDE_BLACK + " " + BOARD[startPosBlackKing] + BOARD[startPosBlackRookQueenSide];
        if (startPosWhiteRookQueenSide != -1)
            MATCH_QUEENSIDE +=
                    MATCH_QUEENSIDE_WHITE + " " + BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookQueenSide];
    } else {
        MATCH_QUEENSIDE_WHITE += "e1c1";
        MATCH_KINGSIDE_WHITE += "e1g1";
        MATCH_QUEENSIDE_BLACK += "e8c8";
        MATCH_KINGSIDE_BLACK += "e8g8";
        MATCH_QUEENSIDE = MATCH_QUEENSIDE_WHITE + " e8c8";
    }

    return chessboard[SIDETOMOVE_IDX];
}
