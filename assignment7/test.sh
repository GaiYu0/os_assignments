for ((i=0; i!=$1; i++))
do
  file="server$i.c"
  path="source/$file"
  echo $path
  cp server.c $path
  ./client default default upload $path $file
done

rm source/*

for ((i=0; i!=$1; i++))
do
  file="server$i.c"
  path="source/$file"
  echo $path
  cp server.c $path
  ./client default default download $file $path
done

#for ((i=0; i!=$1; i++))
#do
#  ./client default default execute ls
#done