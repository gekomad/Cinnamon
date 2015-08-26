project(cinnamon)

set(DR_MEMORY "YESx")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++1y -Wno-reorder -DDEBUG_MODE")

if (DR_MEMORY STREQUAL "YES")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -g -fno-inline -fno-omit-frame-pointer")
endif ()

set(SOURCE_FILES
        gtb/gtb-probe.h
        perft/_TPerftRes.h
        perft/Perft.cpp
        perft/Perft.h
        perft/PerftThread.cpp
        perft/PerftThread.h
        util/Bits.cpp
        util/Bits.h
        util/ObserverSearch.h
        threadPool/ObserverThread.h
        util/Singleton.h
        util/stacktrace.h
        util/String.cpp
        util/String.h
        threadPool/Thread.h
        threadPool/ThreadPool.h
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
        namespaces.h
        OpenBook.cpp
        OpenBook.h
        Search.cpp
        Search.h
        SearchManager.cpp
        SearchManager.h
        Tablebase.cpp
        Tablebase.h
        Uci.cpp
        Uci.h)

add_executable(cinnamon ${SOURCE_FILES})
if (DR_MEMORY STREQUAL "YES")
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/32/libgtb.a)
else()
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a)
endif ()