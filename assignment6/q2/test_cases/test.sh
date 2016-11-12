# setup
rm -f -r files
mkdir files
args=()
for ((count=0; count != $1; count++))
do
  let index=$count%3
  from=test_cases/p$index
  to=files/p$count
  cp $from $to
  args+=($to)
done

# convert
./q2 ${args[@]}

# print converted files
for ((count=0; count != $1; count++))
do
  cat files/p$count
done
