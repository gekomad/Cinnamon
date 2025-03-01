Cinnamon
==========
### UCI Chess Engine

Cinnamon is a chess program for Windows, Linux, Mac OS, Android and Raspberry Pi, is a console-based chess engine for use with [xboard][4], [Arena][5], [Tarrasch][6] or any UCI-compatible GUI. Cinnamon is also a javascript library to play with [chessboardjs][7] or any js GUI.

- [Play on line](https://gekomad.github.io/Cinnamon/)

Version
----------
2.5

News
----------
- Time management
- Tapered eval

Useful links
----------
- [Bitboard Calculator](https://gekomad.github.io/Cinnamon/BitboardCalculator/)
- [Web Eval debugger](https://github.com/gekomad/chess-engine-eval-debugger)
- [Cute Cluster Chess](https://github.com/gekomad/cute_cluster_chess)
- [Perft results](https://gekomad.github.io/Cinnamon/perft.html)
- [Perft 960 results](https://www.chessprogramming.org/Chess960_Perft_Results)


Other links
----------
- [Auriga Distributed Perft](https://github.com/gekomad/Auriga)
- [Cinnamon on Android](https://play.google.com/store/apps/details?id=com.github.gekomad.cinnamonengine)
- [Cinnamon on Android with GUI](https://play.google.com/store/apps/details?id=com.github.gekomad.chess_master)
- [Cinnamon on DGT Pi](https://www.digitalgametechnology.com/index.php/products/revelation-ii/533-dgt-pi-chess-computer-for-dgt-e-boards)
- [Janggi Master (korean chess)](https://play.google.com/store/apps/details?id=com.github.gekomad.rangy_app)
- [Xiangqi Master (chinese chess)](https://play.google.com/store/apps/details?id=com.github.gekomad.rangyxx_app)
- [Rangy Master](https://gekomad.github.io/rangy_master/index.html)

Features
----------

- [Elo ratings][3]
- Available for both Unix, Windows, Mac, Android, ARM and Javascript
- UCI protocol
- GPL 3 License
- Chess 960
- C++11 source
- Rotated bitboards
- Interruptible multithread Perft test
- Ponder
- Gaviota Tablebases
- Syzygy Tablebases

Binaries
----------

Binaries are available [here][1].
All files are compiled statically, no further libraries are necessary.

Command line tools
----------
#### Perft
`cinnamon.exe -perft [-d depth] [-c nCpu] [-h hash size (mb) [-F dump file]] [-Chess960] [-f "fen position"] `

Setting `-F` and `-h` you can stop (Ctrl-c) and restart the perft process.


#### Gaviota DTM (distance to mate)

`cinnamon.exe -dtm-gtb -f "fen position" -p path`

#### Gaviota WDL (win/draw/loss)

`cinnamon.exe -wdl-gtb -f "fen position" -p path`

#### SYZYFY DTZ (distance to zero)

`cinnamon.exe -dtz-syzygy -f "fen position" -p path`

#### SYZYFY WDL (win/draw/loss)

`cinnamon.exe -wdl-syzygy -f "fen position" -p path`

#### EPD generator
`cinnamon.exe -puzzle_epd -t K?K?`

 example: `cinnamon.exe -puzzle_epd -t KRRKPN`

Compiling
---------

Cinnamon requires C++11 or greater

- use MS Visual Studio (cinnamon.vcxproj)

- use cmake (CMakeLists.txt)

- use make - unique Makefile to compile for many architectures:


```
    $ make


    Makefile for cross-compile Linux/Windows/OSX/ARM/Javascript

    make cinnamon64-modern-INTEL     > 64-bit optimized for modern Intel cpu
    make cinnamon64-BMI2             > 64-bit optimized for Haswell Intel cpu
    make cinnamon64-modern-AMD       > 64-bit optimized for modern Amd cpu
    make cinnamon64-modern           > 64-bit with popcnt bsf sse3 support
    make cinnamon64-generic          > Unspecified 64-bit
    make cinnamon64-ARM              > Optimized for ARM cpu

    make cinnamon32-modern           > 32-bit with sse support
    make cinnamon32-generic          > Unspecified 32-bit
    make cinnamon32-ARM              > Optimized for ARM cpu

    make cinnamon-js                 > Javascript build

    add:
     COMP=compiler                   > Use another compiler
     FULL_TEST=yes                   > Unit test (googletest)


    g++ is the default compiler, add COMP=yourcompiler to use another compiler
     example: make cinnamon64-modern-INTEL COMP=clang++

```

License
-------

Cinnamon is released under the GPLv3+ license.

Credits
-------

Cinnamon was written by Giuseppe Cannella at gmail dot com.

  [1]: https://github.com/gekomad/Cinnamon/releases
  [3]: https://www.computerchess.org.uk/ccrl/404/cgi/compare_engines.cgi?family=Cinnamon
  [4]: https://www.gnu.org/software/xboard
  [5]: http://www.playwitharena.de
  [6]: https://triplehappy.com
  [7]: https://chessboardjs.com


