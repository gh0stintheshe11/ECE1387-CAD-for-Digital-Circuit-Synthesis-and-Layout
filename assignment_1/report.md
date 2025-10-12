I spend so much time on the god forsaken parallelization that I missed the deadline.
However I had so much fun with it that I THINK IT'S WORTH IT.
FYI, I think I found something cool: the parallel router actually USES LESS TRACKS THAN THE SERIAL ONE SOMETIMES.
so I did some digging and wrote about it in Q5.

## Q1. Test files plots and segments used

### cct1
- **cct1_distributed**
![cct1_distributed](./cct1_distributed.png)
Used segments = 33
- **cct1_topbottom**
![cct1_topbottom](./cct1_topbottom.png)
Used segments = 31

### cct2
- **cct2_distributed**
![cct2_distributed](./cct2_distributed.png)
Used segments = 132
- **cct2_topbottom**
![cct2_topbottom](./cct2_topbottom.png)
Used segments = 130

### cct3
- **cct3_distributed**
![cct3_distributed](./cct3_distributed.png)
Used segments = 703
- **cct3_topbottom**
![cct3_topbottom](./cct3_topbottom.png)
Used segments = 713

### cct4
- **cct4_distributed**
![cct4_distributed](./cct4_distributed.png)
Used segments = 2368
- **cct4_topbottom**
![cct4_topbottom](./cct4_topbottom.png)
Used segments = 2375

### Summary

| Test Case | Distributed Segments Used | Top-Bottom Segments Used |
| --------- | ------------------------- | ------------------------ |
| cct1      | 33                        | 31                       |
| cct2      | 132                       | 130                      |
| cct3      | 703                       | 713                      |
| cct4      | 2368                      | 2375                     |

## Q2. Minimum tracks

### cct1 minimum tracks

- **cct1_distributed_min**
![cct1_distributed_min](./cct1_distributed_min_3.png)
W = 3
Used segments = 33

- **cct1_topbottom_min**
![cct1_topbottom_min](./cct1_topbottom_min_3.png)
W = 3
Used segments = 31

### cct2 minimum tracks

- **cct2_distributed_min**
![cct2_distributed_min](./cct2_distributed_min_4.png)
W = 4
Used segments = 139

- **cct2_topbottom_min**
![cct2_topbottom_min](./cct2_topbottom_min_4.png)
W = 4
Used segments = 144

### cct3 minimum tracks

- **cct3_distributed_min**
![cct3_distributed_min](./cct3_distributed_min_5.png)
W = 5
Used segments = 712

- **cct3_topbottom_min**
![cct3_topbottom_min](./cct3_topbottom_min_6.png)
W = 6
Used segments = 717

### cct4 minimum tracks 
- **cct4_distributed_min**
![cct4_distributed_min](./cct4_distributed_min_9.png)
W = 9
Used segments = 2371

- **cct4_topbottom_min**
![cct4_topbottom_min](./cct4_topbottom_min_10.png)
W = 10
Used segments = 2377

### Minimum Tracks Summary
| Test Case | Distributed |               | Top-Bottom |               |
| --------- | ----------- | ------------- | ---------- | ------------- |
|           | Min W       | Used Segments | Min W      | Used Segments |
| cct1      | 3           | 33            | 3          | 31            |
| cct2      | 4           | 139           | 4          | 144           |
| cct3      | 5           | 712           | 6          | 717           |
| cct4      | 9           | 2371          | 10         | 2377          |

### Optimization Strategies

1. **Fanout Tree Sharing**: Connections are grouped by source pin, so all sinks driven by the same source are routed consecutively. When routing the second and subsequent sinks, the BFS is seeded from both the source pin and all previously-routed segments for that net. This allows different branches of the fanout tree to share common routing segments, reducing overall segment count.

2. **Shortest Path Routing**: The Lee-Moore BFS algorithm finds minimum-length paths between pins, which minimizes the number of segments per connection. The breadth-first expansion guarantees that the first path found is optimal in Manhattan distance.


## Q4. Impact of Pin Placement on Routability

Pin placement architecture significantly impacts routability as circuit complexity increases. For small circuits (cct1, cct2), both architectures achieve the same minimum W, but for larger circuits, distributed pin placement demonstrates clear advantages: 16.7% fewer tracks for cct3 (W=5 vs W=6) and 10% fewer for cct4 (W=9 vs W=10). This occurs because distributing pins across all four sides reduces channel congestion and provides more routing flexibility, while concentrating pins on two sides creates bottlenecks. Segment usage shows the opposite trend: top/bottom uses fewer segments for small circuits but more for large ones, suggesting distributed placement scales better with complexity. The tradeoff is that distributed pins improve routability and scalability at the cost of slightly longer routes in uncongested designs, while top/bottom is more efficient when resources are abundant but struggles under congestion.

## Q5. Parallelization

### !?!?!? PARALLELIZATION USE LESS TRACKS !?!?!?

