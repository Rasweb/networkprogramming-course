#! /bin/bash
for ((n=1;n<12;n++));
do 
   (echo "test $n" | nc 127.0.0.1 8080) &
done
