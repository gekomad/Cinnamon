cmake_minimum_required(VERSION 3.16)
project(cinnamon_debug)

# debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftrapv -fwrapv -fsanitize=address -mbmi2 -DUSE_BMI2 -g -pthread -Wall -Wpedantic -W -std=c++11 -DDLOG_LEVEL=_TRACE -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# bench
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -pthread -Wpedantic -W -DBENCH_MODE -std=c++11 -DDLOG_LEVEL=_FATAL -Wall -O3 -DNDEBUG -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# release
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -pthread -Wpedantic -W -std=c++11 -DDLOG_LEVEL=_FATAL -Wall -O3 -DNDEBUG -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# unit test
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O3 -pthread -Wall -Wpedantic -W -std=c++11 -DDLOG_LEVEL=_TRACE -DFULL_TEST -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

# tuning
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mbmi2 -DUSE_BMI2 -O3 -pthread -Wall -Wpedantic -W -std=c++11 -DTUNING -DNDEBUG -DDLOG_LEVEL=_FATAL -funroll-loops -msse4.2 -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF -m64 ")

include_directories(db/gtb)
include_directories(db/gtb/compression)
include_directories(db/gtb/compression/lzma)
include_directories(db/gtb/sysport)

set(SOURCE_FILES
        namespaces/constants.h
        namespaces/bits.h
        namespaces/random.h
        namespaces/see.h
        namespaces/board.h
        namespaces/board.cpp
        perft/_TPerftRes.h
        perft/Perft.cpp
        perft/Perft.h
        db/syzygy/SYZYGY.h
        db/syzygy/SYZYGY.cpp
        db/syzygy/tbprobe.c
        db/gaviota/GTB.h
        db/gaviota/GTB.cpp
        db/gaviota/gtb/compression/lzma/Alloc.c
        db/gaviota/gtb/compression/lzma/Alloc.h
        db/gaviota/gtb/compression/lzma/Bra.h
        db/gaviota/gtb/compression/lzma/Bra86.c
        db/gaviota/gtb/compression/lzma/LzFind.c
        db/gaviota/gtb/compression/lzma/LzFind.h
        db/gaviota/gtb/compression/lzma/LzFindMt.h
        db/gaviota/gtb/compression/lzma/LzHash.h
        db/gaviota/gtb/compression/lzma/Lzma86Dec.c
        db/gaviota/gtb/compression/lzma/Lzma86Dec.h
        db/gaviota/gtb/compression/lzma/Lzma86Enc.c
        db/gaviota/gtb/compression/lzma/Lzma86Enc.h
        db/gaviota/gtb/compression/lzma/LzmaDec.c
        db/gaviota/gtb/compression/lzma/LzmaDec.h
        db/gaviota/gtb/compression/lzma/LzmaEnc.c
        db/gaviota/gtb/compression/lzma/LzmaEnc.h
        db/gaviota/gtb/compression/lzma/Types.h
        db/gaviota/gtb/compression/wrap.c
        db/gaviota/gtb/compression/wrap.h
        db/gaviota/gtb/sysport/sysport.c
        db/gaviota/gtb/sysport/sysport.h
        db/gaviota/gtb/gtb-att.c
        db/gaviota/gtb/gtb-att.h
        db/gaviota/gtb/gtb-dec.c
        db/gaviota/gtb/gtb-dec.h
        db/gaviota/gtb/gtb-probe.c
        db/gaviota/gtb/gtb-probe.h
        db/gaviota/gtb/gtb-types.h
        db/gaviota/gtb/version.h
        perft/PerftThread.cpp
        perft/PerftThread.h
        test/test.cpp
        threadPool/ObserverThread.h
        threadPool/Spinlock.h
        threadPool/Thread.h
        threadPool/ThreadPool.h
        util/Bitboard.cpp
        util/Bitboard.h
        util/FileUtil.h
        GetOpt.h
        util/IniFile.cpp
        util/IniFile.h
        util/logger.h
        util/Singleton.h
        util/bench/Time.h
        util/Timer.cpp
        util/Timer.h
        util/getopt.h
        ChessBoard.cpp
        ChessBoard.h
        Eval.cpp
        Eval.h
        GenMoves.cpp
        GenMoves.h
        Hash.cpp
        Hash.h
        IterativeDeeping.cpp
        IterativeDeeping.h
        main.cpp
        Search.cpp
        Search.h
        SearchManager.cpp
        SearchManager.h
        Uci.cpp
        Uci.h
        unistd.h
        db/TB.h)

set(CMAKE_CXX_COMPILER "clang++")
add_executable(cinnamon_debug ${SOURCE_FILES})
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG")
target_link_libraries(cinnamon_debug /usr/lib/libgtest.a)
