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
#include "../namespaces/def.h"

using namespace std;
using namespace _def;
tuple<u64, u64, u64, u64> target{0, 0, 0, 0};

void writeKO() {

    for (int i = 0; i < 100; i++) {

        u64 r = Random::getRandom64();

        usleep(Random::getRandom(100, 100000));
        get<0>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<1>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<2>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<3>(target) = r;

    }
}

void writeOK(Spinlock *spinlock) {

    for (int i = 0; i < 100; i++) {
        spinlock->lock();

        u64 r = Random::getRandom64();

        usleep(Random::getRandom(100, 100000));
        get<0>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<1>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<2>(target) = r;
        usleep(Random::getRandom(100, 100000));
        get<3>(target) = r;

        spinlock->unlock();
    }
}

void readOK(Spinlock *spinlock) {

    for (int i = 0; i < 100; i++) {
        spinlock->sharedLock();

        usleep(Random::getRandom(100, 100000));

        EXPECT_TRUE(get<0>(target) == get<1>(target));
        EXPECT_TRUE(get<0>(target) == get<2>(target));
        EXPECT_TRUE(get<0>(target) == get<3>(target));

        spinlock->sharedUnlock();
    }
}

TEST(spinlockTest_test1_Test, testOK) {

    Spinlock *spinlock = new Spinlock();

    vector<thread > threads;

    int N = 4;
    for (int i = 0; i < N; i++) {
        threads.push_back(thread([=]() {
            writeOK(spinlock);
            return 1;
        }));
        threads.push_back(thread([=]() {
            readOK(spinlock);
            return 1;
        }));
    }


    for (vector<thread>::iterator it = threads.begin() ; it != threads.end(); ++it){
            (*it).join();
    }

    EXPECT_TRUE(get<0>(target) == get<1>(target));
    EXPECT_TRUE(get<0>(target) == get<2>(target));
    EXPECT_TRUE(get<0>(target) == get<3>(target));

    delete spinlock;
}


TEST(spinlockTest_test1_Test, testKO) {
    vector<thread> threads;
    int N = 4;
    for (int i = 0; i < N; i++) {
        threads.push_back(thread([=]() {
            writeKO();
            return 1;
        }));
    }
    for (vector<thread>::iterator it = threads.begin() ; it != threads.end(); ++it){
        (*it).join();
    }
    EXPECT_FALSE(get<0>(target) == get<1>(target) == get<2>(target) == get<3>(target));
}

#endif

