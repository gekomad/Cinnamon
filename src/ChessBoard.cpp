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
    Bitboard();
    fenString = string(STARTPOS);
    if ((sideToMove = loadFen(fenString)) == 2) {
        fatal("Bad FEN position format ", fenString)
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

    for (u64 x2 = rightCastle; x2; RESET_LSB(x2)) {
        const int position = BITScanForward(x2);
        updateZobristKey(RIGHT_CASTLE_RAND, position);//12
    }

    if (enPassant != NO_ENPASSANT) {
        updateZobristKey(ENPASSANT_RAND, enPassant);//13
    }
    static constexpr uchar SIDETOMOVE_RAND = 14;
    updateZobristKey(SIDETOMOVE_RAND, sideToMove);

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
            int q = board::getPieceAt<BLACK>(POW2(63 - ((y * 8) + x)), chessboard);
            if (q == SQUARE_EMPTY) {
                q = board::getPieceAt<WHITE>(POW2(63 - ((y * 8) + x)), chessboard);
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
    if (sideToMove == BLACK) {
        fen.append(" b ");
    } else {
        fen.append(" w ");
    }
    int cst = 0;
    if (rightCastle & RIGHT_KING_CASTLE_WHITE_MASK) {
        fen += whiteRookKingSideCastle;
        cst++;
    }
    if (rightCastle & RIGHT_QUEEN_CASTLE_WHITE_MASK) {
        fen += whiteRookQueenSideCastle;
        cst++;
    }
    if (rightCastle & RIGHT_KING_CASTLE_BLACK_MASK) {
        fen += blackRookKingSideCastle;
        cst++;
    }
    if (rightCastle & RIGHT_QUEEN_CASTLE_BLACK_MASK) {
        fen += blackRookQueenSideCastle;
        cst++;
    }
    if (!cst) {
        fen.append("-");
    }
    if (enPassant == NO_ENPASSANT) {
        fen.append(" -");
    } else {
        fen.append(" ");
        sideToMove ? fen.append(BOARD[enPassant + 8]) : fen.append(BOARD[enPassant - 8]);
    }
    fen.append(" 0 ");
    fen.append(to_string(movesCount));
    return fen;
}

void ChessBoard::display() const {
    cout << endl << "     a   b   c   d   e   f   g   h";
    for (int t = 0; t <= 63; t++) {
        char x;
        if (t % 8 == 0) {
            cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
            cout << " " << 8 - RANK_AT[t] << " | ";
        }
        x = (x = (x = FEN_PIECE[board::getPieceAt<WHITE>(POW2(63 - t), chessboard)]) != '-' ? x
                                                                                            : FEN_PIECE[board::getPieceAt<BLACK>(
                        POW2(63 - t), chessboard)]) == '-' ? ' ' : x;
        x != ' ' ? cout << x : POW2(t) & WHITE_SQUARES ? cout << " " : cout << ".";
        cout << " | ";
    }
    cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
    cout << "     a   b   c   d   e   f   g   h" << endl << endl << "fen:\t\t" << boardToFen() << endl;

    cout << "side:\t\t" << (sideToMove ? "White" : "Black") << endl;
    cout << "castle:\t\t";
    if (rightCastle & RIGHT_KING_CASTLE_WHITE_MASK) cout << whiteRookKingSideCastle;
    if (rightCastle & RIGHT_QUEEN_CASTLE_WHITE_MASK) cout << whiteRookQueenSideCastle;
    if (rightCastle & RIGHT_KING_CASTLE_BLACK_MASK) cout << blackRookKingSideCastle;
    if (rightCastle & RIGHT_QUEEN_CASTLE_BLACK_MASK) cout << blackRookQueenSideCastle;
    cout << endl;

    cout << "ep:\t\t\t"
         << (enPassant == NO_ENPASSANT ? "" : (sideToMove ? BOARD[enPassant + 8] : BOARD[enPassant -
                                                                                         8])) << endl;
    cout << "Chess960:\t" << (chess960 ? "true" : "false") << endl;
    DEBUG(cout << "zobristKey:\t0x" << hex << chessboard[ZOBRISTKEY_IDX] << "ull" << dec << endl)
    cout << endl;
}

void ChessBoard::print(const _Tmove *move) {
    cout << decodeBoardinv(move, move->side) << " " << flush;
}

string
ChessBoard::decodeBoardinv(const _Tmove *move, const uchar side, const bool verbose) {
    const uchar type = move->type;
    const int from = move->from;
    const int to = move->to;
    const int promotionPiece = move->promotionPiece;
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
    assert(!(type & 0xC));
    assert (to >= 0 && to < 64);
    string cap = "";
    string res = "";
    if (verbose) {
        if (move->capturedPiece != SQUARE_EMPTY) cap = "*"; else cap = "-";
    }
    res = BOARD[from] + cap + BOARD[to];
    if (promotionPiece != NO_PROMOTION) {
        res += tolower(FEN_PIECE[promotionPiece]);
    }
    return res;
}

void ChessBoard::clearChessboard() {
    memset(chessboard, 0, sizeof(_Tchessboard));
    enPassant = NO_ENPASSANT;
    rightCastle = 0;
    sideToMove = 0;
}

int ChessBoard::loadFen(const string &fen) {
    if (fen.empty()) {
        return loadFen();
    }
    startPosWhiteKing = NO_POSITION;
    startPosWhiteRookKingSide = NO_POSITION;
    startPosWhiteRookQueenSide = NO_POSITION;

    startPosBlackKing = NO_POSITION;
    startPosBlackRookKingSide = NO_POSITION;
    startPosBlackRookQueenSide = NO_POSITION;
    MATCH_QUEENSIDE = "";
    MATCH_QUEENSIDE_WHITE = "O-O-O ";
    MATCH_KINGSIDE_WHITE = "O-O ";
    MATCH_QUEENSIDE_BLACK = "O-O-O ";
    MATCH_KINGSIDE_BLACK = "O-O ";
    fenString = fen;
    clearChessboard();
    istringstream iss(fen);
    string pos, castle, enpassant, side, a1, a2;
    iss >> pos;
    iss >> side;
    iss >> castle;
    iss >> enpassant;
    iss >> a1;
    iss >> a2;
    a2 += " 1";
    if (String::isNumber(a2))
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
            }
        }
    }
    if (ix != 64) {
        return 2;
    }
    if (side == "b") {
        sideToMove = BLACK;
    } else if (side == "w") {
        sideToMove = WHITE;
    } else {
        return 2;
    }

    for (int i = 0; i < 64; i++) {
        int p = s[63 - i];
        if (p != SQUARE_EMPTY) {
            updateZobristKey(p, i);
            chessboard[p] |= POW2(i);
        } else {
            chessboard[p] &= NOTPOW2(i);
        }
    }
    startPosWhiteKing = BITScanForward(chessboard[KING_WHITE]);
    startPosBlackKing = BITScanForward(chessboard[KING_BLACK]);
    auto whiteRookKingSide = [&](const char c) {
        startPosWhiteRookKingSide = BITScanForward(chessboard[ROOK_WHITE] & 0xffULL);
        updateZobristKey(RIGHT_CASTLE_RAND, 4);
        assert(4 == BITScanForward(RIGHT_KING_CASTLE_WHITE_MASK));
        rightCastle |= RIGHT_KING_CASTLE_WHITE_MASK;
        whiteRookKingSideCastle = c;
    };
    auto blackRookKingSide = [&](const char c) {
        startPosBlackRookKingSide = BITScanForward(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
        updateZobristKey(RIGHT_CASTLE_RAND, 6);
        assert(6 == BITScanForward(RIGHT_KING_CASTLE_BLACK_MASK));
        rightCastle |= RIGHT_KING_CASTLE_BLACK_MASK;
        blackRookKingSideCastle = c;
    };
    auto whiteRookQueenSide = [&](const char c) {
        startPosWhiteRookQueenSide = BITScanReverse(chessboard[ROOK_WHITE] & 0xffULL);
        updateZobristKey(RIGHT_CASTLE_RAND, 5);
        assert(5 == BITScanForward(RIGHT_QUEEN_CASTLE_WHITE_MASK));
        rightCastle |= RIGHT_QUEEN_CASTLE_WHITE_MASK;
        whiteRookQueenSideCastle = c;
    };
    auto blackRookQueenSide = [&](const char c) {
        startPosBlackRookQueenSide = BITScanReverse(chessboard[ROOK_BLACK] & 0xff00000000000000ULL);
        updateZobristKey(RIGHT_CASTLE_RAND, 7);
        assert(7 == BITScanForward(RIGHT_QUEEN_CASTLE_BLACK_MASK));
        rightCastle |= RIGHT_QUEEN_CASTLE_BLACK_MASK;
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
    enPassant = NO_ENPASSANT;
    for (int i = 0; i < 64; i++) {
        if (enpassant == BOARD[i]) {
            enPassant = i;
            if (sideToMove) {
                enPassant -= 8;
            } else {
                enPassant += 8;
            }
            updateZobristKey(ENPASSANT_RAND, enPassant);
            break;
        }
    }

    if (isChess960()) {
        if (startPosWhiteRookKingSide != NO_POSITION)
            MATCH_KINGSIDE_WHITE += BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookKingSide];
        if (startPosBlackRookQueenSide != NO_POSITION)
            MATCH_QUEENSIDE_BLACK += BOARD[startPosBlackKing] + BOARD[startPosBlackRookQueenSide];
        if (startPosWhiteRookQueenSide != NO_POSITION)
            MATCH_QUEENSIDE_WHITE += BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookQueenSide];
        if (startPosBlackRookKingSide != NO_POSITION)
            MATCH_KINGSIDE_BLACK += BOARD[startPosBlackKing] + BOARD[startPosBlackRookKingSide];
        if (startPosBlackRookQueenSide != NO_POSITION)
            MATCH_QUEENSIDE +=
                    MATCH_QUEENSIDE_BLACK + " " + BOARD[startPosBlackKing] + BOARD[startPosBlackRookQueenSide];
        if (startPosWhiteRookQueenSide != NO_POSITION)
            MATCH_QUEENSIDE +=
                    MATCH_QUEENSIDE_WHITE + " " + BOARD[startPosWhiteKing] + BOARD[startPosWhiteRookQueenSide];
    } else {
        MATCH_QUEENSIDE_WHITE += "e1c1";
        MATCH_KINGSIDE_WHITE += "e1g1";
        MATCH_QUEENSIDE_BLACK += "e8c8";
        MATCH_KINGSIDE_BLACK += "e8g8";
        MATCH_QUEENSIDE = MATCH_QUEENSIDE_WHITE + " e8c8";
    }

    return sideToMove;
}
