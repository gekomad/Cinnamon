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
#include "tablebase.cpp"
#include "eval.cpp"
#include "spinlockShared.cpp"
#include "spinlock.cpp"
#include "search.cpp"
#include "util/fileUtil.cpp"
#include "util/string.cpp"
#include "perft.cpp"

#endif