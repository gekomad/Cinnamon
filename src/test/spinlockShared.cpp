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
#include <thread>
#include "../util/Random.h"
#include "../threadPool/Spinlock.h"
#include "../def.h"

using namespace std;
tuple<u64, u64, u64, u64> sharedTarget{0, 0, 0, 0};

void sharedWriteKO() {

    for (int i = 0; i < 100; i++) {

        u64 r = Random::getRandom64();

        usleep(Random::getRandom(100, 100000));
        get<0>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<1>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<2>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<3>(sharedTarget) = r;

    }
}

void sharedWriteOK(Spinlock *spinlock) {

    for (int i = 0; i < 100; i++) {
        spinlock->lockWrite();

        u64 r = Random::getRandom64();

        usleep(Random::getRandom(100, 100000));
        get<0>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<1>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<2>(sharedTarget) = r;
        usleep(Random::getRandom(100, 100000));
        get<3>(sharedTarget) = r;

        spinlock->unlockWrite();
    }
}

void sharedReadOK(Spinlock *spinlock) {

    for (int i = 0; i < 100; i++) {
        spinlock->lockRead();

        usleep(Random::getRandom(100, 100000));

        EXPECT_TRUE(get<0>(sharedTarget) == get<1>(sharedTarget));
        EXPECT_TRUE(get<0>(sharedTarget) == get<2>(sharedTarget));
        EXPECT_TRUE(get<0>(sharedTarget) == get<3>(sharedTarget));

        spinlock->unlockRead();
    }
}

TEST(spinlockSharedTest, testOK) {

    Spinlock *spinlock = new Spinlock();

    vector<thread > threads;

    int N = 2;
    for (int i = 0; i < N; i++) {
        threads.push_back(thread([=]() {
            sharedWriteOK(spinlock);
            return 1;
        }));
        threads.push_back(thread([=]() {
            sharedReadOK(spinlock);
            return 1;
        }));
    }


    for (vector<thread>::iterator it = threads.begin() ; it != threads.end(); ++it){
            (*it).join();
    }

    EXPECT_TRUE(get<0>(sharedTarget) == get<1>(sharedTarget));
    EXPECT_TRUE(get<0>(sharedTarget) == get<2>(sharedTarget));
    EXPECT_TRUE(get<0>(sharedTarget) == get<3>(sharedTarget));

    delete spinlock;
}


TEST(spinlockSharedTest, testKO) {
    vector<thread> threads;
    int N = 4;
    for (int i = 0; i < N; i++) {
        threads.push_back(thread([=]() {
            sharedWriteKO();
            return 1;
        }));
    }
    for (vector<thread>::iterator it = threads.begin() ; it != threads.end(); ++it){
        (*it).join();
    }
    EXPECT_TRUE(get<0>(sharedTarget) != get<1>(sharedTarget) ||  get<0>(sharedTarget) == get<2>(sharedTarget)||  get<0>(sharedTarget) == get<3>(sharedTarget));
}

#endif

