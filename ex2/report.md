# ECE 1387 – Exercise 2

Lang Sun
1003584971

## 1.



## 2.



## 3.



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

## 5.



## 6.



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