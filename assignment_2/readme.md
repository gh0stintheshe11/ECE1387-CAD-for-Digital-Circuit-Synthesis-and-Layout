# FPGA Analytical Placer - ECE1387 Assignment 2

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
./placer -f <circuit_file> [-s <mode>] [--psi-init <val>] [--psi-incr <val>] [--anchor-weight <val>] [-g]
```

### Arguments

- `-f <file>` **(required)**: Input circuit file (e.g., `cct2.txt`)
- `-s <mode>` (optional): Enable spreading with placement mode
  - `ho`: Homogeneous (any cell type in any bin)
  - `he`: Heterogeneous (type-specific columns: type 0 in col%4∈{0,1,2}, type 1 in col%4=3)
- `--psi-init <val>` (optional): Initial ψ value (default: 2.0)
- `--psi-incr <val>` (optional): ψ increment per iteration (default: 1.0)
- `--anchor-weight <val>` (optional): Enable anchoring with given weight
  - Weak anchor: 0.1
  - Strong anchor: 100.0
  - Requires `-s` to be enabled
- `-g` (optional): Enable GUI visualization

## Examples

```bash
# Part 1: Analytical placement only (no spreading)
./placer -f cct1.txt -g

# Part 2i: Spreading with homogeneous mode
./placer -f cct2.txt -s ho --psi-init 0.5 --psi-incr 0.25 -g

# Part 2i: Spreading with heterogeneous mode
./placer -f cct3.txt -s he --psi-init 1.0 --psi-incr 0.25 -g

# Part 2ii: Weak anchoring
./placer -f cct2.txt -s ho --psi-init 0.5 --psi-incr 0.25 --anchor-weight 0.1 -g

# Part 2ii: Strong anchoring
./placer -f cct2.txt -s ho --psi-init 0.5 --psi-incr 0.25 --anchor-weight 100.0 -g
```

## Structure

```
assignment_2/
├── Makefile
├── main.cpp
├── data_structures.h
├── input_parser.h
├── input_parser.cpp
├── weight_matrix.h
├── weight_matrix.cpp
├── umfpack_solver.h
├── umfpack_solver.cpp
├── hpwl_calculator.h
├── hpwl_calculator.cpp
├── spreader.h
├── spreader.cpp
├── graphics.h
└── graphics.cpp
```


## Output

The placer reports:
- Circuit statistics (blocks, nets, connectivity)
- Placement system information (matrix size, sparsity)
- HPWL before and after each step
- Spreading iteration progress (if enabled)
- Total cell displacement (if spreading enabled)
- Anchoring results (if enabled)

If GUI is enabled (`-g`), a graphical window displays the placement with color-coded blocks and nets.