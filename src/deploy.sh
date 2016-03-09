array=( $IPS )

make clean
rm -fr /tmp/deploy/
UUID=$(cat /proc/sys/kernel/random/uuid)
mkdir -p /tmp/deploy/$UUID
cp -r ../src /tmp/deploy/$UUID
cd /tmp/deploy
find $UUID/src/ -type f \( -name "*.html" -o -name "*.ods" -o -name "*.sh" -o -name "*.txt" \) -delete

rm -fr $UUID/src/gtb/Linux $UUID/src/.idea $UUID/src/js $UUID/src/gtb/Windows $UUID/src/gtb/OSX 
tar -czf $UUID.tar.gz $UUID

for ip in "${array[@]}"
do	
	ssh geko@$ip "mkdir test" 2>/dev/null
	echo "scp /tmp/deploy/$UUID.tar.gz geko@$ip:test"
	scp /tmp/deploy/$UUID.tar.gz geko@$ip:test
done
cd ~/workspace/assembla_git/chess-utility/script/remote
./cute.sh
date
exit 0


 


