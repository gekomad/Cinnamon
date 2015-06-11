#    Cinnamon is a UCI chess engine - Makefile for cross-compile Linux/Windows/OSX
#    Copyright (C) 2011-2015 Giuseppe Cannella
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
ARC=
LIBS=-s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
OS=$(shell uname -s)
ifeq ($(OS),Darwin)
	LIBS= -static-libgcc -static-libstdc++ -lpthread
	OS=OSX
else
STATIC="-static "
endif
RM=rm -f
PA="./"
EXE="cinnamon"
STRIP=strip

else

# Windows
ARC="-mno-ms-bitfields"
OS=Windows
RM=del
PA=""
LIBS=-static -static-libgcc -static-libstdc++
EXE="cinnamon.exe"
STRIP=strip.exe
endif

ifeq ($(COMP),g++)
	PARAM="-std=c++11 -O3 -s -pipe -finline-functions -DNDEBUG -fno-exceptions -fno-rtti -fomit-frame-pointer -Wfloat-equal -Wpedantic -Wunreachable-code -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -Wundef -Winit-self -Wredundant-decls -Wcast-align -ffast-math -funroll-loops -Wno-long-long -Wall -Wcast-qual -Wextra -Wshadow "
else
ifeq ($(COMP),clang++)
	PARAM="-std=c++11 -O3 -pipe  -DNDEBUG -fno-exceptions -fno-rtti -fomit-frame-pointer -Wfloat-equal -Wpedantic -Wunreachable-code -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Wmain -Wundef -Winit-self -Wredundant-decls -Wcast-align -ffast-math -funroll-loops -Wno-long-long  -Wcast-qual -Wextra -Wshadow "
else
	PARAM="-std=c++11 -O3 "
endif
endif

help:

	@echo "Makefile for cross-compile Linux/Windows/OSX"
	@echo ""
	@echo "make cinnamon64-native           > x86 64-bit optimized for native cpu"
	@echo "make cinnamon64-modern-INTEL     > x86 64-bit optimized for modern Intel cpu"
	@echo "make cinnamon64-modern-AMD       > x86 64-bit optimized for modern Amd cpu"
	@echo "make cinnamon64-modern           > x86 64-bit with popcnt bsf sse3 support"
	@echo "make cinnamon64-generic          > unspecified 64-bit"
	@echo "make cinnamon32-native           > x86 32-bit optimized for native cpu"
	@echo "make cinnamon32-modern           > x86 32-bit with sse support"
	@echo "make cinnamon32-generic          > unspecified 32-bit"
	@echo "make cinnamon32-CLOP             > x86 32-bit optimized for CLOP parameter tuning"
	@echo ""
	@echo "g++ is the default compiler, add COMP=yourcompiler to use another compiler"
	@echo "example: make cinnamon64-modern-INTEL COMP=clang++"
	@echo ""

build:
ifeq ($(COMP),g++)
		$(MAKE) -j EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC)-fprofile-generate all
		@echo "optimize..."
		$(PA)$(EXE) -b >bench.tmp
		$(RM) *.o bench.tmp
		@echo "build..."
		$(MAKE) -j EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC)-fprofile-use all
else
		$(MAKE) -j EXE=$(EXE) CFLAGS=$(PARAM)$(STATIC) all
endif
	$(STRIP) $(EXE)

cinnamon32-native:
	$(MAKE)  ARC="$(ARC) -march=native" cinnamon32-generic

cinnamon32-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/32/libgtb.a" ARC="$(ARC) -m32" build

cinnamon32-modern:
	$(MAKE) ARC="$(ARC) -mfpmath=sse" cinnamon32-generic


cinnamon64-native:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/64/libgtb.a" ARC="$(ARC) -march=native" build

cinnamon64-generic:
	$(MAKE) LIBS="$(LIBS) gtb/$(OS)/64/libgtb.a" ARC="$(ARC) -m64" build

cinnamon64-modern:
	$(MAKE) ARC="$(ARC) -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF " cinnamon64-generic

cinnamon64-modern-AMD:
	$(MAKE) ARC="$(ARC) -msse4a -march=athlon64 -mtune=athlon64 " cinnamon64-modern

cinnamon64-modern-INTEL:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=corei7 -mtune=corei7 " cinnamon64-modern

all: namespaces.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Hash.o Uci.o OpenBook.o Tablebase.o String.o Bits.o
	$(COMP) ${CFLAGS} ${ARC} -o ${EXE} namespaces.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Hash.o Uci.o OpenBook.o Tablebase.o String.o Bits.o ${LIBS}

default:
	help

namespaces.o: namespaces.cpp
	$(COMP) -c namespaces.cpp ${CFLAGS} ${ARC}

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

Perft.o: Perft.cpp
	$(COMP) -c Perft.cpp ${CFLAGS} ${ARC}

Search.o: Search.cpp
	$(COMP) -c Search.cpp ${CFLAGS} ${ARC}

Uci.o: Uci.cpp
	$(COMP) -c Uci.cpp ${CFLAGS} ${ARC}

Hash.o: Hash.cpp
	$(COMP) -c Hash.cpp ${CFLAGS} ${ARC}

OpenBook.o: OpenBook.cpp
	$(COMP) -c OpenBook.cpp ${CFLAGS} ${ARC}

Tablebase.o: Tablebase.cpp
	$(COMP) -c Tablebase.cpp ${CFLAGS} ${ARC}

String.o: String.cpp
	$(COMP) -c String.cpp ${CFLAGS} ${ARC}

Bits.o: Bits.cpp
	$(COMP) -c Bits.cpp ${CFLAGS} ${ARC}


clean:
	$(RM) *.o $(EXE) *.gcda

