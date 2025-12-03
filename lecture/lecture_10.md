# ECE 1387 Lecture 10 - November 21, 2025

## Page 1: Introduction

**ECE 1387 Lecture 10**  
Nov 21, 2025

Topics:
- Partitioning
- Technology mapping

---

## Page 2: Course Paper Requirements

**Course Paper**  
25 marks total

- 5 marks: Presentation
- 10 marks: Survey
- 10 marks: Critique/critical analysis

**Discuss/email topic by Dec. 1**

---

## Page 3: Refinement of FM Algorithm

**Refinement**

1) **FM algorithm**
   - **Early exit FM**: Halt a pass if X% of swaps did not improve crossing count.
     - Say X = 1%

2) **Limit FM to 2 passes**

---

## Page 4: Limitation of FM

**Limitation of FM**

Say balance constraint is 60/40 split

```
Partition 1:        |        Partition 2:
  O (-1)            |          O    O
  O (-1)            |          O    O
  O (-1)      O (highlighted)  O    O
  O           O (highlighted)  
  O (-1)            |          O    O
  O (-1)            |
```

**FM may move any of verts with -1 gain**

---

## Page 5: Desired Move

**But what we really want is to move two (highlighted) vertices to other side**

---

## Page 6: Hyperedge Refinement

**Limitation of FM leads to "hyperedge refinement"**

- Randomly visit hyperedges that are cut
- Determine if can move subset of vertices on hyperedge so no longer cut
- A greedy approach
- This is followed by FM-based approaches

---

## Page 7: Multi-phase Refinement

**Multi-phase Refinement**

```
         Coarsening → V Cycle → V Cycle → Refinement
HG_0                                           HG_0
HG_1      V          HG_1      V              HG_1
HG_2                 HG_2                     HG_2
HG_3                 HG_3                     HG_3
HG_4                 HG_4                     HG_4
```

**V Cycles**: restricted coarsening  
→ only vertices in same partition can collapse

---

## Page 8: Technology Mapping

**Technology Mapping** → don't care optimization  
(part of logic synthesis)

**Logic synthesis has 2 steps:**

1) **Tech-independent optimization**
   - Like K-maps (but in algorithms)
   - FSM optimizations
   - Removing "dead circuitry"
   - Retiming?

2) **Tech-mapping**
   - Optimized netlist is turned into a network of gates from target library

---

## Page 9: TM Problem Definition

**TM problem definition**

**Given:**
1) Boolean network $G(V,E)$ with dependencies
2) Library of available gates (standard cells)

**Find:** A netlist of available gates that implements functionality of $G$

**So as to minimize:** area, power, delay, etc.

---

## Page 10: Focus on ASIC

**We will consider TM to ASIC standard cells...**

---

## Page 11: Example Standard Cell

**Example standard cell**

[Image shows NAND-2 standard cell layout with VDD rail at top, VSS rail at bottom, inputs A and B, and output]

**NAND-2 standard cell**

**All cells have same height**

Reference: https://teamvlsi.com/2020/05/standard-cells-in-asic-design-standard-cells-in-vlsi.html?amp=1

---

## Page 12: Example Boolean Network

**Example Boolean network:** $y = f(a+q)$

```
    a ──┐
        ├──[OR]──┐
    q ──┘        │
                 ├──[AND]── y
    s ───────────┘
```

**Library:**

