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
tuple <u64, u64, u64, u64> target;

void doFail() {

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

void doOK( Spinlock *spinlock) {

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

TEST(spinlockTest_test1_Test, testOK) {
    Spinlock *spinlock = new Spinlock();
    thread t1([=]() {
        doOK(spinlock);
        return 1;
    });
    thread t2([=]() {
        doOK(spinlock);
        return 1;
    });
    thread t3([=]() {
        doOK(spinlock);
        return 1;
    });
    thread t4([=]() {
        doOK(spinlock);
        return 1;
    });
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    EXPECT_TRUE(get<0>(target) == get<1>(target) );
    EXPECT_TRUE(get<0>(target) == get<2>(target) );
    EXPECT_TRUE(get<0>(target) == get<3>(target) );

    delete spinlock;
}


TEST(spinlockTest_test1_Test, testKO) {
    thread t1([=]() {
        doFail();
        return 1;
    });
    thread t2([=]() {
        doFail();
        return 1;
    });
    thread t3([=]() {
        doFail();
        return 1;
    });
    thread t4([=]() {
        doFail();
        return 1;
    });
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    EXPECT_FALSE(get<0>(target) == get<1>(target) == get<2>(target) == get<3>(target));
}
#endif
