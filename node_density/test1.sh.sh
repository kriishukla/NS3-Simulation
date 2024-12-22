#!/bin/bash

runNum=10
base_path="./node_density/results"

mkdir -p $base_path  # Ensure results directory exists

for numNodesVal in 10 20 30 40 50 60 70 80 90 100
do
    sourceNodeVal=1
    destinationNodeVal=$(($numNodesVal - 1))
    
    for seedVal in {1..10}
    do
        echo "Running simulation for Source Node: $sourceNodeVal, Destination Node: $destinationNodeVal, Seed: $seedVal"
        
        ./ns3 run "node_density/Task1.cc --seed=$seedVal --destinationNode=$destinationNodeVal --sourceNode=$sourceNodeVal --numNodes=$numNodesVal"
      
        output_file=$base_path/output_${numNodesVal}_${seedVal}.tr
        if test -f "$output_file"; then
            echo "Trace file $output_file generated successfully."
        else
            echo "Error: $output_file not found after running ./ns3"
            exit 1
        fi
    done
done
