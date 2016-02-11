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

namespace _eval {

//    static const int VALUEPAWN = 100;
//    static const int VALUEROOK = 520;
//    static const int VALUEBISHOP = 335;
//    static const int VALUEKNIGHT = 330;
//    static const int VALUEQUEEN = 980;
//    static const int VALUEKING = _INFINITE;

//    static constexpr int PIECES_VALUE[13] = {VALUEPAWN, VALUEPAWN, VALUEROOK, VALUEROOK, VALUEBISHOP, VALUEBISHOP, VALUEKNIGHT, VALUEKNIGHT, VALUEKING, VALUEKING, VALUEQUEEN, VALUEQUEEN, 0};


    static const int VALUE_KNOWN_WIN = 15000;    //TODO stockfish
    static const int penaltyKRKN[8] = {0, 10, 14, 20, 30, 42, 58, 80};    //TODO stockfish
    static const int KBNKMateTable[64] = {    //TODO stockfish
            200, 190, 180, 170, 170, 180, 190, 200, 190, 180, 170, 160, 160, 170, 180, 190, 180, 170, 155, 140, 140, 155, 170, 180, 170, 160, 140, 120, 120, 140, 160, 170, 170, 160, 140, 120, 120, 140, 160, 170, 180, 170, 155, 140, 140, 155, 170, 180, 190, 180, 170, 160, 160, 170, 180, 190, 200, 190, 180, 170, 170, 180, 190, 200};

    static const int MateTable[64] = {    //TODO stockfish
            100, 90, 80, 70, 70, 80, 90, 100, 90, 70, 60, 50, 50, 60, 70, 90, 80, 60, 40, 30, 30, 40, 60, 80, 70, 50, 30, 20, 20, 30, 50, 70, 70, 50, 30, 20, 20, 30, 50, 70, 80, 60, 40, 30, 30, 40, 60, 80, 90, 70, 60, 50, 50, 60, 70, 90, 100, 90, 80, 70, 70, 80, 90, 100,};

#include "move_ordering.inc"

    static const int DistanceBonus[8] = {0, 0, 100, 80, 60, 40, 20, 10};    //TODO stockfish
}
