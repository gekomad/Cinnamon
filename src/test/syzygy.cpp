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

#if !defined(FULL_TEST)

#include <gtest/gtest.h>
#include <set>
#include "../SearchManager.h"
#include "../IterativeDeeping.h"
#include "../SYZYGY.h"

TEST(syzygy, bestmove) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY &tablebase = searchManager.createSYZYGY();
    if (!tablebase.setPath("/syzygy")) {
        FAIL() << "path error";
    }

    IterativeDeeping it;

    searchManager.loadFen("K7/B7/B7/8/8/8/7n/7k b - - 5 1");

    EXPECT_EQ("Kg2", searchManager.getSYZYGYbestmove(BLACK));

}

TEST(syzygy, dtm) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    SYZYGY &tablebase = searchManager.createSYZYGY();
    if (!tablebase.setPath("/syzygy")) {
        FAIL() << "path error";
    }

    IterativeDeeping it;

    searchManager.loadFen("K7/B7/B7/8/8/8/7n/7k b - - 5 1");

    EXPECT_EQ(110, searchManager.getSYZYGYdtm(BLACK));

}

#endif