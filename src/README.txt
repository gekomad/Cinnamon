-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --Compile process has two steps:

1 ) Profiling 2 ) Release In Windows use http:	//tdm-gcc.tdragon.net, Makefile assume to be installed in C:\tdmmingw

To build butterfly on 32 - bits architecture:
compile with "make butterfly32-profile" and play a match with butterfly_profiling using a GUI ( winboard or arena ), this generate.gcda files rebuild with "make butterfly32-release" to generate butterfly delete butterfly_profiling to build on 64 - bits architecture substitute 32 with 64 at previous steps-- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
