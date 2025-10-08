# ECE 1387 Lecture #3

**Date:** Sept 26, 2025

---

## Routing continued...

**~ How's A1 going?**

---

## Maze Routing: Route of Length P

**How many cells have label:**

[Grid diagram showing maze routing expansion from center point labeled 0]

```
        3
    3   2   3
3   2   1   2   3
3   2   1   0   1   2   3
3   2   1   2   3
    3   2   3
        3
```

**Counting cells by label:**
- **0:** 1 cell
- **1:** 4 cells
- **2:** 8 cells  
- **3:** 12 cells
- ...
- **P:** 4P cells

**Total # cells visited for path length P:**

Sum from i=0 to P of 4i = 4·P(P+1)/2 = **2P(P+1)**

Or approximately: **O(P²)**

---

## Runtime Analysis

So, for path length of P  
runtime of maze routing is  
**O(P²)**

→

---

## More Complex Routing

### 1) Has to deal with fanout

[Diagram showing source pin with fanout to 2 loads]

```
           load 1
          /
         /
Source  /        fanout = 2
pin     \
         \
          \
           load 2
```

**One way:**
- First route one load
- Then, for 2nd load, we can initialize expansion list to be existing route for first load

---

## 2) Good Routers Optimize for Speed

[Circuit diagram showing logic gates with critical path highlighted in cyan]

```
D ⟶ [D] ⟶ [D] ⟶ D ⟶ D  ← crit path (cyan highlight)

D ⟶ [D] ⟋
            
D ⟶ [D] ⟶ [D] ⟶ D

D ⟶ [D] ⟍
```

**timing-driven router will optimize**

**delay of connections on critical path**

---

## Interconnect Delay Modeling

### FPGA Distributed RC

**Wire ~**

[Distributed RC network diagram showing resistance segments and capacitors to ground]

```
⟶⟶⟶R⟶⟶⟶R⟶⟶⟶R⟶⟶⟶R⟶⟶⟶...
      ⊥      ⊥      ⊥      ⊥
      C      C      C      C
```

### Switches

**off state:**
```
        Cdiff
⟶⟶⟶⟶⟶  ⊥  ⟶⟶⟶⟶⟶
```

**on state:**
```
        Ron
⟶⟶⟶⟶ ⟶⟶⟶⟶ ⟶⟶⟶⟶
    Cdiff  Cdiff
```

---

## Buffered Switch

[SRAM-controlled buffer switch diagram]

### Early FPGAs - SRAM cell 0/1:
```
        [SRAM]
wire ⟶⟶⟶ ⊥ ⟶⟶⟶ wire
```

### Now: FPGAs have buffered routing switches

**off state:**
```
    Const
wire  ⊥   wire
```

**on state:**
```
         Ron
wire ⟶ [>] ⟶⟶⟶ wire
       ⊥   Cdiff
    Const  Stiff
```

**(S) intrinsic constant buffer delay**

---

## What about in ASICs?

**metal wire**

[3D diagram showing multiple metal layers with coupling capacitance between wires]

```
      ┌─────────┐
      │█████████│ ← metal layer
      └─────────┘
        coupling cap
      ┌─────────┐
      │█████████│ ← metal layer
      └─────────┘
```

**metal wires have resistance & capacitance (to neighbouring wires in same layer or adjacent layers of metal)**

---

## More Complex Routing Architecture

**Modern FPGAs have wire segments of different length**

[Diagram showing 4 logic blocks with wire segments spanning different distances]

```
□ ─ □ ─ □ ─ □  ← len-1 seg
─   ─   ─   ─
─────   ─────  ← len-2 seg
```

**Router decides which seg. types to use when routing a connection**

---

## PathFinder

Now, routing is a **very complicated** problem...

**PathFinder does a good job of handling these complexities**

⟹ U. Washington mid-90s  
⟹ basis of Xilinx/Altera commercial routers

---

## ACM FPGA '95

[Reference to PathFinder paper]

**PathFinder: A Negotiation-Based Performance-Driven Router for FPGAs**

Larry McMurchie and Carl Ebeling  
Dept. of Computer Science and Engineering  
University of Washington, Seattle, WA

**Abstract highlights:**
- Routing FPGAs is challenging due to scarcity of routing resources
- PathFinder uses iterative algorithm
- Converges to solution where all signals are routed
- Achieves close to optimal performance
- Signals negotiate for resources
- Delay minimized by allowing critical signals greater say in negotiation

**ACM FPGA '95**

---

## Also Uses PathFinder for CGRA Routing Step

[Reference to CLUMAP paper]

**CLUMAP: Clustered Mapper for CGRAs with Predication**

Omar Ragheb and Jason H. Anderson  
Dept. of Electrical and Computer Engineering, University of Toronto

**Also uses PathFinder for CGRA routing step**

---

## PathFinder Routing Resource Graph (RRG)

**PathFinder represents FPGA routing architecture as a graph G(V, E)**

**called Routing Resource Graph (RRG)**

[Two-part diagram showing physical FPGA structure on left and corresponding graph on right]

**Physical structure (left):**
```
     [□]  ← logic block
  w1 ──┼──
  w2 ──┼──
  w3 ──┼──
      │
     w4
```

**Graph G(V,E) (right):**
```
        P (pin)
       /│\
      / | \
    w1 w2 w3
    │  │  │
   w4 w4 w5 w6
```

**Conductors:** prog. switches  
**(pins, wires)**

---

## Route Connection ⟺ Find Paths Through Graph

**equivalent to**

**Route connection ⟺ graph → graph search**

**With each node n in RRG PathFinder associates:** ⟹ conductor

