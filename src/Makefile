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

NATIVE_ARCH="32"
ifndef SystemRoot
	# *nix
	ARC=
	LIBS=-s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive

	ifeq ($(shell uname -s),Darwin)
		LIBS= -static-libgcc -static-libstdc++ -lpthread
		OS=OSX
	else
		PROFILE_USE_THREAD=" -fprofile-correction -fprofile-use "
		STATIC="-static "
		OS=Linux
	endif

	RM=rm -f
	PA="./"
	EXE="cinnamon"
	STRIP=strip
	UNAME_P := $(shell uname -p)
	ifeq ($(UNAME_P),x86_64)
		NATIVE_ARCH="64"
	endif
else
	# Windows
	ARC="-mno-ms-bitfields"
	OS=Windows
	RM=del
	PA=""
	LIBS=-static -static-libgcc -static-libstdc++
	EXE="cinnamon.exe"
	STRIP=strip.exe

	ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
		NATIVE_ARCH="64"
	endif
endif

PARAM="-std=c++11 -Ofast -pipe -DNDEBUG -fsigned-char -DDLOG_LEVEL=_FATAL -fno-exceptions -fno-rtti -fomit-frame-pointer -Wfloat-equal -Wpedantic -Wunreachable-code -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -Wundef -Winit-self -Wredundant-decls -Wcast-align -ffast-math -funroll-loops -Wno-long-long -Wcast-qual -Wextra -Wshadow "

help:

	@echo "Makefile for cross-compile Linux/Windows/OSX/ARM"
	@echo ""
	@echo "make cinnamon64-modern-INTEL     > 64-bit optimized for modern Intel cpu"
	@echo "make cinnamon64-modern-AMD       > 64-bit optimized for modern Amd cpu"
	@echo "make cinnamon64-modern           > 64-bit with popcnt bsf sse3 support"
	@echo "make cinnamon64-generic          > Unspecified 64-bit"
	@echo ""
	@echo "make cinnamon32-modern           > 32-bit with sse support"
	@echo "make cinnamon32-generic          > Unspecified 32-bit"
	@echo "make cinnamon32-ARM              > Optimized for arm cpu"
	@echo ""
	@echo "make cinnamon-native             > Optimized for native cpu"
	@echo "make cinnamon-drmemory           > Memory monitor"
	@echo ""
	@echo "add:"
	@echo " COMP=compiler                   > Use another compiler"
	@echo " PROFILE_GCC=yes                 > PGO build"
	@echo " FULL_TEST=yes                   > Unit test"
	@echo ""

build:

ifeq ($(FULL_TEST),yes)
	$(MAKE) -j4 EXE=$(EXE) LIBS="$(LIBS) /usr/local/lib/libgtest.a " CFLAGS=$(PARAM)$(STATIC)-DFULL_TEST all
else
ifeq ($(PROFILE_GCC),yes)
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC)-fprofile-generate all
	@echo "optimize..."
	$(PA)$(EXE) -b1 >bench.tmp
	$(RM) *.o bench.tmp
	@echo "build..."
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC)$(PROFILE_USE_THREAD)  all
else
	$(MAKE) -j4 EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC) all
endif
endif

	$(STRIP) $(EXE)
	@echo "create static library..."
	ar rcs libCinnamon.a ChessBoard.o Uci.o WrapperCinnamon.o Search.o Bits.o IterativeDeeping.o Eval.o GenMoves.o Perft.o Hash.o PerftThread.o SearchManager.o OpenBook.o Tablebase.o

drmemory:
	$(MAKE) -j4 EXE=$(EXE) CFLAGS="-pthread -std=c++11 -g -fno-inline -fno-omit-frame-pointer " all
	@echo "check single core search log in drmmemory_1core.log..."
	drmemory -show_reachable -- $(PA)$(EXE) -b1 2>drmmemory_1core.log
	@echo "check quad core search log in drmmemory_4core.log..."
	drmemory -show_reachable -- $(PA)$(EXE) -b4 2>drmmemory_4core.log

cinnamon-drmemory:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/32/libgtb.a" ARC="$(ARC) -m32" drmemory

cinnamon32-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/32/libgtb.a" ARC="$(ARC) -m32" build

cinnamon32-modern:
	$(MAKE) ARC="$(ARC) -mfpmath=sse" cinnamon32-generic

cinnamon32-ARM:
	$(MAKE) LIBS="$(LIBS) gtb/ARM/32/libgtb.a" ARC="$(ARC) -fno-gcse -w" build

cinnamon-native:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/$(NATIVE_ARCH)/libgtb.a" ARC="$(ARC) -march=native" build

cinnamon64-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/64/libgtb.a" ARC="$(ARC) -m64" build

cinnamon64-modern:
	$(MAKE) ARC="$(ARC) -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF " cinnamon64-generic

cinnamon64-modern-AMD:
	$(MAKE) ARC="$(ARC) -msse4a -march=athlon64 -mtune=athlon64 " cinnamon64-modern

cinnamon64-modern-INTEL:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=corei7 -mtune=corei7 " cinnamon64-modern

all: main.o ChessBoard.o Eval.o GenMoves.o test.o String.o WrapperCinnamon.o Timer.o IniFile.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o Tablebase.o Bits.o
	$(COMP) ${CFLAGS} ${ARC} -o ${EXE} main.o test.o ChessBoard.o WrapperCinnamon.o Timer.o Eval.o IniFile.o GenMoves.o String.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o OpenBook.o Tablebase.o Bits.o ${LIBS}

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

Bits.o: util/Bits.cpp
	$(COMP) -c util/Bits.cpp ${CFLAGS} ${ARC}

String.o: util/String.cpp
	$(COMP) -c util/String.cpp ${CFLAGS} ${ARC}

IniFile.o: util/IniFile.cpp
	$(COMP) -c util/IniFile.cpp ${CFLAGS} ${ARC}

Timer.o: util/Timer.cpp
	$(COMP) -c util/Timer.cpp ${CFLAGS} ${ARC}

test.o: test/test.cpp
	$(COMP) -c test/test.cpp ${CFLAGS} ${ARC}

#Endgame.o: Endgame.cpp
#	$(COMP) -c Endgame.cpp ${CFLAGS} ${ARC}
	
WrapperCinnamon.o: WrapperCinnamon.cpp
	$(COMP) -c WrapperCinnamon.cpp ${CFLAGS} ${ARC}
clean:
	$(RM) *.o $(EXE) *.gcda libCinnamon.a
