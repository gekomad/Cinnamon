#pragma once

#include <iostream>

using namespace std;
static mutex _CoutSyncMutex;

struct CoutSync {
    stringstream s;

    template<typename T>
    CoutSync &operator<<(const T &x) {
        s << x;
        return *this;
    }

    ~CoutSync() {
        lock_guard<mutex> lock1(_CoutSyncMutex);
        cout << "time:" << time(0) << " " << s.str() << "\n";
    }
};