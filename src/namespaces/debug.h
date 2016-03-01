

#pragma once


namespace _debug {

#if defined(_WIN32) || !defined(DEBUG_MODE)

    static inline void print_stacktrace() { }

#else

#include <execinfo.h>

    /// (c) 2008, Timo Bingmann from http://idlebox.net/
    /// published under the WTFPL v2.0
    //  Print a demangled stack backtrace of the caller function to FILE* out.
        static inline void print_stacktrace(FILE *out = stdout, unsigned int max_frames = 512) {
        }

#endif

}
