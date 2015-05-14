#Makefile for cross-compile Linux/Windows/Mac

LIBS=-lpthread
CFLAGS_PROFILE="-std=c++0x -w -O3 -fprofile-generate -fno-exceptions -fno-rtti -fomit-frame-pointer"
CFLAGS_RELEASE="-std=c++0x -w -O3 -fprofile-use -fno-exceptions -fno-rtti -fomit-frame-pointer"
CFLAGS_NO_PROFILE="-std=c++0x -w -O3 -fno-exceptions -fno-rtti -fomit-frame-pointer"
CC="g++"
ifndef SystemRoot
RM=rm -f
PA="./"
EXE="butterfly"
STRIP=strip
else
PA=""
EXE="butterfly.exe"
STRIP=strip.exe
RM=del
endif

_init_:
ifndef CFLAGS
	@echo "To compile butterfly type:"
	@echo ""
	@echo "make butterfly32"
	@echo "make butterfly64"
	@echo ""
endif

all: butterfly

cbutterfly:
	$(MAKE) EXE=$(EXE) CFLAGS=$(CFLAGS_PROFILE) all
	@echo "optimize..."
	$(PA)$(EXE) -b >bench.log
	$(RM) *.o bench.log
	@echo "build..."
	$(MAKE) EXE=$(EXE) CFLAGS=$(CFLAGS_RELEASE) all

butterfly32:
	$(MAKE) ARC="-m32" cbutterfly

butterfly64:
	$(MAKE) ARC="-m64" cbutterfly

tuningcrafty32:
	$(MAKE) EXE=$(EXE) CFLAGS=$(CFLAGS_NO_PROFILE) DEF="-DTUNE_CRAFTY_MODE" ARC="-m32" 	all

tuningcrafty64:
	$(MAKE) EXE=$(EXE) CFLAGS=$(CFLAGS_NO_PROFILE) DEF="-DTUNE_CRAFTY_MODE" ARC="-m64" all

butterfly: _init_ utils.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o TuningCrafty.o Hash.o Uci.o
	$(CC) ${CFLAGS} -o ${EXE} utils.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o TuningCrafty.o Hash.o Uci.o  ${LIBS} ${ARC}
	$(STRIP) $(EXE)

utils.o: utils.cpp
	$(CC) -c utils.cpp ${CFLAGS} ${DEF} ${ARC}

ChessBoard.o: ChessBoard.cpp
	$(CC) -c ChessBoard.cpp ${CFLAGS} ${DEF} ${ARC}

Eval.o: Eval.cpp
	$(CC) -c Eval.cpp ${CFLAGS} ${DEF} ${ARC}

GenMoves.o: GenMoves.cpp
	$(CC) -c GenMoves.cpp ${CFLAGS} ${DEF} ${ARC}

IterativeDeeping.o: IterativeDeeping.cpp
	$(CC) -c IterativeDeeping.cpp ${CFLAGS} ${DEF} ${ARC}

main.o: main.cpp
	$(CC) -c main.cpp ${CFLAGS} ${DEF} ${ARC}

Perft.o: Perft.cpp
	$(CC) -c Perft.cpp ${CFLAGS} ${DEF} ${ARC}

Search.o: Search.cpp
	$(CC) -c Search.cpp ${CFLAGS} ${DEF} ${ARC}

TuningCrafty.o: TuningCrafty.cpp
	$(CC) -c TuningCrafty.cpp ${CFLAGS} ${DEF} ${ARC}

Uci.o: Uci.cpp
	$(CC) -c Uci.cpp ${CFLAGS} ${DEF} ${ARC}

Hash.o: Hash.cpp
	$(CC) -c Hash.cpp ${CFLAGS} ${DEF} ${ARC}

clean:
	$(RM) *.o $(EXE) *.gcda

