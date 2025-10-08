# ECE 1387 Lecture #4
**October 3, 2025**

## Topics
- Routing (continued)
- Placement
- How is A1 going?

---

## Pathfinder Routing Algorithm (Recap/Detail)

### Overall Algorithm

The Pathfinder routing algorithm consists of nested loops that iteratively route all signals while resolving resource conflicts:

**1) Initial Routing Phase:**
- Route all signals allowing shorts (overlapping resource usage)

**2) Iterative Refinement Loop:**
```
while (shared resources exist AND not exhausted) {
    loop over all nets i ∈ ℰ {
        // Rip up existing routing for net i
        rip up routing for net i
        
        // Route from source to all targets
        Route(i) ← source_i
        
        loop until all loads t_ij on net i routed {
            // Initialize priority queue with current routing
            init priority queue (expansion list)
            to everything in Route(i) at cost 0
            
            // EXPANSION TARGET SEARCH
            loop until t_ij is found {
                pop lowest cost node m from PRQ
                
                if m is t_ij:
                    break
                
                // Explore neighbors
                loop over nodes n connected to m in RRG {
                    if n already visited:
                        continue
                    
                    // Add neighbor to priority queue
                    add n to PRQ at cost of:
                        C_n + PRQCost(m)
                        // where C_n is the cost of node n
                        // and PRQCost(m) is cost of m in PRQ
                }
            }
            
            // Backtrace from target to existing route
            backtrace from t_ij to Route(i)
            add trace to Route(i)
            
            // Adjust present congestion costs accordingly
            adjust p_n's accordingly
        }
    }
    
    // After routing all signals in this iteration
    adjust P_fac, h_fac  // increase these
    update h_n's accordingly
}
```

**3) Termination:**
- Algorithm terminates when all signals are routed without resource conflicts
- Or when maximum iterations reached

### Key Notes:

**Nair Iteration:**
Each iteration of the outer while loop is one **Nair iteration** (routes all signals once)

**Cost Components:**
- `C_n`: Cost of using routing resource node n
- `PRQCost(m)`: Accumulated cost to reach node m
- Total cost at node n = C_n + PRQCost(m)

---

## A* Routing (Commercial Routers)

### Comparison: Maze Routing vs A* Routing

**Maze Routing:**
```
     target
        •
   ○ ○ ○ ○ ○
  ○ ○ ○ ○ ○ ○
 ○ ○ ○ ○ ○ ○ ○
○ ○ ○ • ○ ○ ○ ○
  src
```
- Explores all directions equally
- Expands wave from source
- Guaranteed to find shortest path
- Can be slow for distant targets

**A* Routing:**
```
        target
           •
         ⟋ ⟋ ⟋
       ⟋ ⟋ ⟋ ⟋
     ⟋ ⟋ ⟋ ⟋ ⟋
   • 
  src
```
- **Costing includes an estimate of cost to target**
- Biases search toward target
- Much faster than pure maze routing
- Used in all commercial routers

**Key Advantage:**
A* routing is **needed for runtime** - significantly faster than basic maze routing while still finding good paths.

---

## Timing-Driven Routing

### Motivation

**Question:** How to make the algorithm give faster resources to connections that need them?

**Approach:**

1) **Determine which connections are timing critical**
   - Identify paths on or near the critical path

2) **For critical connections, want Pathfinder to pay more attention to delay (vs congestion)**
   - Adjust cost function to prioritize delay for critical nets
   - Non-critical nets can tolerate slower routes

---

## Timing Analysis

### Example Circuit

```
      2ns        3ns     1ns
  □──┬──[>○]──┬──────[○
  □ 2ns│       │  5ns       4ns     3ns
       └──[>○]─┴──[>○]──┬──[>○]───□
    5ns    2ns          │   2ns
  □─────────────────────┘
```

**Timing Calculations:**

- **D₁,₂ = 20ns** (arrival time at node 2 from node 1)
- **D₁,₃ = 9 ns** (arrival time at node 3 from node 1)  
- **D₄,₅ = 16ns** (arrival time at node 5 from node 4)

**Criticality Factors:**
- **A₁,₂ = 1** (on critical path)
- **A₁,₃ = 9/20** (moderately critical)
- **A₄,₅ = 4/5** (fairly critical)

**Critical path delay:** 2 + 2 + 5 + 2 + 4 + 2 + 3 = **20ns = D_max**

