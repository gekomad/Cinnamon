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

string board::boardToFen(const _Tchessboard &chessboard) {
    string fen;
    for (int y = 0; y < 8; y++) {
        int l = 0;
        string row;
        for (int x = 0; x < 8; x++) {
            int q = getPieceAt<BLACK>(POW2[63 - ((y * 8) + x)], chessboard);
            if (q == SQUARE_EMPTY) {
                q = getPieceAt<WHITE>(POW2[63 - ((y * 8) + x)], chessboard);
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
        chessboard[SIDETOMOVE_IDX] ? fen.append(BOARD[chessboard[ENPASSANT_IDX] + 8]) : fen.append(
                BOARD[chessboard[ENPASSANT_IDX] - 8]);
    }
    fen.append(" 0 1");
    return fen;
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

void board::print(const _Tmove *move, const _Tchessboard &chessboard) {
    cout << moveToString(move) << " " << flush;
}

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

void board::display(const _Tchessboard &chessboard) {
    cout << endl << "     a   b   c   d   e   f   g   h";
    for (int t = 0; t <= 63; t++) {
        char x = ' ';
        if (t % 8 == 0) {
            cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
            cout << " " << 8 - RANK_AT[t] << " | ";
        }
        x = (x = (x = FEN_PIECE[getPieceAt<WHITE>(POW2[63 - t], chessboard)]) != '-' ? x : FEN_PIECE[getPieceAt<BLACK>(
                POW2[63 - t], chessboard)]) == '-' ? ' ' : x;
        x != ' ' ? cout << x : POW2[t] & WHITE_SQUARES ? cout << " " : cout << ".";
        cout << " | ";
    };
    cout << endl << "   ----+---+---+---+---+---+---+----" << endl;
    cout << "     a   b   c   d   e   f   g   h" << endl << endl << "fen:\t\t" << boardToFen(chessboard) << endl;

    cout << "side:\t\t" << (chessboard[SIDETOMOVE_IDX] ? "White" : "Black") << endl;
    cout << "castle:\t\t";
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_WHITE_MASK) cout << "K";
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_WHITE_MASK) cout << "Q";
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_KING_CASTLE_BLACK_MASK) cout << "k";
    if (chessboard[RIGHT_CASTLE_IDX] & RIGHT_QUEEN_CASTLE_BLACK_MASK) cout << "q";
    cout << endl;

    cout << "ep:\t\t"
         << (chessboard[ENPASSANT_IDX] == 100 ? "" : (chessboard[SIDETOMOVE_IDX] ? BOARD[chessboard[ENPASSANT_IDX] + 8]
                                                                                 : BOARD[chessboard[ENPASSANT_IDX] -
                                                                                         8])) << endl;
#ifdef DEBUG_MODE
    cout << "zobristKey:\t0x" << hex << chessboard[ZOBRISTKEY_IDX] << "ull" << dec << endl;
#endif
    cout << endl;
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

string board::moveToString(const _Tmove *move) {
    string a = decodeBoardinv(move->s.type, move->s.from, move->s.side);
    if (move->s.type & 0xc) return a;
    string b = decodeBoardinv(move->s.type, move->s.to, move->s.side);
    if (move->s.promotionPiece != -1) (a + b) += (char) tolower(FEN_PIECE[move->s.promotionPiece]);
    return a + b;
}

string board::getCell(const int file, const int rank) {
    return BOARD[FILE_AT[file] * 8 + rank];
}

bool board::isPieceAt(const uchar pieces, const uchar pos, const _Tchessboard &chessboard) {
    return chessboard[pieces] & POW2[pos];
}

string board::decodeBoardinv(const uchar type, const int a, const int side) {
    if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
        return "e1c1";
    }
    if (type & KING_SIDE_CASTLE_MOVE_MASK && side == WHITE) {
        return "e1g1";
    }
    if (type & QUEEN_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
        return "e8c8";
    }
    if (type & KING_SIDE_CASTLE_MOVE_MASK && side == BLACK) {
        return "e8g8";
    }
    ASSERT(!(type & 0xC));
    if (a >= 0 && a < 64) {
        return BOARD[a];
    }
    _assert(0);
}