#    Cinnamon UCI chess engine - Makefile for cross-compile
#    Copyright (C) Giuseppe Cannella
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

COMP=g++

ifndef SystemRoot
	# *nix
	ifeq ($(shell uname -s),Darwin)
		LIBS= -static-libgcc -static-libstdc++ -lpthread
		OS=OSX
	else
	    LIBS=-s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static
		PROFILE_USE_THREAD=" -fprofile-correction -fprofile-use "		
		OS=Linux
	endif

	RM=rm -f
	PA="./"
	EXE="cinnamon"
	STRIP=strip
else
	# Windows
	OS=Windows
	RM=del
	PA=""
	LIBS=-static -static-libgcc -static-libstdc++
	EXE="cinnamon.exe"
	STRIP=strip.exe
endif

CFLAGS=" -std=c++11 -DDLOG_LEVEL=_FATAL -Ofast -DNDEBUG -fsigned-char -fno-exceptions -fno-rtti -funroll-loops "

help:

	@echo "Makefile for cross-compile Linux/Windows/OSX/ARM/Javascript"
	@echo ""
	@echo "make cinnamon64-intel-i7         > 64-bit optimized for Intel i7 cpu"
	@echo "make cinnamon64-intel-i9         > 64-bit optimized for Intel i9 cpu"
	@echo "make cinnamon64-athlon64         > 64-bit optimized for Amd cpu"
	@echo "make cinnamon64-sse3             > 64-bit with popcnt bsf sse3 support"
	@echo "make cinnamon64-generic          > Unspecified 64-bit"
	@echo "make cinnamon64-ARM              > Optimized for ARM cpu"
	@echo ""
	@echo "make cinnamon32-sse              > 32-bit with sse support"
	@echo "make cinnamon32-generic          > Unspecified 32-bit"
	@echo "make cinnamon32-ARM              > Optimized for ARM cpu"
	@echo ""
	@echo "make cinnamon-js                 > Javascript build"
	@echo ""
	@echo "make cinnamon-drmemory           > Memory monitor"
	@echo "make cinnamon-profiler           > Google profiler tool"
	@echo "make cinnamon-gprof              > Gnu profiler tool"
	@echo ""
	@echo "add:"
	@echo " COMP=compiler                   > Use another compiler"
	@echo " PROFILE_GCC=yes                 > PGO build"
	@echo " FULL_TEST=yes                   > Unit test (uses libgtest-dev)"	
	@echo ""

build:

ifeq ($(FULL_TEST),yes)
	$(MAKE) -j4 EXE=$(EXE) LIBS="$(LIBS) /usr/local/lib/libgtest.a " CFLAGS=-DDEBUG_MODE$(CFLAGS)-DFULL_TEST all
	$(PA)$(EXE)
else
ifeq ($(PROFILE_GCC),yes)
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(CFLAGS)-fprofile-generate all
	@echo "optimize..."
	$(PA)$(EXE) -b1 >bench.tmp
	$(RM) *.o bench.tmp
	@echo "build..."
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(CFLAGS)$(PROFILE_USE_THREAD) all
else
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(CFLAGS) all
endif
endif

	$(STRIP) $(EXE)
	@echo "create static library..."
	ar rcs libCinnamon.a ChessBoard.o Uci.o WrapperCinnamon.o Search.o IterativeDeeping.o Eval.o Perft.o Hash.o PerftThread.o SearchManager.o OpenBook.o GTB.o

drmemory:
	$(MAKE) -j4 EXE=$(EXE) all

	@echo "\n\n *** PERFT SINGLE CORE ***\n"
	drmemory -- $(PA)$(EXE) -perft -d3

	@echo "\n\n *** PERFT QUAD CORE ***\n"
	drmemory -- $(PA)$(EXE) -perft -d4 -c4 -h100

	@echo "\n\n *** SEARCH SINGLE CORE ***\n"
	drmemory -- $(PA)$(EXE) -b1

	@echo "\n\n *** SEARCH QUAD CORE ***\n"
	drmemory -- $(PA)$(EXE) -b4

