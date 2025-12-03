#!/bin/bash

# Step 7: Analysis - Compare LUT counts with NAND-gate-equivalents

echo "Step 7: Comparing LUT counts with NAND-gate-equivalents"
echo "========================================================"
echo ""

# Create step_7 directory
mkdir -p step_7

# Run Python script to parse and analyze
python3 << 'EOF'
import re

circuits = ["alu4", "clma", "div", "misex3", "sqrt"]

# Parse Step 1 (Depth-optimized LUTs)
step1_luts = {}
with open("step_1/step1_log.txt", "r") as f:
    content = f.read()
    for circuit in circuits:
        # Split by circuit sections
        sections = content.split(f"Processing {circuit}...")
        if len(sections) > 1:
            circuit_section = sections[1].split("Processing ")[0]
            # Find ALL occurrences of nd = value, take the LAST one (after mapping)
            matches = re.findall(r'nd\s*=\s*(\d+)', circuit_section)
            if matches:
                step1_luts[circuit] = int(matches[-1])  # Take last occurrence

# Parse Step 2 (Area-optimized LUTs)
step2_luts = {}
with open("step_2/step2_log.txt", "r") as f:
    content = f.read()
    for circuit in circuits:
        sections = content.split(f"Processing {circuit}...")
        if len(sections) > 1:
            circuit_section = sections[1].split("Processing ")[0]
            matches = re.findall(r'nd\s*=\s*(\d+)', circuit_section)
            if matches:
                step2_luts[circuit] = int(matches[-1])

# Parse Step 6 (NAND and INV counts)
step6_gates = {}
with open("step_6/step6_log.txt", "r") as f:
    content = f.read()
    for circuit in circuits:
        # Find nand2 and inv1 counts
        pattern = rf"Processing {circuit}.*?inv1.*?Instance\s*=\s*(\d+).*?nand2.*?Instance\s*=\s*(\d+)"
        match = re.search(pattern, content, re.DOTALL)
        if match:
            inv1 = int(match.group(1))
            nand2 = int(match.group(2))
            nand_equiv = nand2 + 0.5 * inv1
            step6_gates[circuit] = {
                'inv1': inv1,
                'nand2': nand2,
                'nand_equiv': nand_equiv
            }

# Create output file
output = []
output.append("Step 7: LUT vs NAND-Gate-Equivalent Analysis")
output.append("=" * 90)
output.append("")
output.append(f"{'Circuit':<10} {'Step1 LUTs':<12} {'Step2 LUTs':<12} {'NAND-Equiv':<15} {'Gates/LUT(S1)':<15} {'Gates/LUT(S2)':<15}")
output.append("-" * 90)

total_step1_luts = 0
total_step2_luts = 0
total_nand_equiv = 0

for circuit in circuits:
    s1_luts = step1_luts.get(circuit, 0)
    s2_luts = step2_luts.get(circuit, 0)
    nand_equiv = step6_gates.get(circuit, {}).get('nand_equiv', 0)
    
    gates_per_lut_s1 = nand_equiv / s1_luts if s1_luts > 0 else 0
    gates_per_lut_s2 = nand_equiv / s2_luts if s2_luts > 0 else 0
    
    output.append(f"{circuit:<10} {s1_luts:<12} {s2_luts:<12} {nand_equiv:<15.1f} {gates_per_lut_s1:<15.2f} {gates_per_lut_s2:<15.2f}")
    
    total_step1_luts += s1_luts
    total_step2_luts += s2_luts
    total_nand_equiv += nand_equiv

output.append("-" * 90)
avg_gates_per_lut_s1 = total_nand_equiv / total_step1_luts if total_step1_luts > 0 else 0
avg_gates_per_lut_s2 = total_nand_equiv / total_step2_luts if total_step2_luts > 0 else 0
output.append(f"{'TOTAL':<10} {total_step1_luts:<12} {total_step2_luts:<12} {total_nand_equiv:<15.1f} {avg_gates_per_lut_s1:<15.2f} {avg_gates_per_lut_s2:<15.2f}")
output.append("")
output.append("Notes:")
output.append("- NAND-Equiv = #nand2 + 0.5*#inv1")
output.append("- Gates/LUT(S1) = NAND-Equiv / Step1 LUTs (depth-optimized)")
output.append("- Gates/LUT(S2) = NAND-Equiv / Step2 LUTs (area-optimized)")
output.append("")
output.append(f"Average NAND-gate-equivalents per LUT (Step 1): {avg_gates_per_lut_s1:.2f}")
output.append(f"Average NAND-gate-equivalents per LUT (Step 2): {avg_gates_per_lut_s2:.2f}")

# Print to console
for line in output:
    print(line)

# Save to file
with open("step_7/step7_analysis.txt", "w") as f:
    f.write("\n".join(output))

print("\nResults saved to step_7/step7_analysis.txt")
EOF