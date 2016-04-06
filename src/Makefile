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
	LIBS=-s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

	ifeq ($(shell uname -s),Darwin)
		LIBS= -static-libgcc -static-libstdc++ -lpthread
		OS=OSX
	else
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
	@echo "make cinnamon64-modern-INTEL     > 64-bit optimized for modern Intel cpu"
	@echo "make cinnamon64-modern-AMD       > 64-bit optimized for modern Amd cpu"
	@echo "make cinnamon64-modern           > 64-bit with popcnt bsf sse3 support"
	@echo "make cinnamon64-generic          > Unspecified 64-bit"
	@echo "make cinnamon64-ARM              > Optimized for arm cpu"
	@echo ""
	@echo "make cinnamon32-modern           > 32-bit with sse support"
	@echo "make cinnamon32-generic          > Unspecified 32-bit"
	@echo "make cinnamon32-ARM              > Optimized for arm cpu"
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
	@echo " FULL_TEST=yes                   > Unit test"
	@echo " LIBSTDC=-static                 > Link statically libstdc++"
	@echo ""

build:

ifeq ($(FULL_TEST),yes)
	$(MAKE) -j4 EXE=$(EXE) LIBS="$(LIBS) /usr/local/lib/libgtest.a " CFLAGS=$(LIBSTDC)$(CFLAGS)-DFULL_TEST all
	$(PA)$(EXE)
else
ifeq ($(PROFILE_GCC),yes)
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(LIBSTDC)$(CFLAGS)-fprofile-generate all
	@echo "optimize..."
	$(PA)$(EXE) -b1 >bench.tmp
	$(RM) *.o bench.tmp
	@echo "build..."
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(LIBSTDC)$(CFLAGS)$(PROFILE_USE_THREAD) all
else
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(LIBSTDC)$(CFLAGS) all
endif
endif

	$(STRIP) $(EXE)
	@echo "create static library..."
	ar rcs libCinnamon.a ChessBoard.o Uci.o WrapperCinnamon.o Search.o IterativeDeeping.o Eval.o GenMoves.o Perft.o Hash.o PerftThread.o SearchManager.o OpenBook.o Tablebase.o

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
	em++ -std=c++11 -DJS_MODE -DDLOG_LEVEL=_FATAL util/Bitboard.cpp -fsigned-char ChessBoard.cpp Eval.cpp GenMoves.cpp Hash.cpp IterativeDeeping.cpp js/main.cpp OpenBook.cpp Search.cpp SearchManager.cpp perft/Perft.cpp util/String.cpp util/IniFile.cpp util/Timer.cpp perft/PerftThread.cpp -w -s EXPORTED_FUNCTIONS="['_main','_perft','_command','_isvalid']" -s NO_EXIT_RUNTIME=1 -o cinnamon.js -O3 --memory-init-file 0

cinnamon-drmemory:
	$(MAKE) LIBS="-Wl,--whole-archive -lpthread -Wl,--no-whole-archive gtb/$(OS)/32/libgtb.a" CFLAGS="-pthread -std=c++11 -g -fsigned-char -fno-inline -fno-omit-frame-pointer -m32 " drmemory

cinnamon32-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/32/libgtb.a" ARC="$(ARC) -m32 -w" build

cinnamon32-modern:
	$(MAKE) ARC="$(ARC) -mfpmath=sse -DHAS_BSF" cinnamon32-generic

cinnamon32-ARM:
	$(MAKE) LIBS="$(LIBS) gtb/ARM/32/libgtb.a" ARC="$(ARC) -fno-gcse -DHAS_BSF -w" build

cinnamon64-ARM:
	$(MAKE) LIBS="$(LIBS) gtb/ARM/64/libgtb.a" ARC="$(ARC) -fno-gcse -DHAS_BSF -w" build

cinnamon64-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/64/libgtb.a" ARC="$(ARC) -m64" build

cinnamon64-modern:
	$(MAKE) ARC="$(ARC) -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF " cinnamon64-generic

cinnamon64-modern-AMD:
	$(MAKE) ARC="$(ARC) -msse4a -march=athlon64 -mtune=athlon64 " cinnamon64-modern

cinnamon64-modern-INTEL:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=corei7 -mtune=corei7 " cinnamon64-modern

cinnamon-profiler:
	$(MAKE) ARC=" -msse4.2 -march=corei7 -mtune=corei7 " CFLAGS=" -std=c++11 -ltcmalloc -DDLOG_LEVEL=_FATAL -Ofast -DNDEBUG -fsigned-char -fno-exceptions -fno-rtti -funroll-loops " LIBS=" -Wl,--whole-archive -lpthread -Wl,--no-whole-archive gtb/$(OS)/64/libgtb.a " profiler

cinnamon-gprof:
	$(MAKE) ARC=" -msse4.2 -march=corei7 -mtune=corei7 " CFLAGS=" -std=c++11 -g -pg -DDLOG_LEVEL=_FATAL -DNDEBUG -fsigned-char -fno-exceptions -fno-rtti -funroll-loops " LIBS=" -Wl,--whole-archive -lpthread -Wl,--no-whole-archive gtb/$(OS)/64/libgtb.a " gnuprof

all: main.o ChessBoard.o Eval.o GenMoves.o test.o String.o WrapperCinnamon.o Bitboard.o Timer.o IniFile.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o Tablebase.o
	$(COMP) $(ARC) ${CFLAGS} -o ${EXE} main.o test.o ChessBoard.o WrapperCinnamon.o Bitboard.o Timer.o Eval.o IniFile.o GenMoves.o String.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o Tablebase.o ${LIBS}

default:
	help

ChessBoard.o: ChessBoard.cpp
	$(COMP) -c ChessBoard.cpp ${CFLAGS} ${ARC}

Eval.o: Eval.cpp
	$(COMP) -c Eval.cpp ${CFLAGS} ${ARC}

GenMoves.o: GenMoves.cpp
	$(COMP) -c GenMoves.cpp ${CFLAGS} ${ARC}

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

Hash.o: Hash.cpp
	$(COMP) -c Hash.cpp ${CFLAGS} ${ARC}

OpenBook.o: OpenBook.cpp
	$(COMP) -c OpenBook.cpp ${CFLAGS} ${ARC}

Tablebase.o: Tablebase.cpp
	$(COMP) -c Tablebase.cpp ${CFLAGS} ${ARC}

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
	$(RM) *.o $(EXE) *.gcda libCinnamon.a cinnamon.js

