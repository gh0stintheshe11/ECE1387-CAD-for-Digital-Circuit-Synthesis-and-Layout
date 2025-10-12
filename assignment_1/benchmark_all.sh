#!/bin/bash

# benchmark_all.sh - Run all routing tests and generate results table

OUTPUT_FILE="benchmark_results.csv"
TEMP_FILE="temp_output.txt"

# Create CSV header
echo "Test Case,Version,Architecture,W,Used Segments,Time (ms)" > $OUTPUT_FILE

# Function to run a test and extract results
run_test() {
    local circuit=$1
    local version=$2
    local arch=$3
    local w=$4
    local threads=$5
    
    echo "Running: $circuit $version $arch W=$w threads=$threads"
    
    if [ "$version" == "norm" ]; then
        ./router -f ${circuit}.txt -a $arch -w $w > $TEMP_FILE 2>&1
    else
        ./router_parallel -f ${circuit}.txt -a $arch -w $w -t $threads > $TEMP_FILE 2>&1
    fi
    
    # Check if routing succeeded
    if grep -q "Routed successfully" $TEMP_FILE; then
        # Extract segments - look for "Used segments = NUMBER"
        segments=$(grep "Used segments" $TEMP_FILE | sed 's/.*Used segments = //' | awk '{print $1}')
        
        # Extract time - look for "Routing time: NUMBER ms"
        time=$(grep "Routing time" $TEMP_FILE | sed 's/.*Routing time: //' | sed 's/ ms//')
        
        # Format version name
        if [ "$version" == "norm" ]; then
            version_name="norm"
        else
            version_name="para_${threads}t"
        fi
        
        # Debug output
        echo "  -> Segments: $segments, Time: $time ms"
        
        echo "$circuit,$version_name,$arch,$w,$segments,$time" >> $OUTPUT_FILE
    else
        version_name="norm"
        if [ "$version" == "para" ]; then
            version_name="para_${threads}t"
        fi
        echo "$circuit,$version_name,$arch,$w,FAILED,FAILED" >> $OUTPUT_FILE
        echo "  -> FAILED"
    fi
}

# Test configurations
echo "Starting benchmarks..."
echo "====================="

# CCT1 Tests
run_test "cct1" "norm" "distributed" 3 0
run_test "cct1" "norm" "topbottom" 3 0
run_test "cct1" "para" "distributed" 3 2
run_test "cct1" "para" "topbottom" 3 2
run_test "cct1" "para" "distributed" 4 2
run_test "cct1" "para" "topbottom" 4 2
run_test "cct1" "para" "distributed" 3 4
run_test "cct1" "para" "topbottom" 3 4
run_test "cct1" "para" "distributed" 4 4
run_test "cct1" "para" "topbottom" 4 4
run_test "cct1" "para" "distributed" 3 8
run_test "cct1" "para" "topbottom" 3 8
run_test "cct1" "para" "distributed" 4 8
run_test "cct1" "para" "topbottom" 4 8

# CCT2 Tests
run_test "cct2" "norm" "distributed" 4 0
run_test "cct2" "norm" "topbottom" 4 0
run_test "cct2" "para" "distributed" 4 2
run_test "cct2" "para" "topbottom" 4 2
run_test "cct2" "para" "distributed" 5 2
run_test "cct2" "para" "topbottom" 5 2
run_test "cct2" "para" "distributed" 4 4
run_test "cct2" "para" "topbottom" 4 4
run_test "cct2" "para" "distributed" 5 4
run_test "cct2" "para" "topbottom" 5 4
run_test "cct2" "para" "distributed" 4 8
run_test "cct2" "para" "topbottom" 4 8
run_test "cct2" "para" "distributed" 5 8
run_test "cct2" "para" "topbottom" 5 8

# CCT3 Tests (CORRECTED W VALUES)
run_test "cct3" "norm" "distributed" 5 0
run_test "cct3" "norm" "topbottom" 6 0
run_test "cct3" "para" "distributed" 5 2
run_test "cct3" "para" "topbottom" 6 2
run_test "cct3" "para" "distributed" 6 2
run_test "cct3" "para" "topbottom" 7 2
run_test "cct3" "para" "distributed" 5 4
run_test "cct3" "para" "topbottom" 6 4
run_test "cct3" "para" "distributed" 6 4
run_test "cct3" "para" "topbottom" 7 4
run_test "cct3" "para" "distributed" 5 8
run_test "cct3" "para" "topbottom" 6 8
run_test "cct3" "para" "distributed" 6 8
run_test "cct3" "para" "topbottom" 7 8

# CCT4 Tests
run_test "cct4" "norm" "distributed" 9 0
run_test "cct4" "norm" "topbottom" 10 0
run_test "cct4" "para" "distributed" 9 2
run_test "cct4" "para" "topbottom" 10 2
run_test "cct4" "para" "distributed" 10 2
run_test "cct4" "para" "topbottom" 11 2
run_test "cct4" "para" "distributed" 9 4
run_test "cct4" "para" "topbottom" 10 4
run_test "cct4" "para" "distributed" 10 4
run_test "cct4" "para" "topbottom" 11 4
run_test "cct4" "para" "distributed" 9 8
run_test "cct4" "para" "topbottom" 10 8
run_test "cct4" "para" "distributed" 10 8
run_test "cct4" "para" "topbottom" 11 8

# Cleanup
rm -f $TEMP_FILE

echo ""
echo "====================="
echo "Benchmarking complete!"
echo "Results saved to: $OUTPUT_FILE"
echo ""
echo "Preview:"
head -20 $OUTPUT_FILE | column -t -s,