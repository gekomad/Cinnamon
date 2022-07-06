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
#include <set>
#include "../SearchManager.h"

TEST(syzygy, DTM) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY::getInstance().createSYZYGY("/syzygy");
    searchManager.loadFen("2QN4/8/8/8/8/8/8/1k2K3 w - - 0 1");
    EXPECT_EQ("e1d2", TB::probeRootTB1(searchManager.getSearch()));
}

TEST(syzygy, WDL1) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY::getInstance().createSYZYGY("/syzygy");
    searchManager.loadFen("2QN4/8/8/8/8/8/8/1k2K3 w - - 0 1");
    EXPECT_EQ(4, searchManager.SZtbProbeWDL()); //TB_WIN
}


TEST(syzygy, WDL2) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY::getInstance().createSYZYGY("/syzygy");
    searchManager.loadFen("2QN4/8/8/8/8/8/8/1k2K3 b - - 0 1");
    auto d = searchManager.SZtbProbeWDL();
    EXPECT_EQ(0, d); //TB_LOSS
}

TEST(syzygy, WDL3) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY::getInstance().createSYZYGY("/syzygy");
    searchManager.loadFen("8/8/8/8/8/8/8/1k2K3 w - - 0 1");
    auto d = searchManager.SZtbProbeWDL();
    EXPECT_EQ(2, d); //TB_DRAW
}

#endif

