cmake_minimum_required(VERSION 3.17)
project(cinnamon_debug)

# debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv -fwrapv -fsanitize=address -mbmi2 -DUSE_BMI2 -g -DDEBUG_MODE -pthread -Wall -Wpedantic -W -std=c++11 -DDLOG_LEVEL=_TRACE -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# release
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -pthread -Wpedantic -W -std=c++11 -DDLOG_LEVEL=_FATAL -Wall -O3 -DNDEBUG -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# bench
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -pthread -Wpedantic -W -DBENCH_MODE -std=c++11 -DDLOG_LEVEL=_FATAL -Wall -O3 -DNDEBUG -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# google test
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O3 -pthread -Wall -Wpedantic -W -std=c++14 -DDLOG_LEVEL=_TRACE -DFULL_TEST -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# tuning texel
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -O3 -pthread -Wall -Wpedantic -W -std=c++11 -DTUNING -DNDEBUG -DDLOG_LEVEL=_FATAL -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# valgrind
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pthread -std=c++11 -DDLOG_LEVEL=_FATAL -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -fno-inline -fno-omit-frame-pointer ")
include_directories(db/gtb)
include_directories(db/gtb/compression)
include_directories(db/gtb/compression/lzma)
include_directories(db/gtb/sysport)

set(SOURCE_FILES
        namespaces/board.cpp
        perft/Perft.cpp
        db/syzygy/SYZYGY.cpp
        db/syzygy/tbprobe.c
        db/gaviota/GTB.cpp
        db/gaviota/gtb/compression/lzma/Alloc.c
        db/gaviota/gtb/compression/lzma/Bra86.c
        db/gaviota/gtb/compression/lzma/LzFind.c
        db/gaviota/gtb/compression/lzma/Lzma86Dec.c
        db/gaviota/gtb/compression/lzma/Lzma86Enc.c
        db/gaviota/gtb/compression/lzma/LzmaDec.c
        db/gaviota/gtb/compression/lzma/LzmaEnc.c
        db/gaviota/gtb/compression/wrap.c
        db/gaviota/gtb/sysport/sysport.c
        db/gaviota/gtb/gtb-att.c
        db/gaviota/gtb/gtb-dec.c
        db/gaviota/gtb/gtb-probe.c
        perft/PerftThread.cpp
        test/test.cpp
        util/Bitboard.cpp
        util/IniFile.cpp
        util/Timer.cpp
        ChessBoard.cpp
        Eval.cpp
        GenMoves.cpp
        Hash.cpp
        IterativeDeeping.cpp
        main.cpp
        Search.cpp
        SearchManager.cpp
        Uci.cpp
util/tuning/Tune.h
)
  

set(CMAKE_CXX_COMPILER "clang++")
add_executable(cinnamon_debug ${SOURCE_FILES})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
#target_link_libraries(cinnamon_debug /usr/local/lib/libgtest.a)
