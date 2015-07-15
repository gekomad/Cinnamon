cmake_minimum_required(VERSION 3.2)
project(cinnamon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -DDEBUG_MODE")

set(SOURCE_FILES
    gtb/gtb-probe.h
    util/Bits.cpp
    util/Bits.h
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
    namespaces.cpp
    namespaces.h
    OpenBook.cpp
    OpenBook.h
    Perft.cpp
    Perft.h
    Search.cpp
    Search.h
    SearchManager.cpp
    SearchManager.h
    util/stacktrace.h
    util/String.cpp
    util/String.h
    Tablebase.cpp
    Tablebase.h
    util/Thread.h
    util/ThreadPool.h
    util/Timer.h
    Uci.cpp
    util/Singleton.h
    Uci.h)

add_executable(cinnamon ${SOURCE_FILES} )
target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a)