① **Cn:** congestion cost of n

② **dn:** delay cost of n

---

## We Discuss in Two Parts:

① How it deals with congestion

② How to make it timing-driven  
(make it deal with delay)

---

## Negotiated Congestion Approach

**What is a main weakness of last week's maze routing?**

- depends on the order connections are routed
- earlier connections **"block"** later connections

---

## Nair's Iterative Approach (1987)

**~ Ravi Nair, IBM research, IEEE TCAD**

### CAD this cite:

### Algorithm Structure:

**1) Route all nets**
```
- net 1 // sees "empty" chip
- net 2 // sees net 1
- net 3 // sees nets 1, 2
  :
  :
- net n // sees nets 1→n-1
```

**2) Route all nets**
```
- ripup and reroute net 1 // sees nets 2→n
- "  "   "    "    "  2 // sees net 1, 3→n
  :
  :
```

---

## Continue Iterations

Continue these iterations of  
ripup & reroute all nets

→ Makes it much less order dependent

---

## Nair's Approach Allowed "Illegal" States

[Diagram showing ASIC chip layout]

**ASIC chip divided into bins**

```
┌─────┬─────┬─────┬─────┐
│     │     │  ╱  │     │  we know
├─────┼─────┼─────┼─────┤  how many
│     │  ╱  │     │     │  wires can
├─────┼─────┼─────┼─────┤  cross bin edge
│     │     │     │     │  (based on
├─────┼─────┼─────┼─────┤  wire pitch
│     │     │     │     │  rules)
└─────┴─────┴─────┴─────┘
```

→ penalize over congested bin edges during routing.

---

## PathFinder: Cost-Driven Maze Routing

**PathFinder does cost-driven maze routing ⇒ expansion is a priority queue**

[Diagram showing FPGA routing grid with cost annotations]

```
[□]  │  │  [□]        high
═══  │  │  ═══    ← cost nodes!
 │  -10 -10 │         ⟶
[□]  │  │  [□]
═══  │2 │  ═══
═══  │  │  ═══
[□]  │  │  [□]
```

Green route avoids high-cost nodes (marked -10)  
and takes lower cost path (marked 2)

---

## PathFinder Allows Shorts in Intermediate States

**PathFinder allows shorts in intermediate states ⇒ removes shorts thru costing, rip-up and re-route.**

[Diagram showing two routes intersecting]

```
      [□]  │  │  [□]
       │   │  │   │
      ═╪═══╪══╪═══╪═  green route
       │   │  │   │
      [□]  │  │  [□]
       │   │  │   │
      ═╪═══╪══╪═══╪═  
       │  ╱│  │╲  │
      [□] ││  ││ [□]
       │  ││  ││  │
       │  ╱   ╲   │
      ═╪══════════╪═  purple route
       │          │
          ⭕ ← short! (electrically wrong.)
```

---

## PathFinder Cn Definition

**PathFinder Cn definition** ● **change as routing proceeds**

**Cn = (bn + hn) × pn**

**bn:** base cost of using n **(never changes)**  
e.g. could be based on delay, or length, or ...

**pn:** present congestion on n  
(tied to shorts on n)

**hn:** history of congestion on n.

---

## Pn Definition

**Pn = 1** on first Nair iteration

**Pn = (1 + # of shorts on n) × Pfac**  
on subsequent Nair iterations

**Pfac** is a scalar weight that is bumped up (e.g. by 2x) each Nair iteration

**why?** to make it more costly to create shorts.

---

## Example of Pn Utility

**S₁, S₂, S₃:** sources  
**D₁, D₂, D₃:** destinations

**● Path cost**

[Complete routing graph with all connections and costs]

```
    S₁         S₂         S₃
     ○         ○          ○
    2│        │ │        3│
     │     3  │1│  1     │
    ╱│╲      ╱ │ ╲      ╱│╲
   ╱ │ ╲   ╱   │   ╲  ╱  │4╲
  │  │  │ │    │    │ │   │  │
  A  │  B │    │    │ B   │  C
  ○  3  ○ 1    │    │ ○   2  ○
  │╲   ╱│      │    │ │╲   ╱│
  │ ╲ ╱ │      │    │ │ ╲ ╱ │
 2│  │  │1     │    │ │  │  │3
  │  │  │      │    │ │  │  │
  ○  ○  ○      ○    ○ ○  ○  ○
  D₁ D₂ D₃     D₁   D₂D₁ D₂ D₃
```

**Want to connect:**
- S₁ → D₁
- S₂ → D₂  } 3 connections
- S₃ → D₃  } to route

---

## Behaviour

**- Initially** 
```
S₁ → D₁ }
S₂ → D₂ } all shorted on B
S₃ → D₃ }
```

**- gradually increase Pfac**

**- eventually** S₁ → D₂ is routed  
thru A (Path thru A becomes a cheaper alternative)

**- eventually** S₃ → D₃ is  
routed thru C

---

## Success!

Then, all shorts are eliminated

⇒ PathFinder terminates

⇒ **success!**

**Steve Wilton (UBC)** had recent **FCCM** paper that used **ML** to predict if routing is possible partway thru PathFinder

---

## hn: History of Congestion

**Pn** deals with the present congestion (shorts) on node n

**hn:** history of congestion on n

**hnⁱ:** hn in iteration i

**hn⁰:** 1

**hnⁱ:** hn^(i-1) + (# of shorts on n) × hfac

*scalar weight*

---

## hn Prevents Oscillations

**hn** is a monotonically increasing penalty for nodes that had shorts in past iterations

⟹ **Motivation is to prevent oscillations in the short removal process**

---