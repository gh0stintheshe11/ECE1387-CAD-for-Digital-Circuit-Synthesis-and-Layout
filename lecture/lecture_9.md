# ECE 1387 Lecture #9 - Partitioning (Continued)

**Date:** November 14, 2025

**Topic:** Partitioning cont'd...

---

## Page 2: Scalable Partitioning

**Now...**

**Scalable partitioning**
- Heuristic
- Can apply to huge graph, say 100K vertices

---

## Page 3: FM Partitioning Introduction

**FM Partitioning**

**Fiduccia-Mattheyses**
- DAC'82
- General Electric
- **FM**

---

## Page 4: Definition of Gain

**Definition:** **gain** of a vertex v, $gain(v)$

- Reduction in cut size (crossing count) if vertex v is moved to opposite partition (bipartitioning)

---

## Page 5: Gain Example

**Example:**

```
                    |
        ○          |      ○──○
       /           |        
      /            |        
     ○             |           ○
    +2            +1          0    -1
   gain
```

**Negative gain** means an **increase** in crossing count.

---

## Page 6: FM Algorithm

**FM Algorithm**

```
Given: initial balanced partition V₁, V₂

Repeat:
    Unlock all vertices
    Compute initial gain for all verts.
    
    for i = 1 to n    // n = |V|
        Choose a free (not locked)
        vertex b s.t. b has
        highest gain and moving b
        preserves balance
```

---

## Page 7: FM Algorithm (Continued)

```
        - if no such b exists
              break
        - move b and lock it
        - gᵢ = gain(b)
        - update the affected cell gains  ←(*)
    end for
    
    Find k s.t. G = g₁ + g₂ + ... + gₖ
        is maximized
    
    Shuffle cells up to kᵗʰ step  ←(*)

Until G ≤ 0      ← each iter of
                   repeat is
                   **one pass**
```

---

## Page 8: FM Notes

**(*)** Only cells connected to b could potentially need gain updates.

- Each pass begins with best partitioning from prev. pass **(※)**

---

## Page 9: Hill Climbing in FM

**FM allows hill climbing**

```
cutsize
    ↑
    |  ⌒
    | /  \
    |/    \
    |      \    /⌒\
    |       \  /    \
    |        \/      \
    |                 \
    +------------------→
              ↑      moves during
             kᵗʰ       pass
            step
```

---

## Page 10: Main Breakthrough in FM

**Main breakthrough in FM**

- Let $P$ = # of pins in circuit (graph)
- Each pass takes $O(P)$ time
  - → **linear time!**
- Using clever data structure + reasoning

---

## Page 11: Gain Bucket Data Structure

**Gain bucket data structure**

```
         ┌──────┐
   pmax  │      │
         ├──────┤
MAX      │ j+1  │
gain     ├──────┤──→ [cell]↔[cell]↔[cell]↔ ...
    ↑    │  j   │         ↑
         ├──────┤         │
         │ j-1  │    doubly-linked list of cells
         │  :   │         with gain j
         │  :   │
         ├──────┤
  -pmax  │      │
         └──────┘
         
         pmax: max # of
         pins on a cell

         ┌───┬───┬───┬─ ... ─┬───┬───┐
         │ 1 │ 2 │   │       │ i │   │ n    ← pins on a cell
         └───┴───┴───┴───────┴───┴───┘
```

---

## Page 12: Gain Bucket Operations

**One gain bucket data structure for $V_1$, one for $V_2$**

- Find a cell max gain in $O(1)$ time
- Remove/insert a cell in $O(1)$ time
- Update $gain(i)$ for cell $i$ in $O(1)$ time
- Update max gain pointer in $O(P)$ time // see paper

---

## Page 13: Key Observation

**Key observation:** When a cell b is moved, not all cells connected to b necessarily need gain updates.

---

## Page 14: Non-Critical Net

**Non-critical net**

```
           |
    ○──○──○│○──○──○
           |
```

Has >1 vertex in both partitions

---

## Page 15: Critical Net

**Critical net** has 0 or 1 vertex in one partition ($V_1$ or $V_2$)

```
    ○──○──○──○ |              ○──○──○ | ○
               |                      |
```

---

## Page 16: Critical Net Behavior

- Only nets that are **critical before or after** a move cause gain updates to their cells.

- Once a cell on net n moves from $V_1 \rightarrow V_2$ and another cell on net n moves from $V_2 \rightarrow V_1$, net n is **"dead"**
  - → dead nets do not trigger gain updates

---