| Test Case | Distributed |               | Top-Bottom |               |
| --------- | ----------- | ------------- | ---------- | ------------- |
|           | Min W       | Used Segments | Min W      | Used Segments |
| cct1      | 3           | 30            | 3          | 30            |
| cct2      | 4           | 111           | 4          | 109           |
| cct3      | 5           | 689           | 6          | 703           |
| cct4      | 8 (-1)      | 2244          | 8 (-2)     | 2215          |

For cct4, the parallel router successfully routes with W=8 for both architectures, whereas the serial implementation requires W=9 (distributed) and W=10 (top/bottom). Seems the parallel router finds better solutions, and this behavior is more ovbious on larger circuits. My guess is that the parallel BFS explores the search space more diversely due to thread interleaving, leading to better path selections and less congestion. The serial BFS may get stuck in local minima, especially under tight resource constraints, while the parallel version's concurrent expansions help escape these traps.

![new_cct4_distributed_min](./cct4_distributed_min_8_para4t.png)
W = 8
Used segments = 2244
![new_cct4_topbottom_min](./cct4_topbottom_min_8_para4t.png)
W = 8
Used segments = 2215


### Timing and Segment Usage Summary

| Test Case | Version | Distributed |               |          | Top-Bottom |               |          |
| --------- | ------- | ----------- | ------------- | -------- | ---------- | ------------- | -------- |
|           |         | W           | Used Segments | time(ms) | W          | Used Segments | time(ms) |
| cct1      | norm    | 3           | 33            | 0.654    | 3          | 31            | 0.543    |
| cct1      | para_2t | 3           | 33            | 1.136    | 3          | 30            | 1.114    |
| cct1      | para_2t | 4           | 32            | 1.578    | 4          | 31            | 1.543    |
| cct1      | para_4t | 3           | 33            | 1.367    | 3          | 30            | 1.441    |
| cct1      | para_4t | 4           | 30            | 1.845    | 4          | 30            | 1.785    |
| cct1      | para_8t | 3           | 32            | 2.159    | 3          | 30            | 1.918    |
| cct1      | para_8t | 4           | 30            | 2.367    | 4          | 30            | 2.461    |
| cct2      | norm    | 4           | 139           | 2.835    | 4          | 144           | 3.234    |
| cct2      | para_2t | 4           | 115           | 6.345    | 4          | 111           | 6.207    |
| cct2      | para_2t | 5           | 118           | 7.729    | 5          | 116           | 7.653    |
| cct2      | para_4t | 4           | 122           | 7.025    | 4          | 116           | 6.904    |
| cct2      | para_4t | 5           | 111           | 7.907    | 5          | 109           | 7.662    |
| cct2      | para_8t | 4           | FAILED        | FAILED   | 4          | FAILED        | FAILED   |
| cct2      | para_8t | 5           | 108           | 9.879    | 5          | 110           | 10.005   |
| cct3      | norm    | 5           | 712           | 38.369   | 6          | 717           | 48.61    |
| cct3      | para_2t | 5           | 672           | 60.954   | 6          | 670           | 71.339   |
| cct3      | para_2t | 6           | 693           | 74.184   | 7          | 676           | 81.333   |
| cct3      | para_4t | 5           | FAILED        | FAILED   | 6          | 690           | 68.645   |
| cct3      | para_4t | 6           | 673           | 68.812   | 7          | 666           | 78.587   |
| cct3      | para_8t | 5           | FAILED        | FAILED   | 6          | 677           | 84.99    |
| cct3      | para_8t | 6           | 668           | 83.608   | 7          | 663           | 93.208   |
| cct4      | norm    | 9           | 2371          | 411.043  | 10         | 2377          | 480.201  |
| cct4      | para_2t | 9           | FAILED        | FAILED   | 10         | 2209          | 599.321  |
| cct4      | para_2t | 10          | 2204          | 600.79   | 11         | 2205          | 661.093  |
| cct4      | para_4t | 9           | 2239          | 492.629  | 10         | 2218          | 540.531  |
| cct4      | para_4t | 10          | 2222          | 547.372  | 11         | 2209          | 592.001  |
| cct4      | para_8t | 9           | 2197          | 554.467  | 10         | 2195          | 591.495  |
| cct4      | para_8t | 10          | 2206          | 598.034  | 11         | 2218          | 651.448  |

### Analysis of Parallelization Results

**Performance Characteristics:**
slower execution time in exchange for significantly better solution quality. Across all test cases, the parallel router finds 6-22% fewer routing segments than the serial version, with the largest improvements on medium-sized circuits (cct2 showing 22% reduction). However, this comes at a 1.2-4x runtime cost due to synchronization overhead.

**Thread Scaling:**
Four threads emerged as the optimal configuration, providing the best balance of solution quality and stability. The 8-thread configuration shows diminishing returns, with no significant speedup over 4 threads, suggesting that synchronization contention limits scalability beyond 4 cores for these problem sizes.

