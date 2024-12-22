#!/bin/bash

NUM_NODES=80
TRAFFIC_LOADS=(1 5 10 15 20 25 30)
SEEDS=(1 2 3 4 5 6 7 8 9 10)
OUTPUT_DIR="./traffic_load/results"  

mkdir -p "$OUTPUT_DIR"

# Function to generate source and destination pairs based on traffic load
generate_traffic_pairs() {
  traffic_load=$1
  pairs=""
  for (( i=0; i<$traffic_load; i++ )); do
    source_node=$i
    destination_node=$(( 79 - $i ))
    pairs+="$source_node:$destination_node,"
  done
  pairs=${pairs%,}
  echo $pairs
}

# Function to run simulation
run_simulation() {
  traffic_load=$1
  seed=$2
  traffic_pairs=$(generate_traffic_pairs $traffic_load)
  output_file="${OUTPUT_DIR}/output_${traffic_load}_${seed}.tr"

  echo "Running simulation with traffic load ${traffic_load}, seed ${seed}, and traffic pairs ${traffic_pairs}"

  ./ns3 run "traffic_load/Task2.cc --trafficLoad=$traffic_load --seed=$seed --trafficPairs=$traffic_pairs" > "$output_file"
}

# Iterate over traffic loads and seeds to run simulations
for traffic_load in "${TRAFFIC_LOADS[@]}"; do
  for seed in "${SEEDS[@]}"; do
    run_simulation $traffic_load $seed
  done
done

echo "All simulations completed and trace files stored in $OUTPUT_DIR."

