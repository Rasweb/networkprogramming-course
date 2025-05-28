#! /bin/bash
for((n=1;n<11;n++));
do
  (echo "test $n" | nc 127.0.0.1 8080) &
  sleep 1
done
