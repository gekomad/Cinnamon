engine=./cinnamon
dir_log=/home/geko/perft

mkdir -p $dir_log

$engine -perft -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 25" -c2 -h5000 -d8 -F$dir_log/pos2.bin 2>&1 |tee -a  $dir_log/pos2.log
$engine -perft -c2 -h5000 -d10 -F$dir_log/pos1.bin 2>&1 |tee -a  $dir_log/pos1.log
$engine -perft -c2 -h5000 -d11 -F$dir_log/pos1.bin 2>&1 |tee -a  $dir_log/pos1.log
$engine -perft -f"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 25" -c2 -h5000 -d9 -F$dir_log/pos2.bin 2>&1 |tee -a  $dir_log/pos2.log
