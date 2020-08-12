cmake_minimum_required(VERSION 3.5)
project(debug)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -pedantic -std=c++11 -DDLOG_LEVEL=_TRACE -DFULL_TEST -DDEBUG_MODE ")

set(SOURCE_FILES

        lib/gtb-probe.h
        namespaces/board.h
        namespaces/debug.h
        namespaces/def.h
        namespaces/random.h
        perft/_TPerftRes.h
        perft/Perft.cpp
        perft/Perft.h
        db/OpenBook.cpp
        db/OpenBook.h
        db/GTB.h
        db/GTB.cpp
        perft/PerftThread.cpp
        perft/PerftThread.h
        test/test.cpp
        threadPool/Mutex.h
        threadPool/ObserverThread.h
        threadPool/Spinlock.h
        threadPool/Thread.h
        threadPool/ThreadPool.h
        util/Bitboard.cpp
        util/Bitboard.h
        util/FileUtil.h
        util/GetOpt.h
        util/IniFile.cpp
        util/IniFile.h
        util/logger.h
        util/Random.h
        util/Singleton.h
        util/String.cpp
        util/String.h
        util/Time.h
        util/Timer.cpp
        util/Timer.h
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
        WrapperCinnamon.cpp
        WrapperCinnamon.h)

set(CMAKE_CXX_COMPILER "g++")
add_executable(debug ${SOURCE_FILES})
target_link_libraries(debug ${CMAKE_SOURCE_DIR}/lib/Linux/64/libgtb.a /usr/lib/x86_64-linux-gnu/libgtest.a)
