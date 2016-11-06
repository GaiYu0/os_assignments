for ((i=0; i!=4; i++))
do
  ../bin/server $i &
done
wait
