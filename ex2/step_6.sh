#!/bin/bash

# Step 6: Standard cell mapping with minimal library (inv1, nand2, buf, zero, one)

circuits=("alu4" "clma" "div" "misex3" "sqrt")

# Create step_6 directory
mkdir -p step_6

# Make sure abc.rc is in current directory
if [ ! -f abc.rc ]; then
    cp abc/abc.rc .
fi

# Create output file in step_6 folder
output_file="step_6/step6_log.txt"
echo "Step 6: Standard Cell Mapping (Minimal Library)" > $output_file
echo "================================================" >> $output_file
echo "" >> $output_file

for circuit in "${circuits[@]}"
do
    echo "Processing $circuit..." | tee -a $output_file
    echo "-----------------------------------" >> $output_file
    
    # Run ABC with minimal library mapping
    ./abc/abc -c "read_blif ${circuit}.blif; resyn2; read_library mcnc_step6.genlib; map -v; print_stats; print_gates; write_blif step_6/${circuit}.mapped.sc.blif" >> $output_file 2>&1
    
    echo "" >> $output_file
    echo "" >> $output_file
done

echo "Done! Results saved to step_6/step6_log.txt"
echo "Mapped BLIF files saved to step_6/"