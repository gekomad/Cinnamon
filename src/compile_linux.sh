g++ -w -fno-strict-aliasing -O3 *.cpp -o butterfly -lpthread -DNULL_MODE -DFP_MODE -DHAS_64BITS
#		add these parameters to customize it:
#		-DNULL_MODE (enable null move)
#		-DFP_MODE (enable futility pruning)
#		-DPERFT_MODE (run perft test)
#		-DHASH_MODE (enable hash table)
#		-DHAS_64BITS (64 bit architectures)
#		-DTEST_MODE (test mode)
#		-DDEBUG_MODE (debug mode)
#valgrind:	g++ -w -fno-strict-aliasing -g *.cpp -o butterfly -lpthread -DDEBUG_MODE; valgrind --leak-check=yes --leak-check=full --show-reachable=yes ./butterfly
