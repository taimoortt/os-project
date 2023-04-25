#!/bin/bash

for i in $(seq 0 0); do
  #../LTE-Sim MultiCell 7 1 20 0 0 1 0 8 1 30 0.1 128 ./1slicepf_logs/ue140_b1.json $i > ./1slicepf_edge_logs/doublec_baseline_${i}.log 2> /dev/null &
  ../LTE-Sim MultiCell 7 1 20 0 0 1 0 8 1 3 0.1 128 ./1slicepf_logs/ue140_b5.json $i > ./1slicepf_edge_logs/debug_speed3_muteglobal_${i}.log 2> /dev/null &
done
