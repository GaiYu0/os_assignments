rm -f file/makefile
for ((i=0; i!=$1; i++)) 
do
  ./bin/client default default upload makefile &
done
wait
cat file/makefile
