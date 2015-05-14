g++ -w -fno-strict-aliasing -O3 *.cpp -o butterfly -lpthread
#		add these parameters to customize it:
#		-DNULL_MOVE (enable null move)
#		-DFP_MODE (enable futility pruning)
#		-DPERFT_MODE (run perft test)
#		-DHASH_MODE (enable hash table)
#		-DHAS_64BITS (64 bit architectures)
#		-DTEST_MODE (test mode)
#		-DDEBUG_MODE (debugmode)

