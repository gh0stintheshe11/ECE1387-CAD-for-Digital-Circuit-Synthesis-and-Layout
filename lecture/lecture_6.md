# Lecture 6 - Analytical Placement (cont.) & Simulated Annealing

**Course:** ECE 1387  
**Date:** Oct 17, 2025  
**Topics:**
- Analytical placement
- Simulated annealing-based placement

---

## Page 2: Finding Paths for Overfilled Bins

**Finding $P(b_i)$ - Set of paths for overfilled bin $b_i$**

Use BFS (breadth-first search)

```
demand = 0
mark all bins unvisited
visited(bi) = true
insert bi into an empty path P
add path p to FIFO Q, Q
```

---

## Page 3: BFS Algorithm (continued)

```
repeat
    pop p from Q
    tailbin = current end bin of P
    for neighbour bins bk of tailbin
        if (visited(bk))
            continue
        cost = compute cost(tailbin, bk)
        if cost < ∞
            create a copy of p, Pcopy
            cost = cost(Pcopy) + cost
```

---

## Page 4: BFS Algorithm (continued 2)

```
            insert bk into Pcopy // new tail
            visited(bk) = true
            if (bk is empty) then
                insert Pcopy in P(bi)
                demand++;
            else
                add Pcopy to Q
            end if
    end for
until Q is empty or demand ≥ supply(bi)
```

---

## Page 5: Cost Function Definition

**cost $= \psi$ (tailbin, $b_k$)**

- find a cell in tailbin that can move to $b_k$ s.t. quadratic distance from the cell's solved placement and new position in $b_k$ is minimized and less than $\psi$

- If no such cell exists, cost = $\infty$

- Otherwise cost = quadratic displacement of cell from orig. solved placement.

---

## Page 6: Moving Cells Along a Path

**Moving cells along a path $P_k$**  
(from tail bin towards start bin)

```
start bin → [00]      [00]      [6]
              ↓         ↓        ↓
            [00]       [0]      [00]
              ↓         ↓        ↓
tail bin →  [  ]       [●]      [●]
```

- check costs again to not exceed $\psi$ (because of overlapping paths)

---

## Page 7: After Spreading

After spreading, we should have a placement with no overused bins! → great

If we solve AP system again, get same sol'n back! → bad

→ Want to modify AP system so cells gradually move towards spread locations

---

## Page 8: Pseudo Nets

```
           ● solved location
          /
         / (Cell i)
        ●
         \
          \
           ● i's spread location
            ↓
        w_pseudo
```

- Create a fake (pseudo) net between $i$ and it's spread location

- Weight of pseudo connection is increased with each AP iteration

---

## Page 9: Anchor Concept

In essence, we create an "anchor" at spread position with a connection to the anchor.

---

## Page 10: Mathematical Impact

**How to bring cells towards their spread position?**

Ans: Pseudo (fake) nets + pseudo weights

**which part of math does it affect?**
- right hand side, $b_x$, $b_y$
- on-diag elements of $Q_x$, $Q_y$

---

## Page 11: Convergence

```
      HPWL ↑              spread WL
           │      ●
           │     ●●
           │    ●  ●─────────────────┐
           │   ●                     │ terminate
           │  ●      ↙ spread WL    │ when solved
           │ ●                       │ and spread
           │●        ↗ solved WL     │ WL converge
           │●      ●                 │
           │●    ●                   │
           │●  ●                     │
           │●●                       │
           └─────────────────────────┴────→
             AP iteration
             (solving + spread)
```

---

## Page 12: Overall AP Flow

```
Overall AP Flow
                    
Circuit
   ↓
┌────────────────┐
│ Solve seq w/   │
│ clique model   │
└───────┬────────┘
        ↓
┌────────────────┐     NO
│  B2B net       │←────┐
│  model         │     │
└───────┬────────┘     │
        ↓              │
    ┌───────┐          │
    │ Solve │          │
    └───┬───┘          │
        ↓              │
    ┌───────┐          │
    │ done? │──────────┘
    └───┬───┘
        │ NO
        ↓
    ┌────────────────┐        ┌──────────┐
    │  Spreading     │───────→│ Converge │
    └───────┬────────┘        │  check   │
            │                 └────┬─────┘
            │                      │
            ↓                      │ YES
    ┌────────────────┐             ↓
    │ Pseudo nets    │
    │  anchors       │
    └───────┬────────┘
            ↓
        ┌───────┐
        │ Solve │
        └───┬───┘
            ↓
        ┌───────┐              ┌─────┐
        │ done? │─────YES─────→│     │
        └───┬───┘              └─────┘
            │ NO
            └──────────────────┘
```

---

## Page 13: AP "Fitting" / Fine Legalization

**AP "fitting" fine legalization**

```
x=1.2    ┌─────────────────┐
y=2.4    │  │  │ ● │  │  │
         ├──┼──┼───┼──┼──┤
         │  │● │   │● │  │
         ├──┼──┼───┼──┼──┤
         │  │  │   │● │  │
         ├──┼──┼───┼──┼──┤
         │  │● │   │  │  │
         └─────────────────┘
```

**One way:** Find legal grid point closest to solved location.

→ Snap cell to that point if its vacant.

---

## Page 14: If Closest Slot Not Vacant

If closest slot is not vacant, do BFS to find closest vacant slot.

---

## Page 15: Simulated Annealing-Based Placement

# Simulated Annealing-Based Placement

- SA is a very general optimization strategy
- Can be applied to almost any problem
- IBM Research 1980s
- Based on an analogy: "annealing" of metals

---

## Page 16: Annealing Analogy

- to make strong steel, melt and cool slowly
- atoms coalesce into a regular crystal (hard to break)

---

## Page 17: Temperature Notion

SA has a "temperature" notion

T : algorithm parameter

High T: e.g. in placement cells move all over the chip

Low T: restricted cell movements

---

## Page 18: Hill Climbing Concept

**Important SA Concept: "Hill Climbing"**

```
┌──┬──┬──┬──┬──┬──┐
│1 │2 │3 │4 │5 │6 │  WL = 10
└──┴──┴──┴──┴──┴──┘

Swap(2,4)
     ↓
┌──┬──┬──┬──┬──┬──┐
│1 │4 │3 │2 │5 │6 │  WL = 12 (worse)
└──┴──┴──┴──┴──┴──┘

now... Swap(5,3)
          ↓
┌──┬──┬──┬──┬──┬──┐
│1 │4 │5 │2 │3 │6 │  WL = 8
└──┴──┴──┴──┴──┴──┘
```

---

## Page 19: Hill Climbing Explanation

Hill climbing allows a "move" (a perturbation) where cost gets worse ... in hope of finding a better cost later.

As opposed to a greedy algorithm which always demands cost be reduced.

---

## Page 20: General Framework of SA

**General Framework of SA**

① Need an initial constructed state, $j_0$

② Need a cost function, cost(state)  
   e.g. HPWL

③ Need an initial (high) temperature $T_0$

---

## Page 21: SA Pseudocode (Part 1)

**SA($j_0$, $T_0$) in generic form**

```
T = T₀
X = j₀ // X = current state
while ("stopping criterion not met")
    while ("inner loop criterion not met")
        j = generate(X) // j new state
        calculate ΔC =  // cost change
            cost(j) - cost(X)
```

---

## Page 22: SA Pseudocode (Part 2)

```
        if (Accept(ΔC, T))
            X = j // accept "move"
        } // inner loop
    T = update(T)
} // outer loop
}
```