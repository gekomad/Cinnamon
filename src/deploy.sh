make clean
cd ..
tar -czf src.tar.gz src

array=( 192.168.1.100 192.168.1.101 192.168.1.102 192.168.1.106 192.168.1.108 )

for ip in "${array[@]}"
do
	scp src.tar.gz  geko@$ip:
	ssh geko@$ip /home/geko/run_match.sh >/dev/null &
done
exit 0
