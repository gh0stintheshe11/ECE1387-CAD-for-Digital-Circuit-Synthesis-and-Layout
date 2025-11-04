# Lecture 7 - Simulated Annealing (cont.) & Partitioning

**Course:** ECE 1387  
**Date:** Oct 24, 2025  
**Announcements:**
- A1 marked
- Ex 1 to be posted
- Quiz 1 end of next Lecture #8
  - examples posted
- No class next week!
- Go Blue Jays!

---

## Page 2: A1 Distribution Results

```
              distrib    top/bottom
cct1          2/35 AP, 2V    2/56 Att    ? >4
                             3/164 CM

cct2          3/120 Att      3/164 CM

cct3          4/708 AD       4/730 LQ

cct4          5/2560 LQ      6/2286 2V
```

---

## Page 3: Starting Temperature

**The Pieces of SA:**

**Starting temp** - "hot" enough so most moves would be accepted, e.g. 90%

"acceptance rate" = $\frac{\# \text{accepted}}{\# \text{total moves}}$

- make say N moves
- get ΔC for each, choose typical way
- $T_0$, so 90% of N would be accepted

---

## Page 4: Stopping Criteria

**Stopping criteria:** way of deciding when to stop SA.

- could look at how cost is changing over last temp, or last several temps

⇒ terminate if cost not improving (much).

---

## Page 5: Inner Loop Criteria

**inner-loop criteria:** way to decide when to lower T

- could use statistics of ΔC values to make a decision

- could just do a fixed # of moves per temp (in VPR tool)
  
  $\Rightarrow \# \text{moves}/T = N^{4/3}$ where N = # of objs to place

---

## Page 6: Generate Function

**Generate:** key is random

- given current state, choose a new one

(e.g. in placement, swap cells)

- want it to be fast to calculate ΔC

- want to design it so any state can be reached

---

## Page 7: Accept Function

**Accept:** if $\Delta C < 0$, accept

⟹ these moves reduce cost always

if $\Delta C \geq 0$, maybe accept

$P(\text{Accept}) \uparrow$ when T is high

$P(\text{Accept}) \downarrow$ when ΔC is very large

---

## Page 8: Acceptance Probability

**Most typical** $P(\text{Accept}) = e^{-\frac{\Delta C}{T}}$

```
P(Accept) ↑
    │   ╲╲╲  ↗ higher T
    │    ╲╲╲
    │     ╲╲╲___
    │      ╲╲╲___
    │       ╲╲╲____
    │   ↙ lower T
    └─────────────────→ ΔC
```

---

## Page 9: Update Temperature

**Update T:** how much to lower temperature?

$T_{\text{new}} = \alpha \cdot T_{\text{old}}$ } Common way

α = some # < 1, e.g. 0.9

Also, ∃ "adaptive" cooling schedule: adjust T based on ΔC statistics, acceptance rate, ...

---

## Page 10: Problem Nature Affects SA Performance

Above is general SA framework

How well it works is a function of:

**i. The nature of the problem**

```
cost ↑         SA is good    cost ↑
    │╲                           │  ╲  ╱╲  ╱
    │ ╲╲                         │ ╲╱  ╲╱
    │  ╲╲                        │
    │   ╲╲___                    │
    │    ╲╲___                   │___________
    └──────────────→ position    └──────────→ pos
                                 SA maybe
                                 not great
```

---

## Page 11: Implementation Factors

**ii) The implementation**

- **i) cost function design**
  - SA "best feature" →
  - can almost anything

- **ii) move generation**

- **iii) cooling schedule**
  - how to adjust T

---

## Page 12: Example - TimberWolf

**Example SA-based Placement Implementation: TimberWolf**

UC Berkeley 1980's

- Standard cell placement

---

## Page 13: TimberWolf Cost Function & Moves

**1) Cost Function:** HPWL (like in A2)

**2) Move generation:**

2 types of move

1. pick a random cell, move a random distance (DISPLACE)

2. pick 2 random cells, swap them (interchange positions) (EXCHANGE)

---

## Page 14: Standard-Cell Design Methodology

**Standard-cell Design Methodology**

- All cells equal height

```
VDD ─┬───┬───┬───┬───┬──
     │┝┫ │⌐D│ │ ┌─┐
     │   │   │ │ OR│
     │   │   │ │ │ │
GND ─┴───┴───┴───┴───┴──
```

- Different cells have different width

---

## Page 15: Issue #1 - Overlaps

**Issue #1:** moving a cell or interchanging causes overlap

**What to do?**

1. Shuffle or shift cells to remove overlap
   - can mess up inter-row connections

2. TW sol'n → allow overlaps, penalize overlaps in cost function
   
   = $K_1 (\text{Amount of overlap})^2$

---

## Page 16: Issue #2 - Efficiency at Low Temperature

**Issue #2:** @ low T, unlikely that a long-range random move would be accepted

⇒ meaning, SA not efficient as most moves are rejected

**Sol'n:** TW implements a range window. Moves can only happen in window

---

## Page 17: Range Window Reduction

Initially, window size is whole chip

As $T \downarrow$, lower/reduce window size

⟹ cells can only move in a local region

---

## Page 18: Issue #3 - Row Length

**Issue #3**

- long rows are bad
- waste Si area

- penalize excess row width in cost function
  
  = $K_2 |\text{width}(\text{row}) - \text{AvgWidth}|^2$

---

## Page 19: Complete Cost Function

**Cost function**

= HPWL + $K_1 \cdot$ Overlap Penalty + $K_2 \cdot$ Row Penalty

$K_1, K_2$: weights chosen experimentally

---

## Page 20: Example - Xilinx Virtex FPGA I/O Placement

**Example SA-based I/O placement**

```
Xilinx              ┌──┬──┐  signaling
Virtex              │  │  │← standards
FPGA →              ├──┼──┤  Configurable
8 I/O banks         │  │██│  @ bank
                    │  │██│← I/O bank level
Ⓐ I/O with signaling│  │  │
  standard Ⓐ       └──┴──┘  winner = Ⓐ
```

---

## Page 21: I/O Bank Cost Function

For each bank $z$, Let

winner$(z)$: most common std in $z$

$$\text{Cost} = \sum_{z \in \text{banks}} \# \text{ of I/os in } z \text{ incompatible with winner}(z)$$

+ HPWL + perf + ....

---

## Page 22: Partitioning Introduction

# Partitioning

**Informal definition:** break a circuit into pieces, so the pieces are as disconnected as possible

(# of "cut" signals [between pieces] is minimized)

---

## Page 23: Partitioning Applications

**Many applications:**

- Huge circuit, partition ⇒ place/route pieces (partitions) in parallel

- Input to floorplanning

- Multi-FPGA prototyping system (Synopsys) for ASIC verification

---

## Page 24: More Partitioning Applications

- Distributing an algorithm represented as a graph across processing units

- social networks
  - Find tightly connected pieces in "friend" graph
  - for recommendations, directed advertising