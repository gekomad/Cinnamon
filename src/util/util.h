#pragma once

#include <iostream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <string>

inline std::string getDateTime() {
    const std::time_t current_time = std::time(nullptr);
    std::tm *local_time = std::localtime(&current_time);
    assert(local_time);
    std::stringstream ss;
    ss << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline void *xmalloc(const size_t size) {
    const auto a = malloc(size);
    if (!a) {
        printf("error");
        exit(1);
    }
    return a;
}
