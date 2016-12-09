#for ((i=0; i!=$1; i++))
#do
#  bin/client default default upload src &
#done
#wait

for ((i=0; i!=$1; i++))
do
  file="makefile$i"
  cp makefile test/$file
  bin/client default default upload test/$file
done
wait

rm test/*

for ((i=0; i!=$1; i++))
do
  file="makefile$i"
  bin/client default default download $file test/$file &
done
wait
