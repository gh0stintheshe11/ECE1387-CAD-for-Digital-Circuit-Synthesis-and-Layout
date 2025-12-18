# ECE 1387 – Exercise 2

Lang Sun
1003584971

## 1.

| Circuit | # of LUTs | Depth (levels) |
|---------|-----------|----------------|
| alu4    | 850       | 6              |
| clma    | 3464      | 10             |
| div     | 9919      | 858            |
| misex3  | 803       | 5              |
| sqrt    | 4409      | 1005           |
| **Total** | **19,445** | -            |

- Depth optimization prioritizes reducing critical path length
- LUT count (nd) represents the number of 6-input LUTs in the mapping
- Depth (lev) represents the number of LUT levels on the critical path

## 2.

| Circuit | Step 1 LUTs | Step 1 Depth | Step 2 LUTs | Step 2 Depth | Area Change | Depth Change |
|---------|-------------|--------------|-------------|--------------|-------------|--------------|
| alu4    | 850         | 6            | 815         | 12           | -35 (-4.1%) | +6 (+100%)   |
| clma    | 3464        | 10           | 2991        | 19           | -473 (-13.7%) | +9 (+90%)  |
| div     | 9919        | 858          | 8197        | 2060         | -1722 (-17.4%) | +1202 (+140%) |
| misex3  | 803         | 5            | 754         | 10           | -49 (-6.1%) | +5 (+100%)   |
| sqrt    | 4409        | 1005         | 5127        | 2216         | +718 (+16.3%) | +1211 (+120%) |
| **Total** | **19,445** | -            | **17,884**  | -            | **-1,561 (-8.0%)** | -        |

- **Area reduction**: On average, area optimization reduced LUT count by **8.0%** (1,561 fewer LUTs)
- **Depth impact**: Depth increased significantly (**~100-140%**) across all circuits - this is the expected tradeoff when optimizing for area instead of depth
- **Notable exception**: sqrt increased in LUT count by 16.3%, showing that area optimization doesn't always reduce area for all circuits

## 3.

| Circuit | Step 1 (resyn2 x1) LUTs | Step 1 Depth | Step 3 (resyn2 x3) LUTs | Step 3 Depth | LUT Change | Depth Change |
|---------|-------------------------|--------------|-------------------------|--------------|------------|--------------|
| alu4    | 850                     | 6            | 872                     | 5            | +22 (+2.6%) | -1 (-16.7%) |
| clma    | 3464                    | 10           | 3101                    | 10           | -363 (-10.5%) | 0 (0%) |
| div     | 9919                    | 858          | 9889                    | 858          | -30 (-0.3%) | 0 (0%) |
| misex3  | 803                     | 5            | 787                     | 5            | -16 (-2.0%) | 0 (0%) |
| sqrt    | 4409                    | 1005         | 4528                    | 1005         | +119 (+2.7%) | 0 (0%) |
| **Total** | **19,445**            | -            | **19,177**              | -            | **-268 (-1.4%)** | - |

- **Overall impact**: Calling resyn2 three times resulted in a modest **1.4% reduction** in total LUT count (268 fewer LUTs)
- **Mixed results**: 
  - **clma** showed significant improvement (-10.5%)
  - **alu4** and **sqrt** actually increased in LUT count
  - **div** and **misex3** showed minimal change
- **Depth**: Mostly unchanged, with only alu4 improving by 1 level
- **Conclusion**: Multiple resyn2 calls can help in some cases (like clma), but the improvement is circuit-dependent and not consistently beneficial. The "black magic" nature of technology-independent optimization means results can vary unpredictably.


## 4.

