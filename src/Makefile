#Makefile for cross-compile Linux/Windows
#In Windows use http://tdm-gcc.tdragon.net, assume to be installed in "C:\tdmmingw"

LIBS=-lpthread
CFLAGS_PROFILE="-w -O3 -fomit-frame-pointer -momit-leaf-frame-pointer -std=gnu99 -ftree-loop-distribution -fprofile-generate"
CFLAGS_RELEASE="-w -O3 -fomit-frame-pointer -momit-leaf-frame-pointer -std=gnu99 -ftree-loop-distribution -fprofile-use"

_init_:
ifndef CFLAGS
	@echo "*** ERROR TARGET IS NOT DEFINED***"
	@more README.txt
	false
endif
ifndef SystemRoot
RM=rm -f
EXE_PROFILE="butterfly_profile"
EXE_RELEASE="butterfly"
STRIP=strip
else
EXE_PROFILE="butterfly_profile.exe"
EXE_RELEASE="butterfly.exe"
STRIP=C:\MinGW64\bin\strip.exe
RM=del
endif
all: butterfly

butterfly32-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DNULL_MODE -DFP_MODE" ARC="-m32" \
	all
butterfly64-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DNULL_MODE -DFP_MODE -DHAS_64BITS" ARC="-m64" \
	all
perft32-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DPERFT_MODE" ARC="-m32" \
	all
perft64-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DPERFT_MODE -DHAS_64BITS" ARC="-m64" \
	all
test32-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DNULL_MODE -DFP_MODE -DTEST_MODE" ARC="-m32" \ 
	all
test64-profile:
	$(MAKE) EXE=$(EXE_PROFILE) CFLAGS=$(CFLAGS_PROFILE) DEF="-DNULL_MODE -DFP_MODE -DTEST_MODE -DHAS_64BITS" ARC="-m64" \
	all


butterfly32-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DNULL_MODE -DFP_MODE" ARC="-m32" \
	clean \
	all
butterfly64-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DNULL_MODE -DFP_MODE -DHAS_64BITS" ARC="-m64" \
	clean \
	all
perft32-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DPERFT_MODE" ARC="-m32" \
	clean \
	all
perft64-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DPERFT_MODE -DHAS_64BITS" ARC="-m64" \
	clean \
	all
test32-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DNULL_MODE -DFP_MODE -DTEST_MODE" ARC="-m32" \
	clean \
	all
test64-release:
	$(MAKE) EXE=$(EXE_RELEASE) CFLAGS=$(CFLAGS_RELEASE) DEF="-DNULL_MODE -DFP_MODE -DTEST_MODE -DHAS_64BITS" ARC="-m64" \
	clean \
	all
butterfly: _init_ Bits.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Test.o Uci.o
	g++ ${CFLAGS} -o ${EXE} Bits.o ChessBoard.o Eval.o GenMoves.o IterativeDeeping.o main.o Perft.o Search.o Test.o Uci.o ${LIBS} ${ARC}
	$(STRIP) $(EXE)
Bits.o: Bits.cpp
	g++ -c Bits.cpp ${CFLAGS} ${DEF} ${ARC}	

ChessBoard.o: ChessBoard.cpp
	g++ -c ChessBoard.cpp ${CFLAGS} ${DEF} ${ARC}

Eval.o: Eval.cpp
	g++ -c Eval.cpp ${CFLAGS} ${DEF} ${ARC}

GenMoves.o: GenMoves.cpp
	g++ -c GenMoves.cpp ${CFLAGS} ${DEF} ${ARC}
IterativeDeeping.o: IterativeDeeping.cpp
	g++ -c IterativeDeeping.cpp ${CFLAGS} ${DEF} ${ARC}
main.o: main.cpp
	g++ -c main.cpp ${CFLAGS} ${DEF} ${ARC}
Perft.o: Perft.cpp
	g++ -c Perft.cpp ${CFLAGS} ${DEF} ${ARC}
Search.o: Search.cpp
	g++ -c Search.cpp ${CFLAGS} ${DEF} ${ARC}
Test.o: Test.cpp
	g++ -c Test.cpp ${CFLAGS} ${DEF} ${ARC}
Uci.o: Uci.cpp
	g++ -c Uci.cpp ${CFLAGS} ${DEF} ${ARC}

clean:
	$(RM) *o $(EXE_PROFILE)

