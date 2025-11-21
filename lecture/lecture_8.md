# ECE 1387 Lecture #8 - Partitioning by Branch & Bound

**Date:** November 7, 2025

**Announcements:**
- Partitioning by Branch & Bound
- A3 posted
- Ex1 due today
- Quiz 1 today

---

## Page 2: Bipartitioning Problem Definition

### Bipartitioning
(Break into 2 pieces, as opposed to K-way → break into k pieces)

**Given:**
- Netlist of blocks, all equal size
- Represent as a graph $G(V, E)$
  - $v_1, v_2, \ldots, v_n$ : cells in design
  - $e_1, e_2, \ldots, e_n$ where $e_i = (v_i, v_j) \in E$

---

## Page 3: Objective

**Find:**
- Bipartition of nodes
- Approximately equal number of nodes in each partition
- Minimize # edges (wires) between partitions
  - (minimize **cut**)
  - (minimize **crossing count**)

---

## Page 4: Formal Problem Statement

**Find** $V_1$, $V_2$ such that:
- $V_1 \subset V$, $V_2 \subset V$
- $V_1 \cup V_2 = V$
- $V_1 \cap V_2 = \phi$
- $||V_1| - |V_2|| < \epsilon$ ← **imbalance factor**

---

## Page 5: Objective Function

**Want to minimize** $e = (v_i, v_j)$

$$|E_{ext}| = |\{e \in E \mid v_i \in V_1, v_j \in V_2\}|$$

↑ **external edges** → **crossing count**

---

## Page 6: Real-World Complications

This is simple version, real problems include:

1. Unequal block sizes
2. Community constraints
3. Different block types
4. Fixed blocks
5. Some uncuttable edges
6. K-way
7. Special handling for high-fanout nets
8. Duplication
9. Performance constraints

---

## Page 7: Complexity of Bipartitioning

If $|V| = n$,

**How many partitionings?**

$$\binom{n}{n/2}$$ for equal balance

**If balance is not mandatory?**

$$2^n$$

---

## Page 8: Branch & Bound (B&B)

**Branch & Bound (B&B)**
- General technique, guarantees optimal solution
- Can be applied to many problems

---

## Page 9: Step 1 - Decision Tree

**Step 1:** B&B requires a tree to represent all possible solutions called **decision tree**

- Won't necessarily build whole tree → We will prune it as we go

---

## Page 10: Decision Variables

Set of vertices $v_1, v_2, \ldots, v_n$

Want to know if:
- $v_i \in V_1$ (left) or $v_i \in V_2$ (right)

---

## Page 11: Decision Tree Structure

**One possible decision tree:**

Each level deals with one vertex

```
                    [root]
                   /      \
                v₁L        v₁R
               /  \       /   \
            v₂L   v₂R   v₂L   v₂R    ← v₁,v₃ | v₂
           / \   / \   / \   / \        (partial
         v₃L v₃R ...  ...  ...          solution)
          :
          :
         vₙ
```

- Leaf nodes are complete partitionings

---

## Page 12: Key Insight

- For partial solutions, every node below contains that partial solution

- **Naive way:** Create entire tree
- **B&B is a clever way to avoid looking at many solutions that are provably not optimal**

---

## Page 13: Step 2 - Initial Solution

**Step 2: Initial Solution**

Any initial solution (best one we can get quickly)

Let cost of initial solution (crossing count) = **Best**

---

## Page 14: Getting Initial Solution

**How?**
- Walk circuit graph to decide initial partition
- Randomly (½ left, ½ right)
- Other heuristic (greedy swaps)

---

## Page 15: Step 3 - Bounding Function

**Step 3: Bounding Function**

- Create a bounding function that can be applied to any node on tree: $LB(n)$

- $LB(n)$: **lower bound** on the cost of all possible solutions underneath node n
  - → No solution under n will cost less than $LB(n)$

---

## Page 16: Lower Bound Example

```
            v₁L
           /
        v₂R
       /
    v₃L
   /
  n ●  ────────────────►  [v₁, v₃ | v₂]
                              |
                             v₅
```

→ $LB(n) = 1$ (one edge already cut in partial solution)

---

## Page 17: Properties of Lower Bound

