array=( 192.168.1.100 192.168.1.101 192.168.1.102 192.168.1.106 192.168.1.108 )

make clean;make cinnamon-ARM COMP=arm-linux-gnueabihf-g++-4.9
rm -fr /tmp/deploy/
UUID=$(cat /proc/sys/kernel/random/uuid)
mkdir -p /tmp/deploy/$UUID
cp -r ../src /tmp/deploy/$UUID
cd /tmp/deploy
tar -czf $UUID.tar.gz $UUID
./cinnamon -b
echo "send to servers ? y/n"
read send
if [ "$send" != "y" ]; then
	exit 0;
fi
for ip in "${array[@]}"
do	
	echo $ip
	scp /tmp/deploy/$UUID.tar.gz geko@$ip:test
done
exit 0