**Stability Issues:**
The parallel implementation exhibits intermittent routing failures (5 out of 56 test cases) that do not occur in the serial version. These failures occur exclusively at minimum channel widths where routing resources are tightest, suggesting a race condition in the BFS exploration when multiple threads compete heavily for scarce resources. Thus, there might be a bug in the implementation that needs to be addressed. (My guess is there is a race condition in the index_map lookup or parent tracking.)

## Q6. Software Architecture and Design

### High-Level Program Flow

```mermaid
flowchart TD
    A[Start] --> B[Parse Command Line Arguments]
    B --> C[Read Circuit File]
    C --> D[Initialize Router with n, W, architecture]
    D --> E[Build Routing Graph]
    E --> F[Group Connections by Source Pin]
    F --> G{For Each Net}
    G --> H[Route First Sink via BFS]
    H --> I{More Sinks?}
    I -->|Yes| J[Route Additional Sinks<br/>Using Existing Tree]
    J --> I
    I -->|No| K[Mark Segments Unavailable]
    K --> G
    G -->|All Routed| L{Success?}
    L -->|Yes| M[Display Results]
    L -->|No| N[Report Failure]
    M --> O{GUI Enabled?}
    O -->|Yes| P[Launch EZGL Visualization]
    O -->|No| Q[End]
    N --> Q
    P --> Q
```

### Major Data Structures

#### Router Class
The `Router` struct encapsulates all routing state and algorithms:

```cpp
struct Router {
    int n, W;                              // Grid size and channel width
    Arch arch;                             // Pin placement architecture
    unordered_map<uint64_t, vector<uint64_t>> adj;  // Segment graph
    unordered_set<uint64_t> unavailable;   // Used segments
    unordered_map<uint64_t, int> segment_to_net;    // Net coloring
};
```

Use 64-bit segment IDs with bit-packing to encode segment properties (horizontal/vertical, row/column, span, track number) in a single integer. This provides O(1) lookups while maintaining a compact representation.

#### Segment ID Encoding
Segments are encoded as 64-bit integers:
- Bit 0: Horizontal (1) or Vertical (0)
- Bits 1-16: Row/Column index
- Bits 17-32: Span index
- Bits 33+: Track number (0 to W-1)

Bit-packing eliminates the need for complex multi-field hash functions and enables fast set/map operations.

#### Graph Representation
The routing resource graph uses an adjacency list:
```cpp
unordered_map<uint64_t, vector<uint64_t>> adj;
```

Adjacency list over adjacency matrix reduces memory from O(V²) to O(V+E), given potentially thousands of segments.

### Major Routines

```mermaid
flowchart LR
    A[build_graph] --> B[Creates segments<br/>and connections]
    C[pin_to_adjacent_segments] --> D[Maps pin to<br/>tracks]
    E[bfs_route] --> F[Lee-Moore BFS]
    G[route_one] --> H[Routes one<br/>connection]
    
    H --> C
    H --> E
    H --> I[Marks segments<br/>unavailable]
```

#### 1. `build_graph()`
Constructs the routing resource graph by creating all horizontal and vertical segments. For each intersection, connects each segment to 3 others: straight-through and two perpendicular turns. Direct adjacency list construction rather than a pattern lookup table clearly encodes the planar topology in code.

#### 2. `pin_to_adjacent_segments(x, y, p)`
Maps a logic block pin to its W adjacent routing tracks (Fc=W).

The critical challenge was determining correct channel-to-block mapping:
- Block at (x,y): LEFT channel at x, RIGHT at x+1, BOTTOM at y, TOP at y+1
- Distributed: Pin 1→LEFT, 2→RIGHT, 3→TOP, 4→BOTTOM
- Top/Bottom: Pins 1,2→TOP, Pins 3,4→BOTTOM

This required careful analysis as "TOP of block y" corresponds to channel y+1, not y.

#### 3. `bfs_route(src_seed, tgt_adj)`
Implements multi-source Lee-Moore BFS:

Multi-source initialization enables fanout tree routing. For nets with multiple sinks, subsequent BFS starts from both the source pin AND previously-routed segments, allowing natural tree sharing.

**Algorithm**:
1. Initialize queue with all source segments
2. Expand wave-front, tracking parents
3. Stop when target reached
4. Backtrace to construct path

#### 4. `route_one(source, sink, existing_tree)`
Routes a single source-sink connection.

The `existing_tree` parameter enables fanout optimization. When routing subsequent sinks of a multi-terminal net, the existing tree is included in the BFS seed, automatically creating shared routing trees and reducing segment usage.

#### 5. Visualization

The GUI uses color-coding to associate pins with their routes. Each net gets a unique color (cycling through 8 colors), and both routing segments and endpoint pins use matching colors for easy verification. This visual mapping makes routing correctness immediately apparent and helps debug pin placement issues.