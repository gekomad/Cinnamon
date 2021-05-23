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
#include "../IterativeDeeping.h"
#include "../def.h"

TEST(search, test0) {
    const set<string> v = {"e3g5", "f6g5"};
    IterativeDeeping it;
    it.loadFen("8/pp6/5p1k/2P3Pp/P1P4K/4q1PP/8/6Q1 b - - 0 35");
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    searchManager.setMaxTimeMillsec(250);
    it.start();
    it.join();
    EXPECT_TRUE(v.end() != v.find(it.getBestmove()));
}

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

TEST(search, test2) {
    IterativeDeeping it;
    it.loadFen("r3n1k1/1p1b1ppp/p2rp3/4B3/q1P2P2/3B4/PP3QPP/R2R2K1 b - - 5 23 ");
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    searchManager.setMaxTimeMillsec(1000);
    it.setMaxDepth(5);
    it.start();
    it.join();
    EXPECT_NE("d6d3", it.getBestmove());
}

TEST(search, fourCore) {
    const set<string> v = {"c2c3", "d2d3", "d2d4", "e2e4", "e2e3", "b1c3", "g1f3", "g2g3", "g2g4", "f2f4"};
    IterativeDeeping it;
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    searchManager.setMaxTimeMillsec(250);
    searchManager.setNthread(4);
    it.setMaxDepth(MAX_PLY);
    it.start();
    it.join();
    cout << "fourCore res: " << it.getBestmove() << endl;
    EXPECT_TRUE(v.end() != v.find(it.getBestmove()));
}

TEST(search, ep) {
    IterativeDeeping it;
    it.loadFen("7r/1pk5/p3p1p1/2PqPp2/1P3PKb/P3Q2P/5B1P/2R5 w - f6 0 40");
    it.start();
    it.join();
    EXPECT_EQ("e5f6", it.getBestmove());
}

TEST(search, enpassant) {
    IterativeDeeping it;
    it.loadFen("4r1k1/8/8/8/3pP3/8/3Q1K2/8 b - e3 0 27");
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    searchManager.setMaxTimeMillsec(1000);
    it.setMaxDepth(5);
    it.start();
    it.join();
    EXPECT_EQ("d4e3", it.getBestmove());
}


#endif