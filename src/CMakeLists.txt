project(cinnamon)

set(DR_MEMORY "NO")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -pthread -std=c++11 -Wno-reorder -DDEBUG_MODE")

if (DR_MEMORY STREQUAL "YES")
    set(CMAKE_CXX_FLAGS "-DDEBUG_MODE -pthread -std=c++11 -m32 -g -fno-inline -fno-omit-frame-pointer -Wno-reorder")
endif ()

set(SOURCE_FILES
        perft/Perft.cpp
        perft/PerftThread.cpp
        perft/PerftServer.cpp

        util/Bits.cpp
        util/String.cpp
        util/IniFile.cpp

        ChessBoard.cpp

        Eval.cpp

        GenMoves.cpp

        Hash.cpp

        IterativeDeeping.cpp

        main.cpp

        OpenBook.cpp

        Search.cpp

        SearchManager.cpp

        Tablebase.cpp

        Endgame.cpp

        Uci.cpp

        network/Server.cpp
        network/Client.cpp
)

set(CMAKE_CXX_COMPILER "g++")
add_executable(cinnamon ${SOURCE_FILES})
if (DR_MEMORY STREQUAL "YES")
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/32/libgtb.a)
else()
    target_link_libraries(cinnamon ${CMAKE_SOURCE_DIR}/gtb/Linux/64/libgtb.a)
endif ()