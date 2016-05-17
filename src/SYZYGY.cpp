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

#ifdef JS_MODE
bool SYZYGY::getAvailable() const{return false;}

string SYZYGY::getPath() const{return "";}

bool SYZYGY::setPath(const string &path){return false;}

void SYZYGY::restart(){}

int SYZYGY::getDtm() const{return INT_MAX;}

string SYZYGY::getBestmove(){return false;}
#else

SYZYGY::SYZYGY() {

}

SYZYGY::~SYZYGY() {

}

bool SYZYGY::getAvailable() const { return false; }

string SYZYGY::getPath() const { return ""; }

bool SYZYGY::setPath(const string &path) {
    SYZYGY::path = path;
    tb_init_syzygy(path.c_str());
    return TB_LARGEST;
}

void SYZYGY::restart() { }

int SYZYGY::getDtm(const _Tchessboard &c, const bool turn) {

    int res = search(c, turn);
    if (res != TB_RESULT_FAILED) {
        return TB_GET_DTZ(res);
    }
    return INT_MAX;
}

string SYZYGY::getBestmove(const _Tchessboard &c, const bool turn) {
    int res = search(c, turn);

    printf("[DrawingMoves \"");
    bool prev = false;
    prev = print_moves(results, prev, TB_CURSED_WIN);
    fflush(stdout);
    prev = print_moves(results, prev, TB_DRAW);
    fflush(stdout);
    prev = print_moves(results, prev, TB_BLESSED_LOSS);
    fflush(stdout);
    printf("\"]\n");
    printf("[LosingMoves \"");
    prev = false;
    print_moves(results, prev, TB_LOSS);
    printf("\"]\n");

    return "";
}

int SYZYGY::search(const _Tchessboard &c, const bool turn) {
    u64 white = c.getBitmap<WHITE>();
    u64 black = c.getBitmap<BLACK>();
    u64 kings = c.bit[KING_BLACK] | c.bit[KING_WHITE];
    u64 queens = c.bit[QUEEN_BLACK] | c.bit[QUEEN_WHITE];
    u64 rooks = c.bit[ROOK_BLACK] | c.bit[ROOK_WHITE];
    u64 bishops = c.bit[BISHOP_BLACK] | c.bit[BISHOP_WHITE];
    u64 knights = c.bit[KNIGHT_BLACK] | c.bit[KNIGHT_WHITE];
    u64 pawns = c.bit[PAWN_BLACK] | c.bit[PAWN_WHITE];
    unsigned rule50 = 0;//TODO
    unsigned castling = 0;//TODO
    u64 ep = 0;//TODO

    return tb_probe_root(white, black, kings,
                         queens, rooks, bishops, knights, pawns,
                         rule50, castling, ep, turn, results);
}

bool SYZYGY::print_moves(unsigned *results, bool prev,
                         unsigned wdl) {
    for (unsigned i = 0; results[i] != TB_RESULT_FAILED; i++) {
        if (TB_GET_WDL(results[i]) != wdl)
            continue;
        if (prev)
            printf(", ");
        prev = true;
        char str[32];
        move_to_str(results[i], str);
        printf("%s", str);
    }
    fflush(stdout);
    return prev;
}

void SYZYGY::move_to_str(unsigned move, char *str) {
//    uint64_t occ = pos->black | pos->white;
//    uint64_t us = (pos->turn ? pos->white : pos->black);
    unsigned from = TB_GET_FROM(move);
    unsigned to = TB_GET_TO(move);
    unsigned r = rank(from);
    unsigned f = file(from);

    unsigned r1 = rank(from);
    unsigned f1 = file(from);

    cout << "from: " << r << " " << f << " to: " << r1 << " " << f1 << endl;
//    unsigned promotes = TB_GET_PROMOTES(move);
//    bool capture = (occ & board(to)) != 0 || (TB_GET_EP(move) != 0);
//    uint64_t b = board(from), att = 0;
//    if (b & pos->kings)
//        *str++ = 'K';
//    else if (b & pos->queens) {
//        *str++ = 'Q';
//        att = tb_queen_attacks(to, occ) & us & pos->queens;
//    }
//    else if (b & pos->rooks) {
//        *str++ = 'R';
//        att = tb_rook_attacks(to, occ) & us & pos->rooks;
//    }
//    else if (b & pos->bishops) {
//        *str++ = 'B';
//        att = tb_bishop_attacks(to, occ) & us & pos->bishops;
//    }
//    else if (b & pos->knights) {
//        *str++ = 'N';
//        att = tb_knight_attacks(to) & us & pos->knights;
//    }
//    else {
//        if (capture)
//            *str++ = 'a' + f;
//    }
//    if (tb_pop_count(att) > 1) {
//        if (tb_pop_count(att & (BOARD_FILE_A >> f)) <= 1)
//            *str++ = 'a' + f;
//        else if (tb_pop_count(att & (BOARD_RANK_1 >> r)) <= 1)
//            *str++ = '1' + r;
//        else {
//            *str++ = 'a' + f;
//            *str++ = '1' + r;
//        }
//    }
//    if (capture)
//        *str++ = 'x';
//    *str++ = 'a' + _file(to);
//    *str++ = '1' + rank(to);
//    if (promotes != TB_PROMOTES_NONE) {
//        *str++ = '=';
//        switch (promotes) {
//            case TB_PROMOTES_QUEEN:
//                *str++ = 'Q';
//                break;
//            case TB_PROMOTES_ROOK:
//                *str++ = 'R';
//                break;
//            case TB_PROMOTES_BISHOP:
//                *str++ = 'B';
//                break;
//            case TB_PROMOTES_KNIGHT:
//                *str++ = 'N';
//                break;
//        }
//    }
//    *str++ = '\0';
}

#endif
