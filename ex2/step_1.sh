#!/bin/bash

# Step 1: FPGA mapping with 6-LUTs (depth optimization)

circuits=("alu4" "clma" "div" "misex3" "sqrt")

# Create step_1 directory
mkdir -p step_1

# Make sure abc.rc is in current directory (needed for resyn2)
if [ ! -f abc.rc ]; then
    cp abc/abc.rc .
fi

# Create output file in step_1 folder
output_file="step_1/step1_log.txt"
echo "Step 1: 6-LUT Mapping (Depth Optimization)" > $output_file
echo "=========================================" >> $output_file
echo "" >> $output_file

for circuit in "${circuits[@]}"
do
    echo "Processing $circuit..." | tee -a $output_file
    echo "-----------------------------------" >> $output_file
    
    # Run ABC in batch mode, save mapped file to step_1 folder
    ./abc/abc -c "read_blif ${circuit}.blif; resyn2; print_stats; if -K 6; print_stats; write_blif step_1/${circuit}.mapped.blif" >> $output_file 2>&1
    
    echo "" >> $output_file
    echo "" >> $output_file
done

echo "Done! Results saved to step_1/step1_log.txt"
echo "Mapped BLIF files saved to step_1/"