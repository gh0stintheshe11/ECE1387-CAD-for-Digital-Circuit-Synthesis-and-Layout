# ECE 1387 Lecture 11 - November 28, 2025

## Page 1: Introduction

**ECE 1387 Lecture 11**

Nov 28, 2025

Topics:
- Technology mapping, LP/ILP
- Quiz next week
- Course evaluations
- A3 return next week

---

## Page 2: Gate Library and Boolean Network

| Gate | NOT | NAND | AND | OR |
|------|-----|------|-----|-----|
| Symbol | (inverter) | (nand gate) | (and gate) | (or gate) |
| Area | 0.5 | 1 | 1.2 | 1.2 |

**Pattern graph** (showing gate representations as tree structures)

**BN:** $F = ab + cd$

---

## Page 3: Boolean Network Matching Example

**BN** (showing circuit with nodes numbered 1-6)

**C1** A matches  
cost = 0.5

**C2** C matches  
cost = 1.2

**C3** C matches  
cost = 1.2 + cost@1  
= 1.2 + 0.5 = 1.7

**C4** A matches, cost  
0.5 + cost@3  
= 0.5 + 1.7 = 2.2

---

## Page 4: Matching Costs Analysis

**B matches** cost = 1 + cost@1  
**Best** = 1.5

**C5** A matches cost 0.5 + cost@2  
= 1.7

**B matches** cost = 1

---

## Page 5: More Matching Costs

**C6** C matches cost =  
1.2 + cost@4 + cost@5  
= 1.2 + 1.5 + 1 = 3.7

**C7** A matches cost =  
0.5 + cost@6  
= 0.5 + 3.7 = 4.2

---

## Page 6: Final Matching Options

**B matches** cost =  
1 + cost@4 + cost@5  
= 1 + 1.5 + 1 = 3.5

**D matches** cost =  
1.2 + cost@3 + cost@2  
= 1.2 + 1.7 + 1.2 = 4.1

---

## Page 7: Mapping Result

[Diagram showing the Boolean network broken into trees and the resulting mapping]

---

## Page 8: Summary of Algorithm

**Summary of Algorithm**

1) **Traverse subject graph from inputs to outputs (topological order)**

   For each node visited, find all matches of pattern graphs @ node
   
   For each match, determine cost (based on gate costs +

---

## Page 9: Algorithm Continued

   inputs to gate cost)
   
   - Choose best match (with lowest cumulative cost), store in table

2) **Traverse subject graph in reverse order (outputs to inputs), construct mapping**

---

## Page 10: Delay Optimization

**Also works for delay, just carry forward slowest path**

---

## Page 11: Sub-problem - Matching

**Sub-problem:** matching

- how to check if pattern matches?

**tree matching "Structural"**

- → walk up both trees (from a node on subject grph)
- → and compare
- → have a match if finish with no mismatch

---

## Page 12: Asymmetric Patterns

[Diagram showing asymmetric gate patterns]

**for such asymmetric patterns, can make matching aware of swappable inputs?**

**OR can put multiple variants into pattern graph lib.**

---

## Page 13: Boolean Matching

**also Boolean matching**

→ compare logic functions of std cell gates and subgraphs of subject graph

---

## Page 14: FPGAs

**FPGAs**

- Same basic dynamic programming tech mapping is used
- Matching is a bit different
- k-LUT → any logic function with ≤ k Variables

```
K inputs → [LUT] →
```

Typical k = 4, 6

---

## Page 15: Matching for K-LUTs

**Matching only needs to care about # of inputs (not logic functions)**

[Diagram showing a circuit with nodes and possible cuts]

Say k = 4

3-feasible "cuts" for node ③

$\{i1, i2, 2\}$, $\{i1, i3, i4\}$

**inputs**

---

## Page 16: K-Feasible Cuts

**Each 3-feasible cut is a "match"**  
→ can be implemented in a 3-LUT

**a "cut" can be characterized by its inputs**

**3-feasible cuts for ③**

