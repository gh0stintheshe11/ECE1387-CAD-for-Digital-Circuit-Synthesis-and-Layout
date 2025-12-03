#!/bin/bash

# Step 2: FPGA mapping with 6-LUTs (area optimization)

circuits=("alu4" "clma" "div" "misex3" "sqrt")

# Create step_2 directory
mkdir -p step_2

# Make sure abc.rc is in current directory
if [ ! -f abc.rc ]; then
    cp abc/abc.rc .
fi

# Create output file in step_2 folder
output_file="step_2/step2_log.txt"
echo "Step 2: 6-LUT Mapping (Area Optimization)" > $output_file
echo "=========================================" >> $output_file
echo "" >> $output_file

for circuit in "${circuits[@]}"
do
    echo "Processing $circuit..." | tee -a $output_file
    echo "-----------------------------------" >> $output_file
    
    # Run ABC with -a flag for area optimization
    ./abc/abc -c "read_blif ${circuit}.blif; resyn2; print_stats; if -K 6 -a; print_stats; write_blif step_2/${circuit}.mapped.blif" >> $output_file 2>&1
    
    echo "" >> $output_file
    echo "" >> $output_file
done

echo "Done! Results saved to step_2/step2_log.txt"
echo "Mapped BLIF files saved to step_2/"