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

TEST(pin, pin1) {
    IterativeDeeping it;
    SearchManager &searchManager = Singleton<SearchManager>::getInstance();
    u64 friends, enemies, p;
    int kingPosition;

    it.loadFen("r3k2r/p1ppqpb1/Bn2pnp1/3PN3/1p2P3/2N2Q2/PPPB1PpP/R3K2R w KQkq - 0 1");
    friends = searchManager.getBitmap(0, BLACK);
    enemies = searchManager.getBitmap(0, WHITE);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_BLACK]);
    p = searchManager.getPinned<BLACK>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0, p);

    it.loadFen("rnbq1bnr/pppppkpp/8/5p2/4P2P/5Q2/PPPP1PP1/RNB1KBNR b KQ - 0 1");
    friends = searchManager.getBitmap(0, BLACK);
    enemies = searchManager.getBitmap(0, WHITE);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_BLACK]);
    p = searchManager.getPinned<BLACK>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0x400000000ULL, p);

    it.loadFen("r3k2r/p1ppqpb1/1n2pnp1/3PN3/1p2P3/2Nb1Q1p/PPPBBPPP/R4K1R w kq - 0 1");
    friends = searchManager.getBitmap(0, WHITE);
    enemies = searchManager.getBitmap(0, BLACK);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_WHITE]);
    p = searchManager.getPinned<WHITE>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0x800ULL, p);

    it.loadFen("rn4k1/pp3ppp/2B5/3p4/3P4/8/PPPBb3/RN5K w - - 0 1");
    friends = searchManager.getBitmap(0, WHITE);
    enemies = searchManager.getBitmap(0, BLACK);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_WHITE]);
    p = searchManager.getPinned<WHITE>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0, p);

    it.loadFen("rnK3nk/ppB2pp1/1bp1P3/b4q2/6p1/2P4b/PP2PP1P/RN3BNR w - - 0 1");
    friends = searchManager.getBitmap(0, WHITE);
    enemies = searchManager.getBitmap(0, BLACK);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_WHITE]);
    p = searchManager.getPinned<WHITE>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0x80000000000ULL, p);

    it.loadFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    friends = searchManager.getBitmap(0, WHITE);
    enemies = searchManager.getBitmap(0, BLACK);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_WHITE]);
    p = searchManager.getPinned<WHITE>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0, p);

    it.loadFen("r3k2r/p1pp1pb1/1n2pnp1/3PN3/1p2P3/2Nb1q1p/PPPBBPPP/R4K1R w kq - 0 1");
    friends = searchManager.getBitmap(0, WHITE);
    enemies = searchManager.getBitmap(0, BLACK);
    kingPosition = BITScanForward(searchManager.getChessboard()[KING_WHITE]);
    p = searchManager.getPinned<WHITE>(enemies | friends, friends, kingPosition);
    EXPECT_EQ(0xc00ULL, p);

}

#endif
