# FPGA Branch-and-Bound Partitioner - ECE1387 Assignment 3

## Compile

### To compile and build the artifact, run:
```bash
make
```

### To clean build artifacts:
```bash
make clean
```

## Usage

```bash
./partitioner -f <circuit_file> [-t <num_threads>] [-g]
```

### Arguments

- `-f <file>` **(required)**: Input circuit file (e.g., `cct3.txt`)
- `-t <num_threads>`: Number of threads for parallel execution (default is 4 if flag is not used)
  - Use `-t 1` for sequential execution
  - Use `-t 4`, `-t 8`, `2^n` etc. for parallel execution (I tried up to 128 thread on ECF machine no issue, and signifiacantly reduced the run time, cct4 with 128 threads finishs in 17s, but lowest run time does not give lowest visited node count. Detailed result in report)
- `-g` (optional): Enable GUI visualization of the decision tree
  - Automatically sets `-t 1` (graphics requires sequential mode, a thread safe node recording is complex, I give up)

## Examples

```bash
# Basic sequential run
./partitioner -f cct3.txt

# Parallel execution with 4 threads
./partitioner -f cct3.txt -t 4

# Parallel execution with 32 threads
./partitioner -f cct4.txt -t 32

# Sequential with graphics visualization
./partitioner -f cct3.txt -g

# Explicit sequential (equivalent to default)
./partitioner -f cct3.txt -t 1
```

### Example Output
```
[sunlang@remote assignment_3]$ ./partitioner -f cct4.txt -t 128
Parsing circuit file: cct4.txt
=== Circuit Summary ===
Blocks: 48
Nets: 75
Community pairs: 14
Total pins: 559
Max block fanout: 19
Max net degree: 13

=== Branch and Bound Partitioner ===
Using 128 thread(s)
Blocks sorted by fanout (top 5):
  Block 43 (fanout: 19)
  Block 5 (fanout: 18)
  Block 38 (fanout: 17)
  Block 30 (fanout: 17)
  Block 16 (fanout: 17)
Initial solution cost: 70 (crossing: 69, community: 1)
Starting B&B (block 43 fixed to LEFT)...
  Found better solution: 69 (crossing: 66, community: 3)
  Found better solution: 68 (crossing: 67, community: 1)
  Found better solution: 67 (crossing: 65, community: 2)
  Found better solution: 66 (crossing: 65, community: 1)
  Found better solution: 65 (crossing: 64, community: 1)
  Found better solution: 64 (crossing: 64, community: 0)
  Found better solution: 63 (crossing: 63, community: 0)
  Found better solution: 62 (crossing: 62, community: 0)

=== Final Result ===
Optimal cost: 62
  Crossing count: 62
  Community cost: 0
Nodes visited: 104460956
Runtime: 16960 ms

Left partition (24 blocks): 1 4 5 6 8 13 14 15 16 19 20 21 22 24 25 26 34 36 38 41 42 43 44 46 
Right partition (24 blocks): 2 3 7 9 10 11 12 17 18 23 27 28 29 30 31 32 33 35 37 39 40 45 47 48 
```

## Graphics Visualization

When `-g` is enabled, the decision tree is displayed:
- **Black nodes/edges**: Explored paths
- **Red nodes/edges**: Pruned branches
- **Green nodes/edges**: Solution path