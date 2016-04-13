project(cinnamon)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast -pthread -pedantic -fno-exceptions -fno-rtti -funroll-loops -std=c++11 -DDLOG_LEVEL=_TRACE -DDEBUG_MODE1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -pedantic -std=c++11 -DDLOG_LEVEL=_TRACE -DDEBUG_MODE")

set(SOURCE_FILES
        gtb/gtb-probe.h
        perft/_TPerftRes.h
        perft/Perft.cpp
        perft/Perft.h
        perft/PerftThread.cpp
        perft/PerftThread.h
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
        util/Bitboard.cpp util/Bitboard.h)

set(CMAKE_CXX_COMPILER "g++")
add_executable(cinnamon ${SOURCE_FILES})
target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a /usr/local/lib/libgtest.a)
