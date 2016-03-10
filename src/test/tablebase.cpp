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
#include "../IterativeDeeping.h"
#include "../Tablebase.h"

TEST(tablebase, test1) {
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    Tablebase &tablebase = searchManager.createGtb();
    if (!tablebase.setPath("/gtb4")) {
        FAIL() << "path error";
    }

    IterativeDeeping it;

    if (!searchManager.getGtb().setScheme("cp4")) {
        FAIL() << "set scheme error";
    }
    if (!searchManager.getGtb().setInstalledPieces(4)) {
        FAIL() << "set installed pieces error";
    }
    if (!it.getGtbAvailable()) {
        FAIL() << "error TB not found";
    }
    searchManager.loadFen("8/8/8/8/6p1/7p/4kB2/6K1 w - -");
    EXPECT_EQ(0, searchManager.printDtm());
    searchManager.deleteGtb();
}

#endif