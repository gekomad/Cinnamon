cmake_minimum_required(VERSION 3.2)
project(cinnamon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -DDEBUG_MODEx")

set(SOURCE_FILES
    gtb/gtb-probe.h
    Bits.cpp
    Bits.h
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
    stacktrace.h
    String.cpp
    String.h
    Tablebase.cpp
    Tablebase.h
    Thread.h
    Timer.h
    Uci.cpp
    Uci.h)

add_executable(cinnamon ${SOURCE_FILES})
target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a)