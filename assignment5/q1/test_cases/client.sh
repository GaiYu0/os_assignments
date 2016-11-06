for ((i=0; i!=4; i++))
do
  counter=0
  for from in EUR RMB USD
  do
    for to in EUR RMB USD
    do
      ../bin/client $i $counter $from $to 3.141592 &
      let counter+=1
    done
  done
done
wait