```mermaid
flowchart TD
    Start([Start]) --> Init[Initialize: Read circuit list]
    Init --> CreateDir[Create step_4 directory]
    CreateDir --> OpenLog[Open step4_log.txt]
    
    OpenLog --> LoopCircuits{For each circuit}
    
    LoopCircuits -->|Next circuit| ParseBLIF[Parse BLIF file]
    
    ParseBLIF --> ReadLines[Read file line by line]
    ReadLines --> CheckNames{Line starts with<br/>.names?}
    CheckNames -->|Yes| ExtractTokens[Extract tokens:<br/>inputs and output]
    ExtractTokens --> CheckInputs{Has inputs?}
    CheckInputs -->|Yes| StoreLUT[Store LUT:<br/>output, inputs set]
    CheckInputs -->|No| ReadLines
    StoreLUT --> ReadLines
    CheckNames -->|No| ReadLines
    ReadLines -->|EOF| LUTList[LUT list created]
    
    LUTList --> BuildCompat[Build compatibility graph]
    BuildCompat --> NestedLoop{For each LUT pair<br/>i, j}
    NestedLoop --> CheckPack{Can pack together?<br/>Combined inputs <= 5?}
    CheckPack -->|Yes| AddCompat[Add j to compat of i<br/>Add i to compat of j]
    AddCompat --> NestedLoop
    CheckPack -->|No| NestedLoop
    NestedLoop -->|All pairs checked| SortLUTs[Sort LUTs by<br/>compatibility count<br/>fewer partners first]
    
    SortLUTs --> GreedyPack{For each LUT i<br/>in sorted order}
    GreedyPack -->|Next LUT| CheckUsed{Already used?}
    CheckUsed -->|Yes| GreedyPack
    CheckUsed -->|No| FindPartner[Search compatible LUTs<br/>for unused partner]
    FindPartner --> HasPartner{Partner found?}
    
    HasPartner -->|Yes| PackPair[Pack two LUTs together<br/>Mark both as used]
    PackPair --> GreedyPack
    
    HasPartner -->|No| PackSingle[Pack single LUT<br/>Mark as used]
    PackSingle --> GreedyPack
    
    GreedyPack -->|All LUTs packed| WriteOutput[Write packed results<br/>to circuit.packed.txt]
    WriteOutput --> LogResults[Log statistics:<br/>Original, Fracturable,<br/>Reduction]
    
    LogResults --> LoopCircuits
    LoopCircuits -->|All done| Summary[Generate summary table]
    Summary --> CloseLog[Close log file]
    CloseLog --> End([End])
    
    style Start fill:#90EE90
    style End fill:#FFB6C1
    style ParseBLIF fill:#87CEEB
    style BuildCompat fill:#FFD700
    style GreedyPack fill:#FFA07A
    style PackPair fill:#98FB98
    style PackSingle fill:#DDA0DD
```

### Algorithm Description

The program implements a **greedy compatibility-based packing algorithm**:

1. **Parse BLIF**: Extract each LUT's output name and input signals from the area-optimized mapping (Step 2)

2. **Build Compatibility Graph**: For each pair of LUTs, check if they can be packed together:
   - Compute union of their input sets
   - Compatible if combined unique inputs ≤ 5

3. **Greedy Packing Strategy**:
   - Sort LUTs by compatibility count (ascending order)
   - Prioritize hard-to-pack LUTs (those with fewer compatible partners)
   - For each LUT: pair with compatible partner if available, otherwise pack alone

4. **Output Format**: One line per fracturable LUT:
   - Single LUT: `output_name`
   - Paired LUTs: `output_name1 output_name2`

### Results

| Circuit | Original LUTs | Fracturable LUTs | Paired | Single | Reduction | Reduction % |
|---------|---------------|------------------|--------|--------|-----------|-------------|
| alu4    | 815           | 625              | 190    | 435    | 190       | 23.3%       |
| clma    | 2977          | 2244             | 733    | 1511   | 733       | 24.6%       |
| div     | 8197          | 4246             | 3951   | 295    | 3951      | 48.2%       |
| misex3  | 754           | 589              | 165    | 424    | 165       | 21.9%       |
| sqrt    | 5127          | 2716             | 2411   | 305    | 2411      | 47.0%       |
| **Total** | **17,870**  | **10,420**       | **7,450** | **2,970** | **7,450** | **41.7%** |

- **Overall reduction**: The packing algorithm achieved a **41.7% reduction** in fracturable LUT count (7,450 fewer LUTs)
- **Best performance**: **div** (48.2%) and **sqrt** (47.0%) circuits showed the highest packing efficiency
- **Moderate performance**: **alu4**, **clma**, and **misex3** achieved 22-25% reduction
- **Pairing success rate**: 41.7% of original LUTs (7,450 out of 17,870) were successfully paired
- The greedy algorithm effectively minimizes fracturable LUT usage by prioritizing hard-to-pack LUTs first

## 5.

