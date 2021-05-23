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
		LIBS= -static-libstdc++ -lpthread
		OS=OSX
	else
	    LIBS=-s -Wl,--whole-archive -static -lpthread -Wl,--no-whole-archive
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

CFLAGS=" -std=c++11 -Wall -fno-stack-protector -fno-rtti -fno-exceptions -fomit-frame-pointer -O3 -Wpedantic -W -DNDEBUG -fsigned-char -funroll-loops "

help:

	@echo "Makefile for cross-compile Linux/Windows/OSX/ARM/Javascript"
	@echo ""
	@echo "make cinnamon64-modern-INTEL     > 64-bit optimized for modern Intel cpu"
	@echo "make cinnamon64-BMI2             > 64-bit optimized for Haswell Intel cpu"
	@echo "make cinnamon64-modern-AMD       > 64-bit optimized for modern Amd cpu"
	@echo "make cinnamon64-modern           > 64-bit with popcnt bsf sse3 support"
	@echo "make cinnamon64-generic          > Unspecified 64-bit"
	@echo "make cinnamon64-ARM              > Optimized for ARM cpu"
	@echo ""
	@echo "make cinnamon32-modern           > 32-bit with sse support"
	@echo "make cinnamon32-generic          > Unspecified 32-bit"
	@echo "make cinnamon32-ARM              > Optimized for ARM cpu"
	@echo ""
	@echo "make cinnamon-js                 > Javascript build"
	@echo ""
	@echo "add:"
	@echo " COMP=compiler                   > Use another compiler (default g++)"
	@echo " FULL_TEST=yes                   > Run unit test (googletest)"
	@echo ""

build:

ifeq ($(FULL_TEST),yes)
	$(MAKE) -j EXE=$(EXE) LIBS="$(LIBS) /usr/lib/libgtest.a " CFLAGS=$(CFLAGS)-DFULL_TEST all
	$(PA)$(EXE)
else
	$(MAKE) EXE=$(EXE) CFLAGS=$(CFLAGS) all
endif

	$(STRIP) $(EXE)
	@echo "create static library..."
	ar rcs libCinnamon.a Bitboard.o board.o ChessBoard.o Uci.o Timer.o WrapperCinnamon.o Search.o IterativeDeeping.o Eval.o GenMoves.o Perft.o Hash.o PerftThread.o SearchManager.o

cinnamon-js:
	emcc -std=c++11 -w -DNDEBUG -DJS_MODE -DDLOG_LEVEL=_FATAL util/Bitboard.cpp -fsigned-char namespaces/board.cpp ChessBoard.cpp Eval.cpp Hash.cpp IterativeDeeping.cpp GenMoves.cpp js/main.cpp \
	Search.cpp SearchManager.cpp perft/Perft.cpp util/Timer.cpp perft/PerftThread.cpp \
	-s WASM=0 -s EXPORTED_FUNCTIONS="['_main','_perft','_command','_isvalid']" -s EXTRA_EXPORTED_RUNTIME_METHODS='["cwrap"]' -s NO_EXIT_RUNTIME=1 -o cinnamon.js -O3 --memory-init-file 0

cinnamon32-generic:
	$(MAKE) ARC="$(ARC) -m32" build

cinnamon32-modern:
	$(MAKE) ARC="$(ARC) -mfpmath=sse -DHAS_POPCNT -DHAS_BSF" cinnamon32-generic

cinnamon32-ARM:
	$(MAKE) -j4 ARC="$(ARC) -DHAS_POPCNT -fno-gcse -DHAS_BSF" build

cinnamon64-ARM:
	$(MAKE) -j4 ARC="$(ARC) -DHAS_POPCNT -DHAS_BSF" build

cinnamon64-generic:
	$(MAKE) -j ARC="$(ARC) -m64" build

cinnamon64-modern:
	$(MAKE) ARC="$(ARC) -DHAS_POPCNT -mpopcnt -msse3 -DHAS_BSF " cinnamon64-generic

cinnamon64-modern-AMD:
	$(MAKE) ARC="$(ARC) -msse4a -march=athlon64 " cinnamon64-modern

cinnamon64-modern-INTEL:
	$(MAKE) ARC="$(ARC) -msse4.2 -march=corei7 " cinnamon64-modern

cinnamon64-BMI2:
	$(MAKE) ARC="$(ARC) -mbmi2 -DUSE_BMI2 " cinnamon64-modern-INTEL

