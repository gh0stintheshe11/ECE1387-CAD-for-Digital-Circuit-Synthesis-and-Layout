#!/bin/bash

# Step 5: Standard cell mapping with full mcnc.genlib library

circuits=("alu4" "clma" "div" "misex3" "sqrt")

# Create step_5 directory
mkdir -p step_5

# Make sure abc.rc is in current directory
if [ ! -f abc.rc ]; then
    cp abc/abc.rc .
fi

# Create output file in step_5 folder
output_file="step_5/step5_log.txt"
echo "Step 5: Standard Cell Mapping (Full Library)" > $output_file
echo "=============================================" >> $output_file
echo "" >> $output_file

for circuit in "${circuits[@]}"
do
    echo "Processing $circuit..." | tee -a $output_file
    echo "-----------------------------------" >> $output_file
    
    # Run ABC with standard cell mapping
    ./abc/abc -c "read_blif ${circuit}.blif; resyn2; read_library mcnc.genlib; map -v; print_stats; print_gates; write_blif step_5/${circuit}.mapped.sc.blif" >> $output_file 2>&1
    
    echo "" >> $output_file
    echo "" >> $output_file
done

echo "Done! Results saved to step_5/step5_log.txt"
echo "Mapped BLIF files saved to step_5/"