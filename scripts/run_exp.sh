#!/bin/bash

for i in $(seq 0 2); do
  ../LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 30 0.1 128 ./baseline1.json $i > baseline1_${i}.log 2> /dev/null &
  ../LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 30 0.1 128 ./baseline2.json $i > baseline2_${i}.log 2> /dev/null &
  ../LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 30 0.1 128 ./baseline5.json $i > baseline5_${i}.log 2> /dev/null &
done
