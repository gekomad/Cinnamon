array=( 192.168.1.100 192.168.1.101 192.168.1.102 192.168.1.106 192.168.1.108 )

rm -fr /tmp/deploy/
UUID=$(cat /proc/sys/kernel/random/uuid)
mkdir -p /tmp/deploy/$UUID
cp -r ../src /tmp/deploy/$UUID
cd /tmp/deploy
rm -fr $UUID/src/*.o $UUID/src/gtb/Linux $UUID/src/gtb/Windows $UUID/src/gtb/OSX
tar -czf $UUID.tar.gz $UUID

for ip in "${array[@]}"
do	
	echo $ip
	scp /tmp/deploy/$UUID.tar.gz geko@$ip:test
done
exit 0





