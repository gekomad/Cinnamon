
Cinnamon chess engine - Tests
=============================

Cinnamon 2.5
------------

**Linux - Intel i7-9750H CPU @ 2.60GHz gcc 9.3.0** 
 

| Test file (5 sec) | Solved |
|--|--|
|  wac.epd        |     290/300
|  kaufman.epd    |     21/25
|  zugzwang.epd   |     3/5
|  bk.epd         |     16/24
|  mate.epd       |     51/71
  ------------------- ---------

**perft** depth 6 one cpu no hash

 ./cinnamon -perft -d6 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

** 47 sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

** 1.08 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**  sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

** 1.08 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

** 3.49 sec.**

Cinnamon 2.4
------------

**Linux - Intel i7-9750H CPU @ 2.60GHz gcc 9.3.0** 
 

| Test file (5 sec) | Solved |
|--|--|
|  wac.epd        |     293/300
|  kaufman.epd    |     23/25
|  zugzwang.epd   |     2/5
|  bk.epd         |     16/24
|  mate.epd       |     52/71
  ------------------- ---------

**perft** depth 6 one cpu no hash

 ./cinnamon -perft -d6 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**50 sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**1.20 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**3.41 sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**1.13 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000 -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**3.608 sec.**

Cinnamon 2.3.4
--------------

**Linux - Intel i7-9750H CPU @ 2.60GHz gcc 9.3.0** 
 


| Test file (5 sec) | Solved |
|--|--|
|wac.epd|284/300|
|kaufman.epd|23/25|
|zugzwang.epd|2/5|
|bk.epd|17/24|
|mate.epd|53/71|
  ------------------- ---------

**perft** depth 6 one cpu no hash

 ./cinnamon -perft -d6
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**153 sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**3 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**8 sec.**

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**4 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**9 sec.**

Cinnamon 2.2a
-------------

**Linux - Intel i7-9750H CPU @ 2.60GHz gcc 9.3.0** 

| Test file (5 sec) | Solved |
|--|--|
| wac.epd |            278/300|
  |kaufman.epd|         23/25|
 | zugzwang.epd|        2/5|
 | bk.epd       |       17/24|
|  mate.epd      |      55/71|
  ------------------- ---------

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**4 sec.**

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**9 sec.**

Cinnamon 2.2a
-------------

**Linux - Intel Xeon W3540 gcc 7.3.0** 
 
| Test file (5 sec) | Solved |
|--|--|
|  wac.epd                                                  | 278/300|
|  arasan12.epd                                             | 15/215|
|  sbd.epd                                                  | 55/134|
|  kaufman.epd                                              | 21/25|
|  zugzwang.epd                                             | 1/5|
|  bk.epd                                                   | 16/24|
|  mate.epd                                                 | 48/71|
  |[BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm)  | 2276|
  --------------------------------------------------------- ---------

  tc = 0/0:9+0.05 - 1 cpu
  Rank Name       Elo   +   -   games   score   oppo.   draws
  cinnamon 2.2a   34    2   4   59196   55%     -19     27%
  cinnamon 2.1    -34   4   2   59196   45%     19      27%
 --------------- ----- --- --- ------- ------- ------- -------
 tc = 0/0:9+0.05 - 4 cpu
  Rank Name       Elo   +   -   games   score   oppo.   draws
  cinnamon 2.2a   34    2   4   16272   55%     -34     21%
  cinnamon 2.1    -34   4   2   16272   45%     34      21%
  --------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**8 sec.**


**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**22 sec.**

Cinnamon 2.1
------------

| Test file (5 sec) | Solved (Xeon W3540) |
|--|--|
 | Test file (5 sec)           |                              Solved (Xeon W3540)|
  |wac.epd                        |                           277/300|
  |arasan12.epd             |                                13/215|
  |sbd.epd                       |                            52/134|
  |kaufman.epd               |                                22/25|
  |zugzwang.epd            |                                  3/5|
  |bk.epd                           |                         16/24|
  |mate.epd                     |                             57/71|
