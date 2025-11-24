#!/bin/bash

# Test script for ECE1387 Assignment 3
# Runs partitioner on cct1-cct4 with thread counts 1, 2, 4, 8, 16, 32, 64, 128

CIRCUITS="cct1 cct2 cct3 cct4"
THREADS="1 2 4 8 16 32 64 128"

echo "=== ECE1387 Assignment 3 Testing ==="
echo "Starting at: $(date)"
echo ""

# Create log directories
for cct in $CIRCUITS; do
    mkdir -p ${cct}_log
done

# Run tests
for cct in $CIRCUITS; do
    echo "----------------------------------------"
    echo "Testing ${cct}.txt"
    echo "----------------------------------------"
    
    # Check if circuit file exists
    if [ ! -f "${cct}.txt" ]; then
        echo "  WARNING: ${cct}.txt not found, skipping..."
        continue
    fi
    
    for t in $THREADS; do
        echo "  Running with ${t} thread(s)..."
        logfile="${cct}_log/${cct}_${t}t.txt"
        
        # Run and capture output
        ./partitioner -f ${cct}.txt -t ${t} > "$logfile" 2>&1
        
        # Extract key results for quick summary
        cost=$(grep "Optimal cost:" "$logfile" | head -1 | awk '{print $3}')
        nodes=$(grep "Nodes visited:" "$logfile" | tail -1 | awk '{print $3}')
        time=$(grep "Runtime:" "$logfile" | awk '{print $2, $3}')
        
        echo "    -> Cost: ${cost}, Nodes: ${nodes}, Time: ${time}"
    done
    echo ""
done

echo "=== Testing Complete ==="
echo "Finished at: $(date)"
echo ""
echo "Log files saved to:"
for cct in $CIRCUITS; do
    if [ -d "${cct}_log" ]; then
        echo "  ${cct}_log/"
    fi
done