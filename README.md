
Cinnamon  
==========  
###UCI Chess Engine  
  
Cinnamon is a chess program for Windows, Linux, Mac OS, Android and Raspberry Pi, is a console-based chess engine for use with  
[xboard][4],  
[Arena][5],  
[Tarrasch][6],  
[Chess for Android][7]  
or any UCI-compatible GUI. Cinnamon is also a javascript library to play with  
[chessboardjs][8] or any js GUI.  
  
Version  
----------  
2.1  
  
Features  
----------  
            
- Available for both Unix, Windows, Mac, Android, ARM and Javascript  
- UCI protocol  
- GPL 3 License  
- C++11 source  
- Rotated bitboards  
- Null moves  
- Futility pruning  
- Delta pruning  
- Razoring  
- Interruptible multithread Perft test  
- 32/64 bit architectures  
- Iterative deeping  
- Killer heuristics  
- Lazy evaluation  
- Mvv/Lva  
- Transposition Table  
- Aspiration Windows  
- Late Move Reduction  
- Ponder  
- Available with Tarrasch GUI for Windows  
- Open Book (Polyglot)  
- Gaviota Tablebases  
- [Elo ratings][3]  
  
Binaries  
----------  
  
Binaries are available [here][1].  
All files are compiled statically, no further libraries are necessary.  
  
Command line tools  
----------  
#### Perft  
`cinnamon -perft [-d depth] [-c nCpu] [-h hash size (mb) [-F dump file]] [-f "fen position"] `

Setting dump file (-F) you can stop and start the perft process.

#### Gaviota distance to mate

`cinnamon -dtm-gtb -f "fen position" -p path`

#### EPD generation
`cinnamon -puzzle_epd -t K..K..`
 example: `cinnamon -puzzle_epd -t KRKP`

Compiling  
---------  
  
Cinnamon requires C++11 or greater, use unique Makefile to compile for many architectures:  
  
    $ make  
  
    Makefile for cross-compile Linux/Windows/OSX/Raspberry  
  
    make cinnamon64-modern-INTEL     > 64-bit optimized for modern Intel cpu  
    make cinnamon64-modern-AMD       > 64-bit optimized for modern Amd cpu  
    make cinnamon64-modern           > 64-bit with popcnt bsf sse3 support  
    make cinnamon64-generic          > Unspecified 64-bit  
    make cinnamon64-ARM              > Optimized for ARM cpu  
  
    make cinnamon32-modern           > 32-bit with sse support  
    make cinnamon32-generic          > Unspecified 32-bit  
    make cinnamon32-ARM              > Optimized for ARM cpu  
  
    g++ is the default compiler, add COMP=yourcompiler to use another compiler  
     example: make cinnamon64-modern-INTEL COMP=clang++  
  
  
License  
-------  
  
Cinnamon is released under the GPLv3+ license.  
  
Credits  
-------  
  
Cinnamon was written by Giuseppe Cannella at gmail dot com.  
  
  [1]: http://cinnamonchess.altervista.org  
  [3]: http://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?family=Cinnamon  
  [4]: http://www.gnu.org/software/xboard  
  [5]: http://www.playwitharena.com  
  [6]: http://triplehappy.com  
  [7]: https://play.google.com/store/apps/details?id=com.google.android.chess  
  [8]: http://chessboardjs.com