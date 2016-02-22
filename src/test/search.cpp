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

#ifdef DEBUG_MODE

#include <gtest/gtest.h>
#include <set>
#include "../IterativeDeeping.h"

TEST(search, test1) {
    IterativeDeeping it;
    it.loadFen("8/p5p1/k3p1p1/5pP1/5PKP/bP2r3/P7/3RB3 w - f6");
    it.start();
    it.join();
    EXPECT_EQ("g5f6", it.getBestmove());
}

TEST(search, depth1) {
    IterativeDeeping it;
    it.loadFen("rnb1r1k1/pp3ppp/2p5/3p4/B2P2n1/6qP/PPPBN3/RN1QK2R w KQ -");
    it.setMaxDepth(1);
    it.start();
    it.join();
    EXPECT_EQ("e1f1", it.getBestmove());
}

TEST(search, twoCore) {
    const set<string> v = {"d2d4", "e2e4"};
    IterativeDeeping it;
    it.setNthread(2);
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    searchManager.setMaxTimeMillsec(250);
    it.start();
    it.join();
    EXPECT_TRUE(v.end() != v.find(it.getBestmove()));
}

#endif