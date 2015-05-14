#Makefile for cross-compile Linux/Windows/Mac

CFLAGS_PROFILE="-std=c++11 -w -O3 -fprofile-generate -fno-exceptions -fno-rtti -fomit-frame-pointer -ffast-math -march=native -funroll-loops $(EXTRACFLAGS)"
CFLAGS_RELEASE="-std=c++11 -w -O3 -fprofile-use -fno-exceptions -fno-rtti -fomit-frame-pointer -ffast-math -march=native -funroll-loops $(EXTRACFLAGS)"
CC="g++"
ifndef SystemRoot
RM=rm -f
PA="./"
LIBS=-lpthread
EXE="cinnamon"
STRIP=strip
EXTRACFLAGS= -static-libstdc++
else
RM=del
PA=""
LIBS=
EXE="cinnamon.exe"
STRIP=strip.exe
EXTRACFLAGS= -static -static-libgcc -static-libstdc++
endif

_init_:
ifndef CFLAGS
	@echo
	@echo "To compile cinnamon type:"
	@echo ""
	@echo "make cinnamon64"
	@echo "make cinnamon64_popcnt"
	@echo "make cinnamon32"
	@echo ""
endif

make_cinnamon:
	$(MAKE) -j EXE=$(EXE) CFLAGS=$(CFLAGS_PROFILE) all
	@echo "optimize..."
	$(PA)$(EXE) -b >bench.tmp
	$(RM) *.o bench.tmp
	@echo "build..."
	$(MAKE) -j EXE=$(EXE) CFLAGS=$(CFLAGS_RELEASE) all

cinnamon32:
	$(MAKE) ARC="-m32 -mfpmath=sse" make_cinnamon

cinnamon64:
	$(MAKE) ARC="-m64" make_cinnamon

cinnamon64_popcnt:
	$(MAKE) EXTRACFLAGS='$(EXTRACFLAGS) -DHAS_POPCNT -mpopcnt' cinnamon64

all: _init_ namespaces.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Hash.o Uci.o OpenBook.o
	$(CC) ${CFLAGS} -o ${EXE} namespaces.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Hash.o Uci.o OpenBook.o ${LIBS} ${ARC}
	$(STRIP) $(EXE)

namespaces.o: namespaces.cpp
	$(CC) -c namespaces.cpp ${CFLAGS} ${DEF} ${ARC}

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

Uci.o: Uci.cpp
	$(CC) -c Uci.cpp ${CFLAGS} ${DEF} ${ARC}

Hash.o: Hash.cpp
	$(CC) -c Hash.cpp ${CFLAGS} ${DEF} ${ARC}

OpenBook.o: OpenBook.cpp
	$(CC) -c OpenBook.cpp ${CFLAGS} ${DEF} ${ARC}

clean:
	$(RM) *.o $(EXE) *.gcda

