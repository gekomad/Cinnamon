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

#if defined(FULL_TEST)

#include <gtest/gtest.h>
#include "../def.h"
#include "../IterativeDeeping.h"

namespace makemove {
    string go(const string &fen) {
        IterativeDeeping it;
        it.loadFen(fen);
        SearchManager &searchManager = Singleton<SearchManager>::getInstance();
        searchManager.setMaxTimeMillsec(250);
        it.start();
        it.join();
        return it.getBestmove();
    }
}

TEST(search, stalemate1) {
    EXPECT_NE("c2f2", makemove::go("8/8/8/3K4/8/3N4/2Q5/7k w - - 9 67"));
}

TEST(search, stalemate2) { EXPECT_NE("c7d6", makemove::go("4k3/2Q5/6K1/8/5N2/8/8/8 w - - 15 67")); }

TEST(search, stalemate3) {
    EXPECT_NE("b3c2", makemove::go("8/8/8/8/3K4/1Q6/8/k7 w - - 15 222"));
}

TEST(search, stalemate4) {
    EXPECT_NE("d3f4", makemove::go("8/8/7p/5k1P/5P2/3n4/7K/4q3 b - - 1 72"));
}

TEST(search, stalemate5) {
    EXPECT_NE("f4f3", makemove::go("8/6p1/6kp/8/5p2/6q1/8/5K2 b - - 5 43"));
}

#endif