| Circuit | Area | Delay | Levels |
|---------|------|-------|--------|
| alu4    | 4,578.00 | 12.90 | 12 |
| clma    | 17,402.00 | 30.60 | 26 |
| div     | 86,404.00 | 3,440.36 | 2,237 |
| misex3  | 4,139.00 | 11.80 | 11 |
| sqrt    | 39,155.00 | 4,170.78 | 3,884 |
| **Total** | **151,678.00** | - | - |

- Area is reported in arbitrary units (gate area sum)
- Delay is reported in time units (gate delays on critical path)
- The full library provides a rich set of gates (23 types) including complex gates like AOI (AND-OR-INVERT) and OAI (OR-AND-INVERT) which can reduce both area and delay
- Most common gates used: nand2, nand3, aoi21, oai21, nor2, inv1

## 6.

| Circuit | Step 5 Area (Full) | Step 6 Area (Minimal) | Area Increase | Step 5 Delay | Step 6 Delay | Delay Increase | Step 5 Levels | Step 6 Levels |
|---------|--------------------|-----------------------|---------------|--------------|--------------|----------------|---------------|---------------|
| alu4    | 4,578.00          | 6,075.00              | +32.7%        | 12.90        | 23.80        | +84.5%         | 12            | 25            |
| clma    | 17,402.00         | 22,289.00             | +28.1%        | 30.60        | 55.00        | +79.7%         | 26            | 57            |
| div     | 86,404.00         | 97,950.00             | +13.4%        | 3,440.36     | 4,535.29     | +31.8%         | 2,237         | 4,555         |
| misex3  | 4,139.00          | 5,500.00              | +32.9%        | 11.80        | 21.90        | +85.6%         | 11            | 23            |
| sqrt    | 39,155.00         | 47,284.00             | +20.8%        | 4,170.78     | 5,970.54     | +43.2%         | 3,884         | 6,082         |
| **Total** | **151,678.00**  | **179,098.00**        | **+18.1%**    | -            | -            | -              | -             | -             |

- **Area penalty**: Using only inv1 and nand2 (minimal library) increases area by **18.1%** on average compared to the full 23-gate library
- **Delay penalty**: Delay increased significantly (**31.8% to 85.6%**) due to lack of complex gates like AOI/OAI
- **Levels increase**: Critical path levels roughly doubled, as simple 2-input gates require more stages than complex gates
- **Gate distribution**: Approximately 80% nand2, 20% inv1 across all circuits
- The full library's complex gates (aoi21, oai21, etc.) significantly improve both area and delay

## 7.

| Circuit | Step 1 LUTs (Depth) | Step 2 LUTs (Area) | NAND-Equiv | Gates/LUT (S1) | Gates/LUT (S2) |
|---------|---------------------|-----------------------|------------|----------------|----------------|
| alu4    | 850                 | 815                   | 3,037.5    | 3.57           | 3.73           |
| clma    | 3,464               | 2,991                 | 11,144.5   | 3.22           | 3.73           |
| div     | 9,919               | 8,197                 | 48,975.0   | 4.94           | 5.97           |
| misex3  | 803                 | 754                   | 2,750.0    | 3.42           | 3.65           |
| sqrt    | 4,409               | 5,127                 | 23,642.0   | 5.36           | 4.61           |
| **Total** | **19,445**        | **17,884**            | **89,549.0** | **4.61**     | **5.01**       |

**Average NAND-gate-equivalents per LUT:**
- **Step 1 (depth-optimized)**: 4.61 gates/LUT
- **Step 2 (area-optimized)**: 5.01 gates/LUT

1. Area optimization generally reduces LUT count
   - alu4: 850 → 815 (saved 35 LUTs)
   - clma: 3464 → 2991 (saved 473 LUTs)
   - div: 9919 → 8197 (saved 1722 LUTs)
   - misex3: 803 → 754 (saved 49 LUTs)
   - sqrt: 4409 → 5127 (increased by 718 LUTs??) - maybe area optimization changes the circuit structure??

2. NAND-gates per LUT ratio (3.2 - 5.97)
   - This makes sense since A 6-LUT can implement complex functions
   - NAND gates are only 2-input, so need multiple to match a LUT

3. complexity
   - More complex circuits pack more logic into each LUT

The fact that a 6-LUT to replace ~5 NAND gates kinda shows why LUT-based FPGAs are efficient for implementing arbitrary logic functions