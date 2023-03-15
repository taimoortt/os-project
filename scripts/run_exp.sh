#!/bin/bash

for i in $(seq 0 2); do
  ../LTE-Sim MultiCell 7 1 20 0 0 1 0 8 1 30 0.1 128 ./configs/4slice_b1.json $i > ./rural_logs/4slice1_${i}.log 2> /dev/null &
  ../LTE-Sim MultiCell 7 1 20 0 0 1 0 8 1 30 0.1 128 ./configs/4slice_b5.json $i > ./rural_logs/4slice_2times5_${i}.log 2> /dev/null &
done