## Page 17: Complexity Result

- Every net will bring about changes to the gains of its cells at most k times ($k = 4$)

- **$O(P)$ gain updates/pass!**

---

## Page 18: hMetis Multi-level Partitioning

**hMetis Multi-level Partitioning**
- IEEE TVLSI '99
- Univ of Minnesota

**h = hypergraph**
- → edges can have > 2 nodes

```
       ●
      /|\      e₂
     / | \    /|\
    ●  ●  ●──●─●─●
    \e₁/      \
     \/        ●
              e₃
```

**Size of edge = # nodes on edge**

$|e_1| = 3$, $|e_2| = 4$, $|e_3| = 2$

---

## Page 19: TritonPart

**TritonPart**
- ICCAD'23
- Part of **OpenRoad** Flow

[Reference to paper: "An Open-Source Constraints-Driven General Partitioning Multi-Tool for VLSI Physical Design"]

---

## Page 20: Multi-level Partitioning Algorithm

**Given an input hypergraph:**

1. Construct a seq of successively smaller graphs **"coarsening step"**
   - Done by collapsing certain nodes together in "super nodes"

2. Apply partitioning to smallest graph -- compute bisection

3. Project partitioning onto the next finer level of graph (**"uncoarsening"**)

---

## Page 21: Iterative Refinement

...and apply **iterative refinement** to improve partitioning
- Choose some nodes to move to other partition
- A node may correspond to multiple (many) nodes in original graph

---

## Page 22: Step 4

**#4** Repeat step #3 until we return to original flat graph

---

## Page 23: Multi-level Illustration

**Illustration**

```
                    Coarsening ↓        Uncoarsening ↓
                                        and refinement
    
    g₄              (small)             (small)|partition
                        ↑                   ↓
    ════════════════════════════════════════════════
    g₃              (medium)            (medium)|partition
                        ↑                   ↓
    ════════════════════════════════════════════════
    g₂              (larger)            (larger)|partition
                        ↑                   ↓
    ════════════════════════════════════════════════
    g₁              (larger)            (larger)|partition
                        ↑                   ↓
    ════════════════════════════════════════════════
    g₀              (original)          (original)|final partition
```

---

## Page 24: Benefit

Avoids full blown partitioning on flat graph $g_0$

---

## Page 25: Pieces of hMetis

**Pieces of hMetis**

**1) Coarsening phase**
- Create a smaller graph
- Want a partitioning of smaller graph to be a decent partitioning of larger graph
- Collapse connected cells together

---

## Page 26: Edge Coarsening

**3 ways of coarsening**

**i) Edge coarsening** (used in TritonPart)
- Choose two vertices connected to one another & collapse them

```
Visit verts in random order
For each uncollapsed vertex v
    Find vertex u, s.t. u
    is uncollapsed and (u,v)
    connection has highest weight
```

---

## Page 27: Edge Weight Formula

$$weight(e) = \frac{1}{|e| - 1}$$

→ low fanout nets have higher weight

- Collapse u,v together

---

## Page 28: Edge Coarsening Illustration

[Diagram showing original graph with nodes and hyperedges, then the resulting coarsened graph with collapsed node pairs circled]

---

## Page 29: Hyperedge Coarsening

**Hyperedge coarsening**

Sort hyperedges in decreasing weight order
- → randomly for a given weight

Visit hyperedges in order

For each hyperedge that connects vertices that have not been collapsed
- Collapse whole hyperedge

---

## Page 30: Hyperedge Coarsening Illustration

[Diagram showing hyperedges collapsed into super-nodes. Notes that "Nodes have different #s of internal nodes"]

---

## Page 31: Modified Hyperedge Coarsening

**Modified Hyperedge Coarsening**

- After hyperedge coarsening
- Traverse hyperedges again
- For each hyperedge not collapsed, collapse its remaining nodes

---

## Page 32: Coarsening Options Comparison

**Coarsening Options**

[Figure from ResearchGate showing three coarsening methods side by side:]

| Edge Coarsening | Hyperedge | Modified Hyperedge |
|-----------------|-----------|-------------------|
| Pairs of connected vertices matched | Entire hyperedges collapsed | Hyperedges + remaining nodes |

---

## Page 33: Initial Partitioning of Small Graph

**Initial partitioning of small graph** (< 200 vertices)

- Two approaches (hMetis):
  1. Random partitioning
  2. BFS from a random vertex

- Other ideas:
  - B & B
  - ILP (integer linear programming)
    - → TritonPart