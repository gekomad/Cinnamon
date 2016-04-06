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
#include "../../util/String.h"

TEST(StringTest, trim) {
    String s(" hello ");
    ASSERT_EQ("hello", s.trim());
}

TEST(StringTest, endsWith) {
    String s("hello");
    ASSERT_TRUE(s.endsWith("lo"));
}

TEST(StringTest, trimLeft) {
    String s(" hello ");
    ASSERT_EQ("hello ", s.trimLeft());
}

TEST(StringTest, trimRight) {
    String s(" hello ");
    ASSERT_EQ(" hello", s.trimRight());
}


TEST(StringTest, replace) {
    String s(" hello ");
    ASSERT_EQ(" hexxo ", s.replace("l", "x"));
}

TEST(StringTest, replaceChar) {
    String s(" hello ");
    ASSERT_EQ(" hexxo ", s.replace('l', 'x'));
}

TEST(StringTest, toLower) {
    String s("HELLO");
    ASSERT_EQ("hello", s.toLower());
}

#endif