profiler:
	$(MAKE) -j4 EXE=$(EXE) all
	@echo "\n\n **** \nsudo apt-get install google-perftools libgoogle-perftools-dev && export LD_PRELOAD=/usr/lib/libtcmalloc.so:/usr/lib/libprofiler.so\n **** \n"

	@echo "\n\n *** PERFT SINGLE CORE ***\n"
	$(RM) profiler.tmp
	CPUPROFILE=profiler.tmp $(PA)$(EXE) -perft -d7
	google-pprof --text $(PA)$(EXE) profiler.tmp

	@echo "\n\n *** PERFT QUAD CORE ***\n"
	$(RM) profiler.tmp
	CPUPROFILE=profiler.tmp $(PA)$(EXE) -perft -d7 -c4 -h100
	google-pprof --text $(PA)$(EXE) profiler.tmp

	@echo "\n\n *** SEARCH SINGLE CORE ***\n"
	$(RM) profiler.tmp
	CPUPROFILE=profiler.tmp $(PA)$(EXE) -b1
	google-pprof --text $(PA)$(EXE) profiler.tmp

	@echo "\n\n *** SEARCH QUAD CORE ***\n"
	$(RM) profiler.tmp
	CPUPROFILE=profiler.tmp $(PA)$(EXE) -b4
	google-pprof --text $(PA)$(EXE) profiler.tmp

gnuprof:
	$(MAKE) -j4 EXE=$(EXE) all

	@echo "\n\n *** PERFT SINGLE CORE ***\n"
	$(PA)$(EXE) -perft -d6
	gprof $(PA)$(EXE)

	@echo "\n\n *** PERFT QUAD CORE ***\n"
	$(PA)$(EXE) -perft -d7 -c4 -h100
	gprof $(PA)$(EXE)

	@echo "\n\n *** SEARCH SINGLE CORE ***\n"
	$(PA)$(EXE) -b1
	gprof $(PA)$(EXE)

	@echo "\n\n *** SEARCH QUAD CORE ***\n"
	$(PA)$(EXE) -b4
	gprof $(PA)$(EXE)

cinnamon-js:
	em++ -std=c++11 -DJS_MODE -DDLOG_LEVEL=_FATAL util/Bitboard.cpp -fsigned-char ChessBoard.cpp Eval.cpp Hash.cpp IterativeDeeping.cpp js/main.cpp OpenBook.cpp Search.cpp SearchManager.cpp perft/Perft.cpp util/String.cpp util/IniFile.cpp util/Timer.cpp perft/PerftThread.cpp -w -s EXPORTED_FUNCTIONS="['_main','_perft','_command','_isvalid']" -s NO_EXIT_RUNTIME=1 -o cinnamon.js -O3 --memory-init-file 0

cinnamon-drmemory:
	$(MAKE) LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive lib/$(OS)/32/libgtb.a" CFLAGS="-pthread -std=c++11 -DHAS_BSF -DHAS_POPCNT -g -fsigned-char -fno-inline -fno-omit-frame-pointer -m32 " drmemory

cinnamon32-generic:
	$(MAKE) LIBS="$(LIBS) lib/$(OS)/32/libgtb.a" ARC="$(ARC) -m32 -w" build

cinnamon32-sse:
	$(MAKE) ARC="$(ARC) -mfpmath=sse -DHAS_POPCNT -DHAS_BSF" cinnamon32-generic

cinnamon32-ARM:
	$(MAKE) LIBS="$(LIBS) lib/ARM/32/libgtb.a" ARC="$(ARC) -DHAS_POPCNT -fno-gcse -DHAS_BSF -w" build

cinnamon64-ARM:
	$(MAKE) LIBS="$(LIBS) lib/ARM/64/libgtb.a" ARC="$(ARC) -DHAS_POPCNT -fno-gcse -DHAS_64BIT -m64 -DHAS_BSF -w" build

cinnamon64-generic:
	$(MAKE) LIBS="$(LIBS) lib/$(OS)/64/libgtb.a" ARC="$(ARC) -DHAS_64BIT -m64" build

cinnamon64-sse3:
	$(MAKE) ARC="$(ARC) -DHAS_POPCNT -mpopcnt -msse3 -DHAS_POPCNT -DHAS_BSF " cinnamon64-generic

cinnamon64-athlon64:
	$(MAKE) ARC="$(ARC) -msse4a -march=athlon64 -mtune=athlon64 " cinnamon64-sse3

cinnamon64-intel-i7:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=corei7 -mtune=corei7 " cinnamon64-sse3

cinnamon64-intel-i9:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=haswell -mtune=haswell " cinnamon64-sse3

