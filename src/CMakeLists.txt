project(cinnamon)

set(DR_MEMORY "NO")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -I/usr/local/include/ -pedantic -std=c++11 -Wno-reorder -DDLOG_LEVEL=_TRACE -DDEBUG_MODE")

if (DR_MEMORY STREQUAL "YES")
    set(CMAKE_CXX_FLAGS " -pthread -std=c++11 -m32 -g -fno-inline -fno-omit-frame-pointer -Wno-reorder")
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
        ObserverSearch.h
        util/Singleton.h
        util/String.h
        threadPool/Thread.h
        threadPool/ThreadPool.h
        threadPool/ObserverThread.h
        util/Timer.h
        util/String.cpp
        util/IniFile.cpp
        util/Timer.cpp
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
        namespaces/def.h
        namespaces/board.h
        namespaces/debug.h
        namespaces/random.h
        OpenBook.cpp
        OpenBook.h
        Search.cpp
        Search.h
        SearchManager.cpp
        SearchManager.h
        Tablebase.cpp
        Tablebase.h
#        Endgame.cpp
        WrapperCinnamon.cpp
        Uci.cpp
        #google test
        test/test.cpp
        )

set(CMAKE_CXX_COMPILER "clang++")
add_executable(cinnamon ${SOURCE_FILES})

if (DR_MEMORY STREQUAL "YES")
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/32/libgtb.a)
else ()
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a /usr/local/lib/libgtest.a)
endif ()