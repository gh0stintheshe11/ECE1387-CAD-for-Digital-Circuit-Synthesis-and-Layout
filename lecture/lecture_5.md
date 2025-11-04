# Lecture 5 - Analytical Placement (AP)

**Course:** ECE1887  
**Date:** October 10, 2025  
**Note:** A2 is almost posted  
**Today's Topic:** Analytical placement (AP)

---

## Page 2: HPWL Limitation for High Fanout Nets

**5-pin example:**

```
        ●  double
       /|\  counted
      / | \
     ●  ●  ●
      \ | /
       \|/
        ●
```

- HPWL is an **underestimate** of WL for high fanout nets

---

## Page 3: Placement Algorithms Overview

Now we know a way to estimate placement goodness

**Many different placement algorithms**

This course:
- **analytical placement** (today)
- **Simulated annealing**

---

## Page 4: Analytical Placement (AP) Introduction

# Analytical Placement (AP)

- widely used in commercial FPGA, ASIC placers
- scalable to large designs
- papers from 90s, still active research today

**Main idea:**
- "compute" a placement for all cells by solving a system of equations

---

## Page 5: Quadratic Placement

**Most popular form:**
# Quadratic placement

**Why?**

Because it minimizes quadratic WL (squared WL)

---

## Page 6: 1D Placement Problem

**Consider 1D placement problem:**

```
[A]────①────[B]
 xₐ    x₁?   xᵦ
fixed       fixed
```

**min Linear WL**

$$\min \{ |x_1 - x_A| + |x_1 - x_A| + |x_1 - x_B| \}$$

**min quadratic WL**

---

## Page 7: Why Quadratic WL?

$$\min \{ (x_A - x_1)^2 + (x_A - x_1)^2 + (x_1 - x_B)^2 \}$$

```
[A]────①────[B]
```

**This is worse WL than minimizing linear!**

So, why minimize quadratic WL?
- **because we can easily!**

---

## Page 8: AP Inputs

# AP inputs

- netlist of cells to place
- **net model** (later)

**gives us:**
- a weight $w_{i,j}$ representing connectivity between cells $i, j$

- weight $w_{i,j} = 0$ if cells $i, j$ are not connected to each other

---

## Page 9: AP Cost (Objective) Function

**minimize:**

$$\Phi = \sum_{i \in \text{cells}} \sum_{j \in \text{cells}} w_{i,j} \cdot (x_i - x_j)^2 + w_{i,j}(y_i - y_j)^2$$

**Can break into two probs:**

$$\min \Phi_x = \sum_{i \in \text{cells}} \sum_{j \in \text{cells}} w_{i,j}(x_i - x_j)^2 \quad \min \Phi_y = \ldots$$

Can minimize independently

---

## Page 10: AP Example Formulation

```
           w₁,₂    w₂,ᵦ
[A]───w_{A,1}───①───────②───────[B]
 xₐ             x₁?     x₂?      xᵦ
```

**what we are solving for**

$$\min \Phi = w_{A,1} \cdot (x_A - x_1)^2 + w_{1,2} \cdot (x_1 - x_2)^2 + w_{2,B}(x_2 - x_B)^2$$

---

## Page 11: Solving via Derivatives

$$\min \Phi = w_{A,1} \cdot (x_A - x_1)^2 + w_{1,2} \cdot (x_1 - x_2)^2 + w_{2,B}(x_2 - x_B)^2$$

$$\frac{d\Phi}{dx_1} = -2w_{A,1}(x_A - x_1) + 2w_{1,2}(x_1 - x_2) = 0$$

$$(w_{A,1} + w_{1,2})x_1 + (-w_{1,2})x_2 = w_{A,1} \cdot x_A$$

$$\frac{d\Phi}{dx_2} = -2w_{1,2}(x_1 - x_2) + 2w_{2,B}(x_2 - x_B) = 0$$

---

## Page 12: Matrix Form Derivation

$$\frac{d\Phi}{dx_2} = -2w_{1,2}(x_1 - x_2) + 2w_{2,B}(x_2 - x_B) = 0$$

$$-w_{1,2}x_1 + (w_{1,2} + w_{2,B})x_2 = w_{2,B} \cdot x_B$$

**In Matrix form:**

$$\begin{bmatrix} w_{A,1} + w_{1,2} & -w_{1,2} \\ -w_{1,2} & w_{1,2} + w_{2,B} \end{bmatrix} \begin{bmatrix} x_1 \\ x_2 \end{bmatrix} = \begin{bmatrix} w_{A,1} \cdot x_A \\ w_{2,B} \cdot x_B \end{bmatrix}$$

---

## Page 13: Linear System Solution

**Matrix form** - can solve with standard linear system solver

$$Q_x \cdot \vec{x} = \vec{b_x}$$

- $n \times 1$ vectors // $n = \#$ moveable cells
- $n \times n$ matrix

$Q_x$ is **symmetric** & **sparse**

---

## Page 14: Fixed Cells Requirement

- $\vec{b_x}$ non-zero elements arise from connections between moveable & fixed cells

$\Rightarrow$ if $b_x = 0$ then trivial $\text{sol} = \vec{0}$ !

**Need some fixed cells for AP to work**

---

## Page 15: Trick to Formulate $Q_x = b_x$