cinnamon-profiler:
	$(MAKE) ARC=" -msse4.2 -march=corei7 -mtune=corei7 " CFLAGS=" -std=c++11 -ltcmalloc -DHAS_POPCNT -DDLOG_LEVEL=_FATAL -Ofast -DNDEBUG -fsigned-char -fno-exceptions -fno-rtti -funroll-loops " LIBS=" -Wl,--whole-archive -lpthread -Wl,--no-whole-archive gtb/$(OS)/64/libgtb.a " profiler

cinnamon-gprof:
	$(MAKE) ARC=" -msse4.2 -march=corei7 -mtune=corei7 " CFLAGS=" -std=c++11 -g -pg -DHAS_POPCNT -DDLOG_LEVEL=_FATAL -DNDEBUG -fsigned-char -fno-exceptions -fno-rtti -funroll-loops " LIBS=" -Wl,--whole-archive -lpthread -Wl,--no-whole-archive gtb/$(OS)/64/libgtb.a " gnuprof

all: main.o ChessBoard.o tbprobe.o SYZYGY.o Eval.o test.o String.o GenMoves.o WrapperCinnamon.o Bitboard.o Timer.o IniFile.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o GTB.o
	$(COMP) $(ARC) ${CFLAGS} -o ${EXE} main.o test.o ChessBoard.o GenMoves.o tbprobe.o SYZYGY.o WrapperCinnamon.o Bitboard.o Timer.o Eval.o IniFile.o String.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o GTB.o ${LIBS}

default:
	help

ChessBoard.o: ChessBoard.cpp
	$(COMP) -c ChessBoard.cpp ${CFLAGS} ${ARC}

Eval.o: Eval.cpp
	$(COMP) -c Eval.cpp ${CFLAGS} ${ARC}

IterativeDeeping.o: IterativeDeeping.cpp
	$(COMP) -c IterativeDeeping.cpp ${CFLAGS} ${ARC}

main.o: main.cpp
	$(COMP) -c main.cpp ${CFLAGS} ${ARC}

Perft.o: perft/Perft.cpp
	$(COMP) -c perft/Perft.cpp ${CFLAGS} ${ARC}

PerftThread.o: perft/PerftThread.cpp
	$(COMP) -c perft/PerftThread.cpp ${CFLAGS} ${ARC}

Search.o: Search.cpp
	$(COMP) -c Search.cpp ${CFLAGS} ${ARC}

SearchManager.o: SearchManager.cpp
	$(COMP) -c SearchManager.cpp ${CFLAGS} ${ARC}

Uci.o: Uci.cpp
	$(COMP) -c Uci.cpp ${CFLAGS} ${ARC}

GenMoves.o: GenMoves.cpp
	$(COMP) -c GenMoves.cpp ${CFLAGS} ${ARC}

Hash.o: Hash.cpp
	$(COMP) -c Hash.cpp ${CFLAGS} ${ARC}

OpenBook.o: db/OpenBook.cpp
	$(COMP) -c db/OpenBook.cpp ${CFLAGS} ${ARC}

SYZYGY.o: db/syzygy/SYZYGY.cpp
	$(COMP) -c db/syzygy/SYZYGY.cpp ${CFLAGS} ${ARC}

tbprobe.o: db/syzygy/tbprobe.cpp
	$(COMP) -c db/syzygy/tbprobe.cpp ${CFLAGS} ${ARC}

GTB.o: db/GTB.cpp
	$(COMP) -c db/GTB.cpp ${CFLAGS} ${ARC}

String.o: util/String.cpp
	$(COMP) -c util/String.cpp ${CFLAGS} ${ARC}

IniFile.o: util/IniFile.cpp
	$(COMP) -c util/IniFile.cpp ${CFLAGS} ${ARC}

Timer.o: util/Timer.cpp
	$(COMP) -c util/Timer.cpp ${CFLAGS} ${ARC}

test.o: test/test.cpp
	$(COMP) -c test/test.cpp ${CFLAGS} ${ARC}

Bitboard.o: util/Bitboard.cpp
	$(COMP) -c util/Bitboard.cpp ${CFLAGS} ${ARC}

WrapperCinnamon.o: WrapperCinnamon.cpp
	$(COMP) -c WrapperCinnamon.cpp ${CFLAGS} ${ARC}

clean:
	$(RM) *.o $(EXE) *.gcda libCinnamon.a cinnamon.js util/*.o js/*.o perft/*.o test/*.o db/*.o