### Delay Metrics

**D_i,j = longest delay path through pin (i) to (j)**

This represents the arrival time at sink j from source i through the longest combinational path.

---

## Criticality Metric

### Definition

Finally, define criticality as:

```
A_i,j = D_i,j / D_max
```

Where:
- **D_i,j** = longest delay path from pin i to pin j
- **D_max** = critical path delay (longest path in entire circuit)

### Interpretation

**A_i,j close to 1:**
- Connection (i)→(j) is **on or close to critical path**
- This connection is timing-critical
- Router should prioritize fast routing resources

**A_i,j << 1:**
- Connection (i)→(j) is **not critical**
- So it's OK to slow down
- Can use congested/slower routing resources

---

## Timing-Driven Cost Function

### New Total Cost Formula

When routing a connection from pin (i)→(j):

```
Cost_n = A_i,j · d_n + (1 - A_i,j) · C_n
```

Where:
- **A_i,j** = criticality factor (from timing analysis)
- **d_n** = delay of routing resource node n
- **C_n** = congestion cost of node n (from last week)

### Effect

**Critical connections (A_i,j ≈ 1) are routed with greater emphasis on delay:**
- When A_i,j = 1: Cost_n = d_n (purely delay-driven)
- When A_i,j = 0: Cost_n = C_n (purely congestion-driven)

This allows the router to make intelligent trade-offs between delay and congestion based on timing criticality.

---

## Setting Initial Criticality Values

### Problem

**How to set A_i,j initially?**

To compute A_i,j = D_i,j / D_max, we need:
- D_i,j values (delay from i to j)
- D_max (critical path delay)

**Challenge:** We need **best-possible delays initially** to have accurate timing estimates, but we don't have routing yet!

### Solutions

**Option 1: Estimate delays from placement**
- Use Manhattan distance × wire delay per unit length
- Add estimated logic delays
- Not very accurate but gives rough estimates

**Option 2: Set all A_i,j to be 0.5 initially**
- Balanced between delay and congestion
- Simple but doesn't prioritize anything initially

**Option 3: Set all A_i,j to be 1 initially** ✓
- **Get high speed initial routing**
- All connections prioritize delay
- After first routing, compute actual delays and update A_i,j
- Preferred approach in practice

---

## Updating Criticality Values

### Why A_i,j's Become Stale

**As signals are ripped up and rerouted:**
- Route paths change
- Actual delays change
- D_i,j values change
- D_max may change
- Therefore, A_i,j values become outdated

### Solution

**Need to periodically invoke timing analysis to get fresh A_i,j values:**

Typical approach:
- Perform timing analysis every N Nair iterations
- Recompute all D_i,j and D_max
- Update all A_i,j = D_i,j / D_max
- Continue routing with updated criticalities

This keeps the router focused on the truly critical paths as the routing evolves.

---

## Global Routing vs Detailed Routing

### Another Style of Routing: Global Routing

**Key issue in CAD:** How is the entire CAD flow broken up into subtasks?

**Pathfinder / Maze Routing:**
- Solve entire routing problem at once
- Choose specific wire segments and switch connections simultaneously
- More complex but gives complete solution

**Before Pathfinder, one idea was to break routing into two sub-tasks:**

### 1) Global Routing

**Goal:** Just decide which channels to use (not individual wire segments)

**Visual representation:**
```
┌─────┐      ┌─────┐      ┌─────┐      ┌─────┐
│     │      │     │      │     │      │     │
│     │──┐   │     │      │     │   ┌──│     │
└─────┘  │   └─────┘      └─────┘   │  └─────┘
         │                           │
┌─────┐  │   ┌─────┐      ┌─────┐   │  ┌─────┐
│     │  │   │     │      │     │   │  │     │
│     │  └───│     │──┐   │     │───┘  │     │
└─────┘      └─────┘  │   └─────┘      └─────┘
                      │          
┌─────┐      ┌─────┐ │   ┌─────┐      ┌─────┐
│     │      │     │ └───│     │      │     │
│     │      │     │     │     │──┐   │     │
└─────┘      └─────┘     └─────┘  │   └─────┘
                                   │
┌─────┐      ┌─────┐      ┌─────┐ │   ┌─────┐
│     │      │     │      │     │ │   │     │
│     │      │     │      │     │─┘   │     │
└─────┘      └─────┘      └─────┘     └─────┘
```

