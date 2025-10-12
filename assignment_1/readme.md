# FPGA Maze Router - ECE1387 Assignment 1

## Compilation

### To compile and build the artifact, run:
```bash
make
```

For parallel version:
```bash
make -f Makefile.parallel
```

### To clean build artifacts:
```bash
make clean
```
For parallel version:
```bash
make -f Makefile.parallel clean
```

## Usage

```bash
./router -f <circuit_file> [-a <architecture>] [-w <W>] [-i]
```
For parallel version:
```bash
./router_parallel -f <circuit_file> [-a <architecture>] [-w <W>] [-i] [-t <num_threads>]
```

### Arguments

- `-f <file>` **(required)**: Input circuit file (e.g., `cct1.txt`)
- `-a <architecture>` (optional): Pin placement architecture
  - `distributed` (default): Pins on all four sides
  - `topbottom` or `tb`: Pins on top and bottom only
- `-w <W>` (optional): Override channel width (number of tracks per channel)
  - If not specified, uses W from the input file
- `-i` (optional): Enable GUI visualization
- `-h` or `--help`: Display help message

For parallel version:
- `-t <num_threads>` (optional): Number of threads to use (default: 4)

## Examples

```bash
# Route with default settings (no GUI)
./router -f cct1.txt

# Route with distributed architecture and GUI
./router -f cct1.txt -a distributed -i

# Route with top/bottom architecture
./router -f cct2.txt -a topbottom -i

# Override W to find minimum channel width
./router -f cct3.txt -a distributed -w 5 -i

# Parallel router with 8 threads
./router_parallel -f cct4.txt -a distributed -w 10 -i -t 8
```

## Output

The router reports:
- Architecture used
- Routing success/failure
- Total number of routing segments used
- Total routing time in milliseconds (does not include GUI time, just the routing algorithm)

If GUI is enabled (`-i`), a graphical window displays the routing solution with color-coded nets.

## Requirements

- C++14 or later
- GTK3 and Cairo (for GUI)

For parallel version:
- OpenMP (for parallelization)