all: main.o ChessBoard.o board.o Eval.o test.o GenMoves.o WrapperCinnamon.o Bitboard.o Timer.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o GTB.o SYZYGY.o tbprobe.o Lzma86Dec.o LzFind.o Lzma86Enc.o Alloc.o LzmaDec.o Bra86.o LzmaEnc.o wrap.o gtb-dec.o gtb-att.o sysport.o gtb-probe.o
	$(COMP) $(ARC) ${CFLAGS} -o ${EXE} main.o test.o ChessBoard.o board.o GenMoves.o WrapperCinnamon.o Bitboard.o Timer.o Eval.o IterativeDeeping.o Perft.o PerftThread.o Search.o SearchManager.o Hash.o Uci.o GTB.o SYZYGY.o tbprobe.o Lzma86Dec.o LzFind.o Lzma86Enc.o Alloc.o LzmaDec.o Bra86.o LzmaEnc.o wrap.o gtb-dec.o gtb-att.o sysport.o gtb-probe.o ${LIBS}

default:
	help

ChessBoard.o: ChessBoard.cpp
	$(COMP) -c ChessBoard.cpp ${CFLAGS} ${ARC}

board.o: namespaces/board.cpp
	$(COMP) -c namespaces/board.cpp ${CFLAGS} ${ARC}

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

GTB.o: db/gaviota/GTB.cpp
	$(COMP) -c db/gaviota/GTB.cpp ${CFLAGS} ${ARC}

Lzma86Dec.o: db/gaviota/gtb/compression/lzma/Lzma86Dec.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/Lzma86Dec.c ${CFLAGS} ${ARC}

LzFind.o: db/gaviota/gtb/compression/lzma/LzFind.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/LzFind.c ${CFLAGS} ${ARC}

Lzma86Enc.o: db/gaviota/gtb/compression/lzma/Lzma86Enc.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/Lzma86Enc.c ${CFLAGS} ${ARC}

LzmaDec.o: db/gaviota/gtb/compression/lzma/LzmaDec.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/LzmaDec.c ${CFLAGS} ${ARC}

Alloc.o: db/gaviota/gtb/compression/lzma/Alloc.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/Alloc.c ${CFLAGS} ${ARC}

Bra86.o: db/gaviota/gtb/compression/lzma/Bra86.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/Bra86.c ${CFLAGS} ${ARC}

LzmaEnc.o: db/gaviota/gtb/compression/lzma/LzmaEnc.c
	$(COMP) -c db/gaviota/gtb/compression/lzma/LzmaEnc.c ${CFLAGS} ${ARC}

wrap.o: db/gaviota/gtb/compression/wrap.c
	$(COMP) -c db/gaviota/gtb/compression/wrap.c ${CFLAGS} ${ARC}

gtb-dec.o: db/gaviota/gtb/gtb-dec.c
	$(COMP) -c db/gaviota/gtb/gtb-dec.c ${CFLAGS} ${ARC} 

gtb-att.o: db/gaviota/gtb/gtb-att.c
	$(COMP) -c db/gaviota/gtb/gtb-att.c ${CFLAGS} ${ARC}

sysport.o: db/gaviota/gtb/sysport/sysport.c
	$(COMP) -c db/gaviota/gtb/sysport/sysport.c ${CFLAGS} ${ARC}

gtb-probe.o: db/gaviota/gtb/gtb-probe.c
	$(COMP) -c db/gaviota/gtb/gtb-probe.c ${CFLAGS} ${ARC}

SYZYGY.o: db/syzygy/SYZYGY.cpp
	$(COMP) -c db/syzygy/SYZYGY.cpp ${CFLAGS} ${ARC}

tbprobe.o: db/syzygy/tbprobe.c
	$(COMP) -c db/syzygy/tbprobe.c ${CFLAGS} ${ARC}

Timer.o: util/Timer.cpp
	$(COMP) -c util/Timer.cpp ${CFLAGS} ${ARC}

test.o: test/test.cpp
	$(COMP) -c test/test.cpp ${CFLAGS} ${ARC}

Bitboard.o: util/Bitboard.cpp
	$(COMP) -c util/Bitboard.cpp ${CFLAGS} ${ARC}

WrapperCinnamon.o: WrapperCinnamon.cpp
	$(COMP) -c WrapperCinnamon.cpp ${CFLAGS} ${ARC}

clean:
	$(RM) *.o $(EXE) cinnamon.js libCinnamon.a

