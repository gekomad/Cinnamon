#pragma once
#ifdef DEBUG_MODE
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
        nanoseconds ms = duration_cast<nanoseconds>(system_clock::now().time_since_epoch());
        lock_guard<mutex> lock1(_CoutSyncMutex);
        cout << "TIME:" << ms.count() << " " << s.str() << "\n";
    }
};
#endif