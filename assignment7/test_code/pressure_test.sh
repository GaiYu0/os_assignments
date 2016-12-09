#for ((i=0; i!=$1; i++))
#do
#  bin/client default default upload src &
#done
#wait

mkdir test

for ((i=0; i!=$1; i++))
do
  file="server$i"
  cp ./bin/server test/$file
  ./bin/client default default upload test/$file &
done
wait

rm -r test

for ((i=0; i!=$1; i++))
do
  file="server$i"
  ./bin/client default default download $file &
done
wait
