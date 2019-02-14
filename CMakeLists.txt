cmake_minimum_required(VERSION 3.5)
project(cinnamon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -D_FORTIFY_SOURCE=2  -pthread -pedantic -std=c++11 -DDEBUG_MODE -DFULL_TESTx")

set(SOURCE_FILES
        src/lib/gtb-probe.h
        src/namespaces/board.h
        src/namespaces/debug.h
        src/namespaces/def.h
        src/namespaces/random.h
        src/perft/_TPerftRes.h
        src/perft/Perft.cpp
        src/perft/Perft.h
        src/perft/PerftThread.cpp
        src/perft/PerftThread.h
        src/test/test.cpp
        src/threadPool/Mutex.h
        src/threadPool/ObserverThread.h
        src/threadPool/Spinlock.h
        src/threadPool/Thread.h
        src/threadPool/ThreadPool.h
        src/util/Bitboard.cpp
        src/util/Bitboard.h
        src/util/FileUtil.h
        src/util/GetOpt.h
        src/util/IniFile.cpp
        src/util/IniFile.h
        src/util/logger.h
        src/util/Random.h
        src/util/Singleton.h
        src/util/String.cpp
        src/util/String.h
        src/util/Time.h
        src/util/Timer.cpp
        src/util/Timer.h
        src/ChessBoard.cpp
        src/ChessBoard.h
        src/Eval.cpp
        src/Eval.h
        src/GenMoves.cpp
        src/GenMoves.h
        src/Hash.cpp
        src/Hash.h
        src/IterativeDeeping.cpp
        src/IterativeDeeping.h
        src/main.cpp
        src/OpenBook.cpp
        src/OpenBook.h
        src/Search.cpp
        src/Search.h
        src/SearchManager.cpp
        src/SearchManager.h
        src/Tablebase.cpp
        src/Tablebase.h
        src/db/GTB.cpp
        src/db/syzygy/SYZYGY.cpp
        src/db/syzygy/tbprobe.cpp
        src/db/syzygy/main.cpp
        src/Uci.cpp
        src/Uci.h
        )

add_executable(cinnamon ${SOURCE_FILES})

target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/src/lib/Linux/64/libgtb.a /usr/local/lib/libgtest.a)
