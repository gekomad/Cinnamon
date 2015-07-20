#pragma once

#include <iostream>

using namespace std;
using namespace std::chrono;
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
        nanoseconds ms = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());

        cout << "TIME:" << ms.count() << " " << s.str() << "\n";
    }
};