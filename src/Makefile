CPP  = g++
OBJ  = cap.o eval.o butterfly.o gen.o openbook.o search.o test.o utility.o winboard.o
LINKOBJ  = cap.o eval.o butterfly.o gen.o openbook.o search.o test.o utility.o winboard.o
LIBS =  -lpthread 
BIN  = butterfly
CXXFLAGS = -DNULL_MOVEx -DFP_MODEx -DPERFT_MODEx -DHASH_MODEx -DHAS_64BITSx -DTEST_MODEx -DDEBUG_MODEx  -w  -fno-strict-aliasing -O3
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before butterfly all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o "butterfly" $(LIBS)

cap.o: cap.cpp
	$(CPP) -c cap.cpp -o cap.o $(CXXFLAGS)

eval.o: eval.cpp
	$(CPP) -c eval.cpp -o eval.o $(CXXFLAGS)

butterfly.o: butterfly.cpp
	$(CPP) -c butterfly.cpp -o butterfly.o $(CXXFLAGS)

gen.o: gen.cpp
	$(CPP) -c gen.cpp -o gen.o $(CXXFLAGS)

openbook.o: openbook.cpp
	$(CPP) -c openbook.cpp -o openbook.o $(CXXFLAGS)

search.o: search.cpp
	$(CPP) -c search.cpp -o search.o $(CXXFLAGS)

test.o: test.cpp
	$(CPP) -c test.cpp -o test.o $(CXXFLAGS)

utility.o: utility.cpp
	$(CPP) -c utility.cpp -o utility.o $(CXXFLAGS)

winboard.o: winboard.cpp
	$(CPP) -c winboard.cpp -o winboard.o $(CXXFLAGS)
