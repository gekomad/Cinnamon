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

#if defined(DEBUG_MODE) || defined(FULL_TEST)

#include <gtest/gtest.h>
#include <set>
#include "../IterativeDeeping.h"

TEST(pin, test1) {
    IterativeDeeping it;
    it.loadFen("rnK3nk/ppB2pp1/1bp1P3/b4q2/6p1/2P4b/PP2PP1P/RN3BNR w - - 0 1");
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    u64 friends = searchManager.getBitmap(WHITE);
    u64 enemies = searchManager.getBitmap(BLACK);
    u64 p = searchManager.getPin<WHITE>(enemies, friends);
    EXPECT_EQ(0x80000000000ULL, p);

    it.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    friends = searchManager.getBitmap(WHITE);
    enemies = searchManager.getBitmap(BLACK);
    p = searchManager.getPin<WHITE>(enemies, friends);
    EXPECT_EQ(0, p);

}

#endif