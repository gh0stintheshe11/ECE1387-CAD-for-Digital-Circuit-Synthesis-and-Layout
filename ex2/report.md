
## 7.

1. Area optimization generally reduces LUT count
   - alu4: 850 → 815 (saved 35 LUTs)
   - clma: 3464 → 2991 (saved 473 LUTs)
   - div: 9919 → 8197 (saved 1722 LUTs)
   - misex3: 803 → 754 (saved 49 LUTs)
   - sqrt: 4409 → 5127 (increased by 718 LUTs??) - maybe area optimization changes the circuit structure??

2. NAND-gates per LUT ratio (3.2 - 5.97)
   - This makes sense since A 6-LUT can implement complex functions
   - NAND gates are only 2-input, so need multiple to match a LUT