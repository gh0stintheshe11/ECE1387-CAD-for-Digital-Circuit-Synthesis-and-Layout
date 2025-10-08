# ECE 1387 Lecture 1 Part 2 - Synthesis in ICs

**Date:** September 5, 2025

---

## Synthesis in ICs (Integrated Circuits)

### IC Media: Different Kinds of ICs

IC media refers to the different kinds of ICs that could be the end target of CAD flow:

#### ① Full-custom ULSI
- Drawing rectangles/routes by hand
- Rarely done these days
- **Exceptions:**
  - Diffusion
  - FPGA logic/routing arch
  - Analog/RF
  - RAM cells
  - SAT

[Diagram shows a simple gate structure]

#### ② Semi-custom ULSI
- **Standard cells**
  - Library of cells provided by foundry (e.g., TSMC, ST Micros, etc.)
  - ⟹ Inverter, NAND, XOR, ......
  - Multiple versions of each cell with different sizes (different drive strengths)
  - Each cell has a layout
- **Output:** all mask layers

#### ③ Field-programmable
- **FPGAs** (Field-Programmable Gate Arrays)
- **CGRAs** (Coarse-grained reconfigurable arrays)
- **Output:** Configuration bitstream

**Note:** We will stray between all three.

---

## Optimization is Key Part of CAD

### What is optimization?

- ∃ many ways to implement same function
- Some have:
  - good **area**
  - " " **delay**
  - " " **power**
- **Typically trade-offs between these**

**Optimization is a way to search through some (all?) choices to find one you want** → target speed, good area

---

## Optimization Strategies

**Key strategies covered (highlighted in notes):**
- **Simulated annealing**
- **Graph search**
- **Linear programming**
- **Convex optimizations**
- **Branch-and-bound**
- **Dynamic programming**

**Other strategies:**
- Genetic algorithms
- Reinforcement learning
- Gradient descent
- Greedy algorithm
- Ant colony optimization
- Network flow

---

## CAD Sub-tasks

Many sub-tasks have evolved in the synthesis of ICs.

We will cover main subtasks in ECE1387.

---

## Top-to-Bottom View of Synthesis Tasks

### Input:
- Desired behavior of chip @ behavioral (software-like) level
- **Constraints:**
  - Speed spec
  - Area spec
  - Power spec
  - etc.

### Output:
- All mask layers (std cells)
- Config bitstream (field prog.)

---

## 3 Major Steps

### Step 1: High-level Synthesis

```
Software-like behavioral description
- cycle-by-cycle behaviour not specified (untimed)
           ↓
IC media → High-level Synthesis  
          (Behavioural)          
                                   Tasks:
                                   - scheduling
                                   - binding
                                   ~ loop unrolling
                                   - compiler opts
           ↓
          RTL (Register Transfer Level)
          Verilog or VHDL          optimization
          - cycle-by-cycle
            behaviour is fixed
           ↓
```

### Step 2: Logic Synthesis

```
IC media → Logic Synthesis
                                   Tasks:
                                   - Boolean optimization
                                   - retiming
                                   - FSM optimization
                                   - technology mapping
                                   optimization
           ↓
          Netlist of connected
          cells and larger blocks
          (e.g. standard cells or
          LUTs, FFs in FPGA case)
           ↓
```

### Step 3: Layout Synthesis

```
IC media → Layout Synthesis
                                   Tasks:
                                   - Partitioning
                                   - Floorplanning
                                   - Placement
                                   - Routing      optimization
           ↓
          All mask layers or
          config bitstream
           ↓
```

---

## Course Strategy

The course will cover the intersection of three key areas:

**[IC media]** ⟷ **[CAD task]** ⟷ **[optimization & strategy]**

**Approach:** Pick one of each and describe in depth.