**Trick to formulate**
$$Q_x \vec{x} = \vec{b}$$

- $Q(i, i) =$ sum of all weights on cell $i$
- $Q(i, j) = -w_{i,j}$ for $i \neq j$
- $b(i) =$ Sum of all weights × Fixed positions of cells connected to $i$ (not include connections to moveable cells)

---

## Page 16: How to Get Edge Weights?

**How to get edge weights $w_{i,j}$?**

**Easy way:** clique model

say 4-pin net:

```
    ●        ●─────w─────●
   / \        │ ╲    ╱  │
  /   \       │   ╲╱w   │
 ●     ●      w   ╱╲    w
  \   /       │ ╱    ╲  │
   \ /        │╱      ╲ │
    ●        ●─────w─────●
              ────w────
```

---

## Page 17: Clique Model Weight Formula

**Usually for a p-pin net**

Set $w = \frac{2}{p}$

**Rationale:** p-clique has $\frac{p \cdot (p-1)}{2}$ edges

So, total weight = $\frac{p \cdot (p-1)}{2} \times \frac{2}{p} = (p-1)$ # of edges in a tree with p nodes

---

## Page 18: Bound-to-Bound (B2B) Model

# Bound-to-bound (B2B) model

- TU Munich 2008, widely used
- Given a p-pin net
- **exactly models HPWL**

```
     ●           3 crossings
    ╱│╲         (in general p-1
   ╱ │ ╲        crossing for
  ●  │  ●──────  p-pin net)
   ╲ │ ╱   │
    ╲│╱    │
     ●     ●
```

$$w_{x_{i,j}} = \frac{1}{(p-1)|x_i - x_j|} \cdot (x_i - x_j)^2$$

---

## Page 19: Downside of B2B Net Model

**Downside of B2B net model**

- need a **placement** to compute weights
- weights are different in $x, y$ dimensions

---

## Page 20: Iterative Process

```
need to      ↓ x,y placement
iterate to   
get          ┌─────────────────────┐
weights      │ Adjust weights      │ ← no
             │ using x_old, y_old  │    │
             └──────────┬──────────┘    │
                        ↓                │
                  ┌─────────────┐       │
                  │ Solve for new│      │
                  │ Placement    │      │
                  └──────┬───────┘      │
                         ↓ x_new, y_new │
                    ┌──────────┐        │
                    │ compare  │        │
           yes,close│ x_new,y_new│ are they
             ↓      │ with      │  close?
          found     │ x_old,y_old│      │
          weights   └────────────┘──────┘
```

---

## Page 21: AP Output - Overlaps

**AP output so far...**

```
           blob
           of
    ● ● ●  moveable
  ● ████████ cells
  ● ████████
  ● ████████  
  ● ████████ ●
    ● ● ●    ●

Fixed       
I/Os        
           ● Spread
          cells to
          remove
          overlaps
```

---

## Page 22: Commercial Implementation

Screenshot showing:

**ACM Digital Library**

**RESEARCH-ARTICLE**

# Multi-Commodity Flow-Based Spreading in a Commercial Analytic Placer

**Authors:** Nima Karimpour Darav, Andrew Kennings, Kristofer Vorwerk, Arun Kundu

**FPGA '19:** Proceedings of the 2019 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays • February 2019 • Pages 122–131

**Note:** Microchip/Microsemi

**Spreading is how AP algorithms differ**

**Best paper**

---

## Page 23: Bin Grid Spreading

**Spreading** - overlay a "bin grid" on placement area

```
┌────────────────────┐
│    │    │  ● │    │
├────┼────┼────┼────┤
│    │ ●  │ ●● │    │
│    │ ●● │ ●● │    │
├────┼────┼────┼────┤
│    │ ●● │ ●● │    │
│    │ ●● │ ●● │ ●  │
├────┼────┼────┼────┤
│    │    │    │    │
└────┴────┴────┴────┘
```

- want to remove overlaps
- with minimum cell movement

---

## Page 24: Spreading Algorithm Concept

for each overused bin $b_i$:
- find selected paths to bins with capacity (underused)
- move cells along such paths
- cell movement can be at most $\psi$ (allowable cell movement)
- $\psi$ increases slowly

---

## Page 25: Supply Definition

$$\text{supply}(b_i) = \max(0, \text{usage}(b_i) - \text{capacity}(b_i))$$

$\text{supply}(b_i) > 0$ means bin $b_i$ is overused.

---

## Page 26: Spread vs Solved Placement

**Finding the "spread" placement (as opposed to the "solved" placement from linear system solver)**

---

## Page 27: Spreading Algorithm Pseudocode (Part 1)

```
iter = 0
Repeat
    ψ = max movement (iter) // 2×iter²
    B = Set of overfilled bins
        (sorted in ascending order
         of supply)
    for each bin bᵢ ∈ B do
        P(bᵢ) = List of Candidate
                paths for bᵢ
                sorted in ascending cost
```

---

## Page 28: Spreading Algorithm Pseudocode (Part 2)

```
        for each Pₖ ∈ P(bᵢ) do
            if supply(bᵢ) > 0
                move cells over Pₖ
                end if
            end for
        end for
        iter++
    until B is empty // no overused bins
```