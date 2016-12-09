for ((i=0; i!=$1; i++))
do
  file="makefile$i"
  cp makefile test/$file
  bin/client default default upload makefile $file &
# bin/client default default execute ls
done
wait

#rm test/*
#
#for ((i=0; i!=$1; i++))
#do
#  file="makefile$i"
#  bin/client default default download $file test/$file &
#done
#wait