$\{\{1, 2\}$,  
$\{i1, i2, 2\}$,  
$\{1, i3, i4\}\}$

---

## Page 17: How to Find Cuts

**How to find cuts for node $v$**

Let A, B be two sets of cuts  
↑  
a cut: set of nodes producing signals crossing cut

$A \diamond B = \{p \cup q \mid p \in A, q \in B, |p \cup q| \leq k\}$

---

## Page 18: Set Cuts for Node

**Set cuts for $v$, $\phi(v)$**  
with fanins $n_1, n_2, \ldots, n_m$

$\phi(v) = \{\phi(n_1) \diamond \phi(n_2) \ldots \diamond \phi(n_m)\} + \{\{v\}\}$

↑  
trivial cut

---

## Page 19: Finding K-Feasible Cuts

**Can find k-feasible cuts for node $v$ by combining cuts from $v$'s fanins**

---

## Page 20: (Integer) Linear Programming

**(Integer) Linear programming**

**ILP / LP** → want to minimize or maximize

$f(x_1, x_2, \ldots, x_n)$

$= c_1 \cdot x_1 + c_2 \cdot x_2 + \ldots + c_n \cdot x_n$ ← obj function

**Subject to constraints**

$a_{11}x_1 + a_{12}x_2 + \ldots + a_{1n}x_n \leq b_1$  
$a_{21}x_1 + a_{22}x_2 + \ldots + a_{2n}x_n \leq b_2$  
⋮

---

## Page 21: LP vs ILP

**if all $x_i$'s are Real, LP** (Poly time)  
**if all $x_i$'s are Int, ILP** (NP-hard)

**Example:**

Joe writes 15 lines/hour  
Sally writes 20 lines/hour

Joe tests 25 lines/hour  
Sally tests 15 lines/hours

---

## Page 22: Work Allocation Problem

**Joe, Sally work 8 hours a day**

**How to allocate Joe & Sally's time?**

**JT:** # hours Joe tests  
**JW:** # hours Joe writes  
**ST:** # hours Sally tests  
**SW:** # hours Sally writes

---

## Page 23: Objective Function

**Objective function**

max $15 JW + 20 SW$

**Subject to:**

$JT + JW = 8$  
$ST + SW = 8$  
$15 JW + 20 SW = 25 ST + 15 ST$

$JT \geq 0$, $JW \geq 0$, $ST \geq 0$, $SW \geq 0$

---

## Page 24: NoC Placement Problem

**NoC (Network on Chip) Placement**

[Image showing 4×4 mesh NoC with coordinates (0,0) to (3,3) and a CAD flow diagram for NoC-CGRA architecture]

**HEART'24**

---

## Page 25: ILP Variables for NoC

**∀i ∈ Partition** $X_i, y_i$: position of partition i in NoC mesh

**∀i,j ∈ Partitions:** $rx_{i,j}$, $rx_{j,i}$, $ry_{i,j}$, $ry_{j,i}$  
→ binary variables

**∀n ∈ Connections:** $lx_n$, $hx_n$, $ly_n$, $hy_n$  
are integer variables representing  
lowest x, highest x, low y, high y for Conn n.

---

## Page 26: Objective Function for NoC

min $\phi = \sum_{n \in \text{Conns}} (hx_n - lx_n) + (hy_n - ly_n)$

↳ HPWL

**Subject to:**

---

## Page 27: NoC Boundary Constraints

$X_i \geq 0$, $X_i \leq 3$  
$y_i \geq 0$, $y_i \leq 3$

→ Stay inside NoC mesh

---

## Page 28: Non-Overlap Constraints

**∀i,j ∈ Partitions, i≠j:**

- $X_i < X_j + rx_{i,j} \cdot M$
- $X_j < X_i + rx_{j,i} \cdot M$
- $y_i < y_j + ry_{i,j} \cdot M$
- $y_j < y_i + ry_{j,i} \cdot M$

**∀i,j ∈ Partitions, i≠j**

$rx_{i,j} + rx_{j,i} + ry_{i,j} + ry_{j,i} \leq 3$

---