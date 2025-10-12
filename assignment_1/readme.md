# FPGA Maze Router - ECE1387 Assignment 1

## Compilation

```bash
make
```

This will compile the router executable.

To clean build artifacts:
```bash
make clean
```

## Usage

```bash
./router -f <circuit_file> [-a <architecture>] [-w <W>] [-i]
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