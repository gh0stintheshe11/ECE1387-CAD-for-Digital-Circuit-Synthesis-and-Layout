#!/bin/bash

# Step 3: FPGA mapping with 6-LUTs (depth optimization) with resyn2 called 3 times

circuits=("alu4" "clma" "div" "misex3" "sqrt")

# Create step_3 directory
mkdir -p step_3

# Make sure abc.rc is in current directory
if [ ! -f abc.rc ]; then
    cp abc/abc.rc .
fi

# Create output file in step_3 folder
output_file="step_3/step3_log.txt"
echo "Step 3: 6-LUT Mapping (Depth Optimization) with resyn2 x3" > $output_file
echo "==========================================================" >> $output_file
echo "" >> $output_file

for circuit in "${circuits[@]}"
do
    echo "Processing $circuit..." | tee -a $output_file
    echo "-----------------------------------" >> $output_file
    
    # Run ABC with resyn2 called THREE times
    ./abc/abc -c "read_blif ${circuit}.blif; resyn2; resyn2; resyn2; print_stats; if -K 6; print_stats; write_blif step_3/${circuit}.mapped.blif" >> $output_file 2>&1
    
    echo "" >> $output_file
    echo "" >> $output_file
done

echo "Done! Results saved to step_3/step3_log.txt"
echo "Mapped BLIF files saved to step_3/"