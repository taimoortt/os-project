#!/bin/bash

# There are only two cells, with 20 ues in total

for i in $(seq 0 4); do
  ../LTE-Sim MultiCell 2 1 10 0 0 1 0 8 1 30 0.1 128 ./configs/micro_b1.json $i > ./rural_logs/micro1_${i}.log 2> /dev/null &
  ../LTE-Sim MultiCell 2 1 10 0 0 1 0 8 1 30 0.1 128 ./configs/micro_b5.json $i > ./rural_logs/micro5_${i}.log 2> /dev/null &
done
