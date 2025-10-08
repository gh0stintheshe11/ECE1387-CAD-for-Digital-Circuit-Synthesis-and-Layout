# ECE 1387 Lecture #2

**Date:** Sept 19, 2025

---

## Routing - Assignment #1

**Topic:** FPGA maze routing

**Flow:**
```
        ↓
High-level behavioral
        ↓
Logic Synthesis
        ↓
Layout Synthesis
        ↓
```

---

## Routing

### Generic Problem Statement of Routing

**Given:**
- i) positions of cells (gates, FFs, or LUTs, DSP blocks)
- ii) "netlist" of connections between pins on cells

**Find:**
- paths of wires so that the required connections are made & no unrelated wires are shorted.

---

## Exact Problem Statement Depends On:

- target IC media (FPGA or ASIC)
- metal stack
- specific standard cell lib
- area target
- performance constraint
- power
- level of "detail"

**Note:** Today FPGA maze routing → ASIC routing uses same core routing algorithm

---

## FPGAs Architecture

FPGAs are an array of logic blocks surrounded by programmable routing

**W ≈ 300** in commercial FPGA

[Diagram shows a 3x3 grid of logic blocks with routing channels between them. Logic blocks are shown as squares, with routing tracks (shown as horizontal and vertical lines) connecting them. Routing channels are highlighted with cyan ovals at intersections.]

**A key parameter is W:** # of tracks per channel

**Here W = 3**

---

## Programmable Switches

### Early FPGAs
- SRAM cell
- 0/1 control

```
wire ——[SRAM cell]—— wire
```

### Now: FPGAs
- have buffered routing switches

```
     [>]
wires ——[buffer with SRAM control]—— wire
```

**A route of a connection is made by turning on the programmable switches to connect gates in desired way.**

---

## Two Key Routing Structures

### ① Connection between pins on logic blocks and neighbouring tracks

[Diagram shows a logic block (gate) with programmable switches connecting to horizontal routing tracks above and below]

**Here, Fc = W**

Each pin can connect to all W neighbour tracks.

**Fc = flexibility of connection block (box)**

---

### ② At intersection of horizontal & vertical channels

**this is called switch block (box)**

[Diagram shows intersection of horizontal and vertical channels with tracks numbered 0,1,2 and programmable switches at the intersection]

**Here, Fs = 3:** means every track can connect to 3 other tracks (on other 3 sides)

**Fs: flexibility of the switch block**

---

## Switch Block Design

**Turns out... switch block does not need to be a full crossbar**

---

## Routing Example

**Say —— is routed first.**

The selected route becomes an obstacle when —— is routed.

[Diagram shows FPGA grid with multiple routes marked in different colors (red, purple, green) demonstrating how earlier routes become obstacles for later routing]

---

## Switch Block Topologies

All have **Fs = 3**

[Three diagrams showing different switch block connection patterns, each with tracks numbered 0-4 on all four sides]

**a) Disjoint** - "planar"  
Fs = 3

**b) Universal**  
Fs = 3

**c) Wilton**  
Fs = 3

---

## Lee-Moore Maze Routing Algorithm

**- Similar to Dijkstra's alg.**

**array? Up to you...**

**Data structure:** need to have an entry (label) for each track segment.

**Initially:** label each track segment as 'A' (available)

**Action:** route one connection at a time.  
Make segment unavailable 'U' to subsequent connections to be routed

---

## Lee-Moore Example

**W = 3**

[Detailed diagram showing an FPGA routing grid with:
- Track segments labeled with coordinates (0,0), (0,1), (0,2), (1,0), (1,1), (1,2), (2,0), (2,1), (2,2)
- Numbers showing distances (0, 1, 2, 3, 4) from source
- Purple route showing path from pin 4 to pin 2
- Cyan circles indicating switch blocks
- Grid coordinates showing routing progression]

**route from 2,2 pin 4 to 1,0 pin 2**

---

## Lee-Moore Algorithm Steps

### For all conns to route

**pin #**  
source pin: X₁, y₁, P₁  
sink pin: X₂, y₂, P₂ (target)

**1)** Mark all track segments adjacent to target 'T' (if they are avail)

**2)** Mark available track segments adjacent to source '0' and push onto expansion list (queue)

---

### While Loop

```
while (expansion list not empty) {
    pop segment j from expansion list
    
    if j is target, exit while loop
    
    for each segment k reachable from j thru switch {
        if k is available ('A') {
            label(k) = label(j) + 1
            push k onto expansion list
        }
    }
}
```

[Cyan highlighting shows the wave expansion pattern]

---

### Traceback

**If exit while loop having hit target:**
- trace back thru descending labels
- → define routing path for connection
- → mark segments unavailable 'U'

**Reset labeled segments as 'A'**

---

### Failure Case

**If exit while loop without hitting target:**
- → failed to route connection

**This alg. will find a routing path if it exists.**

This is a **heuristic alg.** Exhaustive would try all connection orderings, all trace back options, ... → **(huge # of combinations)**

---