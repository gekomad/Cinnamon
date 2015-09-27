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

#pragma once

#include "ChessBoard.h"
#include <map>
#include "namespaces/eval.h"

using namespace _eval;

class Endgame : public virtual ChessBoard {

public:

    Endgame();

    virtual ~Endgame();

    int getEndgameValue(const int N_PIECE, const int side);

private:
    int KRKP(int loserSide, int tempo, int winnerKingPos, int loserKingPos, int rookPos, int pawnPos);

    int KQKP(int side, int winnerKingPos, int loserKingPos, int pawnPos);

    int KBBKN(int winnerKingPos, int loserKingPos, int knightPos);

    int KQKR(int winnerKingPos, int loserKingPos);

    int KBNK(int winnerKingPos, int loserKingPos);

    int KRKB(int loserKingPos);

    int KRKN(int loserKingPos, int knightPos);

};