- Routes shown at channel level
- Doesn't specify exact wire tracks or segments
- Coarser granularity

### 2) Detailed Routing

**Goal:** Choose individual wire segments and connections through switch blocks

**Constraints:** Must honor the global routing decisions

**For FPGAs:**

Global routing would say:
- "Try not to exceed using 70% of W in any channel"
- Where W = channel width (number of tracks)

**Why 70%?**
- Leaves margin for detailed routing flexibility
- **Otherwise detailed routing may fail**
- If global routing over-subscribes a channel, detailed router can't find a solution

---

## Placement: Step Before Routing

### Problem Definition

**Given:**

1) **IC media** (FPGA or standard cell layout)
   - Available locations/sites for cells
   - Routing architecture

2) **Netlist of cells and connections between them**
   - Logic blocks/gates to be placed
   - Nets connecting pins on cells

**Find:** Positions of cells

---

## Placement Objectives

### Goal

**So as to minimize:** ← First order goal of any placer

- **Wirelength (estimated)** - primary objective
- **Congestion** - avoid routing hotspots  
- **Area of layout** - minimize chip size
- **Critical path delay** - timing optimization
- **Power** - reduce switching/routing power
- ... (other objectives)

### Complexity

**Question:** If have n cells to place, how many placements?

**Answer:** **At least n!**

→ **Giant search space!**

This is why placement is such a challenging problem - the solution space grows factorially with the number of cells.

---

## Naive Placement Algorithm

### Brute Force Approach

```
for all placements {
    route (or global route)
    save best
}
```

**Problem:** This will take **far too long...**

With n cells, we have at least n! possible placements:
- 10 cells: 3.6 million placements
- 20 cells: 2.4 × 10¹⁸ placements  
- 100 cells: 9.3 × 10¹⁵⁷ placements

Even if we could evaluate 1 billion placements per second, this is completely intractable.

**Need smarter algorithms!**

---

## Cost Function for Placement

### Definition

**Cost Function:** A function we design or choose to measure the "goodness" of a placement.

### Requirements

✓ **Want this to be quick to compute**
- Will evaluate many placements during optimization
- Can't afford to route each placement
- Need fast proxy for quality

### Common Approach

Use estimated wirelength as primary cost metric:
- Correlates well with routability
- Correlates well with delay
- Fast to compute
- Doesn't require actual routing

---

## Wirelength Estimation: Half-Perimeter Bounding Box

### Most Favorite Wirelength Estimation Method

**Half-Perimeter Bounding Box Wirelength ("HPWL")**

### For a Single Net

**Visual representation:**
```
        • pin
        │
    ┌───┼───────────┐  Δy
    │   │           │
    │   └─• pin     │
    │               │
    │        • pin  │
    └───────────────┘
    
    ←─────Δx───────→
```

**Formula:**
```
half-perim WL = Δx + Δy
```

Where:
- Δx = (max x-coordinate) - (min x-coordinate) of all pins
- Δy = (max y-coordinate) - (min y-coordinate) of all pins

### Total Placement Cost

```
Placement cost = Σ HPWL(i)
                i∈nets
```

**Sum over all nets in the design**

**Note:** HPWL is **very widely used** in placement tools because:
- Fast to compute (O(pins) per net)
- Good proxy for actual routed wirelength
- Differentiable (useful for analytical placers)

---

## Accuracy of HPWL

### When is HPWL an Accurate Measure of True Routed WL?

**2-pin net:**
```
•─────┐
      │
      │
      │
      └──────────•
```
✓ These ways to route **match with HPWL**
- Any L-shaped or rectilinear path has length = Δx + Δy
- HPWL is exact for 2-pin nets

**3-pin net:**
```
•────────────────────────•
│                        │
│                        │
│                        │
│        • pin           │
└────────┘               │
                         •
```
✓ These **match with HPWL**
- Steiner tree routing approaches HPWL
- For well-distributed pins, HPWL is good estimate

### General Observations

**HPWL works well when:**
- Nets have few pins (2-4 pins)
- Pins are arranged in roughly rectangular patterns
- Using rectilinear (Manhattan) routing

**HPWL may underestimate when:**
- Large multi-pin nets
- Pins require complex Steiner tree
- Severe congestion forces detours

**Despite limitations, HPWL remains the most popular wirelength metric for placement.**