- The higher the better
- Requires some cleverness/innovation
- **If not a true lower bound, optimality is not guaranteed**

---

## Page 18: Step 4 - Traverse Tree

**Step 4: Traverse Tree**

- From root down
- Create tree while traversing (not all up front)

**For each node n visited:**
```
if n is a leaf and cost(n) < Best
    replace Best
    
if balance violated(n)
    prune / don't traverse below
```

---

## Page 19: Pruning Condition

```
if LB(n) ≥ Best
    prune / don't traverse below
```

- **Avoid enumerating solutions provably bad (not optimal)**

---

## Page 20: Step 5 - Termination

**Step 5:** After entire tree is pruned or traversed, **Best** is **optimal**

---

## Page 21: Example Netlist

**Example netlist:**

```
    ①───②───③───④───⑤───⑥
     \___________/    \_/
```

**Initial solution:** 1,2,3 | 4,5,6

**Best = 4**

---

## Page 22: Decision Tree Example

```
         ①───②───③───④───⑤───⑥

                        ← v₁ on left
                          by symmetry
                    [1L]
                   /    \
              v₂L         v₂R
             LB=0         LB=0
            /    \       /    \
         v₃L    v₃R    v₃L    v₃R
        LB=0   LB=1   LB=0   LB=1
        /  \   / \    / \    / \
      v₄R  ... v₄L v₄R  ...  v₄L v₄R
       ✗      LB=1 LB=2      LB=2  ✗
    balance    / \              balance
              5L  5R
             LB=2 LB=3
              ↓
             LB=4 ← lower bound prune
```

Legend:
- Red ✗ = balance prune
- Green ✓ = optimal solution found
- LB=4 = lower bound prune (since LB ≥ Best)

---

## Page 23: Optimal Solution

**Optimal solution:**

$$\frac{1,3,5}{2,4,6}$$

**crossing count = 1**

---

## Page 24: Design Choices - Alternative Decision Tree

**Design Choices**

**1. Decision tree design**

Alternative: Each level corresponds to one **slot**

```
         3 slots  |  3 slots
        [L1|L2|L3] | [R1|R2|R3]

L1:        [root]
          / | \ \ \ \
         1  2  3  4  5  6     ← each level
                              of tree
R1:     /|\ 
       2 3 4 5 6  ...  1 2 3 4   corresponds
                               to one slot
L2:    /|\
      3 4 5 6    ...
```

---

## Page 25: Pros and Cons of Alternative Tree

**Good:** Solves balance problem

**Bad:**
- Much bigger tree
- Because this contains same solution many times

---

## Page 26: More Design Choices

**2. Bounding function** (most important)

**3. Traversal order**
   1. BFS
   2. DFS
   3. **Lowest bound first**

---

## Page 27: Lowest Bound First Strategy

```
        ● tree root
       /|\
      / | \
     ●  ●  ●  ← frontier of built tree
```

- Build from frontier node with **lowest lower bound**
- → Hope it leads to a complete solution cost < Best

---

## Page 28: More Design Choices

**4. Initial solution generation**
   → As good as possible to prune more of tree

**Modification:** Also compute an **upper bound** $UB(n)$ for interior node n.

```
if UB(n) < Best
    replace Best (even though it's a partial solution)
```

---

## Page 29: Upper Bound Requirement

- **Must be a true upper bound or will lose optimality**

---

## Page 30: "Stop Before Exhaustion" Heuristic

**"Stop before exhaustion" heuristic**

- Look at $LB(n)$ for all frontier nodes, compare with Best
- Find maximum difference (max)
- If stop now, we are at most **max** cost from optimal

---

## Page 31: Inexact B&B

**A heuristic based on B&B**

**Inexact B&B** (not A3)

- $LB(n)$ is a heuristic to allow more pruning
  - → No longer optimal
  - → Want $LB(n)$ to be **higher**

---

## Page 32: A3 Example Graphics

**A3 example graphics**

[Image showing a decision tree visualization with numbered levels 8, 25, 11, 19, 9, 18, 7, 12, 17, 5, 26, 16, 21, 3, 13, 14, 1, 23, 4, 2, 22, 6, 24, 10, 15, 20, 27 on the left axis, displaying the branching structure of the B&B algorithm with pruning]