| Element | Area | Delay |
|---------|------|-------|
| ─[>o─   | 3    | 1     |
| ─[D──   | 6    | 2     |
| ─[>─    | 8    | 2.5   |
| ─[D─    | 8    | 2.5   |
| ─[>D─ (OR-AND cell) | 25 | 4 |

**Implementation 1:**
- area = 8 + 6 + 3 = 17
- delay = 2.5 + 2 = 4.5

---

## Page 13: Alternative Implementation

```
    a ──┐
    s ──┼──[>D──┐
    f ──┘       ├──[>── y
                │
```

**area = 8 + 8 = 16**  
**delay = 2.5 + 2.5 = 5**

---

## Page 14: Real Situation

**This is a small library & BN**

**Real Situation:**
- Library with dozens or 100+ cells
- User constraints (area, delay)
- Larger circuit

---

## Page 15: DAGON Paper

**DAGON: Technology Binding and Local Optimization by DAG Matching**

*Kurt Keutzer*  
AT&T Bell Laboratories  
Murray Hill, New Jersey 07974

→ UC Berkeley  
→ CTO Synopsys

[Abstract excerpt visible discussing technology binding and pattern matching]

---

## Page 16: Step 1 - Base Functions

**Step 1:** Take Boolean network and rewrite using a set of "base functions"

**Popular choice for base functions:**
- 2-input AND + inverters

**AIG: AND-inverter graph**  
→ Used in ABC logic synthesis tool (UC Berkeley)

---

## Page 17: Subject Graph Example

$x = de(a+b)$ express as AIG

Using De Morgan's law:
$a+b = \overline{\overline{a} \cdot \overline{b}}$

```
        x
        │
       [A]
      /   \
     d    [I]
           │
          [A]
         /   \
        [I]  [I]
        │    │
        a    b
```

**Called "subject graph"**  
→ Subject of our optimizations

---

## Page 18: MockTurtle Tool

**EPFL logic synthesis tool**  
**MockTurtle**

- Base functions are Majority + inv

**MIG: majority inverter graph**

---

## Page 19: Majority Function

**Majority function**

$F = ab + bc + ac$ (Carry function in full adder)

- if $c = 0$, $F = ab = \text{AND}$
- if $c = 1$, $F = a + b = \text{OR}$

**Jin Hee work:** use MIG to map regular logic onto FPGA carry chain

---

## Page 20: Step 2 - Express Library

**Step 2:** Express all gates in library in same base functions

---

## Page 21: Pattern Graph Examples

**Examples**

**Gate** → **Pattern graph**

```
─[>o─  (INV)      ──[>o──

─[D──  (OR)       ──[>o──[A]──[>o──
                          │
                          
─[D─   (AND)      ───────[A]─────
                        /   \

─[>D─  (AND-4)    [>o─[A]───[A]──
                      │ \   / │
                      └──[A]──┘
```

---

## Page 22: Multiple Pattern Graphs

**May be multiple pattern graphs for given gate (like AND-4)**

**Find all such pattern graphs**

---

## Page 23: Standard Cell Properties

**Now have subject graph and many pattern graphs.**

**Because the gates are standard cells, Know:**
- area of each
- (something about) delay

---

## Page 24: Covering Problem

**Pose TM problem as a "covering" problem. Well known problem in graph theory**

**Def:** A cover of subj. graph by other pattern graphs is a network of pattern graphs such that:

---

## Page 25: Cover Conditions

**i)** Every node of subject graph must be covered by a node of a pattern graph that matches its type.

**ii)** Legal connectivity between pattern graphs: each input required by a pattern graph is produced by an output of another pattern graph in network.

---

## Page 26: Illegal Cover Example

```
[Diagram showing pattern graphs with some nodes circled]
```

**No legal** ✗

**Since (highlighted) needs inputs that are not produced by other pattern graph outputs**

---

## Page 27: Minimum Cost Cover

**TM is to find a minimum cost cover.**

**Cost:** 
- **Area** = $\sum$ Area of gates
- **Delay** = max delay along critical path of mapped circuit

**Exact covering problem is very hard - NP-hard**

---

## Page 28: DAGON Simplifications

**To simplify, DAGON breaks:**

**(1)** subject graph into trees (all fanout = 1) [**not done today**]

**(2)** restricts gates to be trees too

**Covers trees with trees, and glues back together**

---

## Page 29: Network Decomposition

**Network broke into 3 trees**

```
[Diagram showing a network decomposed into 3 separate tree structures, 
with nodes marked in red where the network was split]
```

---

## Page 30: Dynamic Programming Approach

**optimization approach:** How to cover a tree with trees?

**Dynamic programming**

Applies to problems with two key ingredients:

**1) "optimal substructure":** overall soln can be broken into subprobs. Solve subprobs optimally. Stitch together to form optimal soln of overall problem

---

## Page 31: Dynamic Programming (cont.)

**2) Overlapping sub probs** - In solving overall prob, we "visit" (need to solve) same sub probs many times (can store soln in a table)

**Basic idea:**

1. Break overall prob into subprobs
2. Solve subprobs optimally
3. Use sols to subprobs to compose/construct soln to overall prob