|  [BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm)   |2282|
  --------------------------------------------------------- ---------------------

  tc = 0/0:9+0.05 - 1 cpu
  Rank Name      Elo   +   -   games   score   oppo.   draws
  cinnamon 2.1   34    2   4   59196   59%     -34     21%
  cinnamon 2.0   -34   4   2   59196   41%     34      21%
  -------------- ----- --- --- ------- ------- ------- -------

 tc = 0/0:9+0.05 - 4 cpu
  Rank Name               Elo    +   -   games   score   oppo.   draws
  cinnamon 2.1 (4 core)   49     3   2   33980   64%     -49     26%
  cinnamon 2.0 (1 core)   -49   2   3   33980   36%     49      26%
                                                                 
  ----------------------- ------ --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**8 sec.** Linux - Intel Xeon W3540 gcc 7.3.0

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**22 sec.** Intel Xeon W3540 Linux gcc 7.3.0

Cinnamon 2.0
------------

| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
|wac.epd|276/300|
|arasan12.epd | 14/215|
|sbd.epd| 46/134|
|kaufman.epd |20/25|
|zugzwang.epd | 3/5|
|bk.epd|17/24|
|mate.epd | 57/71|
|[BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm)| 2240|
  --------------------------------------------------------- ---------------------

**4 core**

| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
|wac.epd|277/300|
|arasan12.epd | 14/215|
|sbd.epd| 45/134|
|kaufman.epd |21/25|
|zugzwang.epd | 3/5|
|bk.epd|18/24|
|mate.epd | 58/71|
|[BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm)| 2220|

  tc = 40/4 - 1 cpu
  Rank Name       Elo    +   -   [games](http://cinnamonchess.altervista.org/pgn/cinnamon2.0-single-core.pgn.7z)   score   oppo.   draws
  cinnamon 2.0    2059   4   4   7589                                                                              60%     1989    33%
  cinnamon 1.2b   1989   4   4   7857                                                                              40%     2059    33%
  --------------- ------ --- --- -------------------------------------------------------------------

 tc = 40/4 - 4 cpu 
  Rank Name                Elo     +   -   [games](http://cinnamonchess.altervista.org/pgn/cinnamon2.0-quad-core.pgn.7z)   score   oppo.   draws
  cinnamon 2.0 (4 core)    2094    6   6   3236                                                                            66%     1989    33%
  cinnamon 1.2b (1 core)   1989   6   6   3236                                                                            34%     2094    33%                                                                                                                                          


---------------
 tc = 40/4 - 4 cpu
 Rank Name               Elo   +    -    [games](http://cinnamonchess.altervista.org/pgn/cinnamon2.0-quad-core-bis.pgn.7z)   score   oppo.   draws
  cinnamon 2.0 (4 core)        6   5   4662                                                                                66%            33% 
  cinnamon 2.0 (1 core)        5   6   4662                                                                                34%           33%
                                                                                                                                              
  ----------------------- ----- ---- ---- ----------------------------------------------------------------------------------- ------- -

**perft** depth 5 one cpu no hash

 ./cinnamon -perft -d5
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**11 sec.** Linux - Intel Xeon W3540 gcc 4.9.2

**11 sec.** Windows 7 - Intel Xeon W3540 gcc 5.1.0

**perft** depth 6 8 cpu 2 GB hash

 ./cinnamon -perft -d6 -c8 -h2000
-f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**29 sec.** Intel Xeon W3540 Linux gcc 4.9.2

**30 sec.** Intel Xeon W3540 Windows 7 gcc 5.1.0

cinnamon 1.2a/1.2b
------------------

| Test file (5 sec) | Solved (i5 M430)|Solved (Xeon W3540)|
|--|--|--|
|wac.epd|273/300|271/300
|arasan12.epd|15/215|14/215
|sbd.epd|46/134|48/134
|kaufman.epd|20/25|19/25
|zugzwang.epd|2/5|2/5
|bk.epd|16/24|17/24
|mate.epd|55/71|58/71
|[BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm)|-|2180
  --------------------------------------------------------- ------------------ ---------------------

  tc = 40/4, no tablebase, no open book
  Rank Name            Elo    +    -    games   score   oppo.   draws
  cinnamon 1.2a/1.2b   14    5   5   4740    54%    -14%    32%                                                                
  cinnamon 1.1c        -14   5   5   4740    46%    14%    32%
                                                                
  -------------------- ------ ---- ---- ------- ------- ------- -------

 tc = 40/4, tablebase, open book
  Rank Name            Elo    +    -    games   score   oppo.   draws
  cinnamon 1.2a/1.2b   21    6   5   4244    56%    -21%    28%  
  cinnamon 1.1c        -21   5   6   4244    44%    21%    28%                                                                
  -------------------- ------ ---- ---- ------- ------- ------- -------


**perft** one cpu no hash fen

 ./cinnamon -perft -d 5 -f
"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**19 sec.** Intel i5 CPU M 430 2.27GHz Linux 3.2.0-57-generic gcc 4.8.2

**12 sec.** Intel Xeon W3540 Linux 3.13.0-37-generic gcc 4.8.2

**perft** 8 cpu 2 GB hash

 ./cinnamon -perft -d 5 -f
"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"

**?** Intel Xeon W3540 Linux 3.13.0-37-generic gcc 4.8.2

cinnamon 1.1c
-------------


| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
  |wac.epd |277/300
 |arasan12.epd |16/215
 |sbd.epd |40/134
 |kaufman.epd |20/25
 |zugzwang.epd |3/5
 |bk.epd |17/24
 |mate.epd |57/71
 |[BT2450.epd](http://www.rebel.nl/rebelfaq/rc3faq03.htm) |2252
  --------------------------------------------------------- ---------


  Rank Name       Elo   +   -   games   score   oppo.   draws
  cinnamon 1.1c              4000                  61%
  cinnamon 1.0               4000                  39%
                                                        
  --------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**17 sec.**

cinnamon 1.0
------------


| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
 |wac.epd |277/300
 |arasan12.epd |17/215
 |sbd.epd |67/134
 |kaufman.epd |17/25
 |zugzwang.epd |2/5
 |bk.epd |14/24
 |mate.epd |56/71
 |BT2450.epd |2222

  --------------- ----- --- --- ------- ------- ------- -------
  Rank Name       Elo   +   -   games   score   oppo.   draws
  cinnamon 1.0    49    6   6   8000    65%     -49     41%
  butterfly 0.6   -49   6   6   8000    35%     49      41%
  --------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**22 sec.**

butterfly 0.6
-------------

| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
|wac.epd|273/300
|arasan12.epd|14/215
|sbd.epd|40/134
|kaufman.epd|13/25
|zugzwang.epd|2/5
|bk.epd|12/24
|mate.epd|56/71
|BT2450.epd|1847

  --------------- ----- --- --- ------- ------- ------- -------
  Rank Name       Elo   +   -   games   score   oppo.   draws
  butterfly 0.6   40    6   6   7919    64%     -40     55%
  butterfly 0.5   -40   6   6   7919    36%     40      55%
  --------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**28 sec.**

butterfly 0.5
-------------


| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
 |wac.epd |266/300
 |arasan12.epd |16/215
 |sbd.epd |39/134
 |kaufman.epd |8/25
 |zugzwang.epd |3/5
 |bk.epd |12/24
 |mate.epd |67/71
 |BT2450 |1806

  --------------- ----- --- --- ------- ------- ------- -------
  Rank Name       Elo   +   -   games   score   oppo.   draws
  butterfly 0.5   32    6   6   6820    62%     -32     65%
  butterfly 0.4   -32   6   6   6820    38%     32      65%
  --------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**23 sec.**

butterfly 0.4
-------------


| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
 |wac.epd |239/300
 |arasan12.epd |22/215
 |sbd.epd |38/134
 |kaufman.epd |7/25
 |zugzwang.epd |3/5
 |bk.epd |9/24
 |mate.epd |67/71

  ----------------- ----- --- --- ------- ------- ------- -------
  Rank Name         Elo   +   -   games   score   oppo.   draws
  butterfly 0.4     48    6   6   7991    66%     -48     52%
  butterfly 0.3.3   -48   6   6   7991    34%     48      52%
  ----------------- ----- --- --- ------- ------- ------- -------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**32 sec.**

butterfly 0.3.3
---------------


| Test file (5 sec) | Solved (Xeon W3540)|
|--|--|
 |wac.epd |228/300
 |arasan12.epd |20/215
 |sbd.epd |40/134
 |kaufman.epd |6/25
 |zugzwang.epd |3/5
 |bk.epd |9/24
 |mate.epd |66/71
  ------------------- ---------

**perft** depth 5 one cpu no hash fen
r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -

on i5 CPU M 430 2.27GHz Linux 3.2.0-48-generic gcc 4.8.1

**31 sec.**
