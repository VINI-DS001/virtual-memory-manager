# Virtual Memory Manager (VMM) Simulator

[![Language: C99](https://img.shields.io/badge/Language-C99-00599C?style=flat&logo=c)](https://en.wikipedia.org/wiki/C99)
[![Build System: CMake](https://img.shields.io/badge/Build-CMake-064F8C?style=flat&logo=cmake)](https://cmake.org/)
[![Python: 3.8+](https://img.shields.io/badge/Python-3.8+-3776AB?style=flat&logo=python)](https://www.python.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

A modular, high-performance Virtual Memory Manager (VMM) simulator written in C. It models hardware-level address translation (MMU), a fully associative Translation Lookaside Buffer (TLB), single-level page tables, simulated secondary Swap storage, and dynamic page replacement algorithms.

Includes an automated Python benchmarking suite capable of processing over **1,000,000 operations/sec** and generating publication-grade performance charts.

---

## Key Features

* **Hardware Subsystem Modeling:**
  * **Memory Management Unit (MMU):** Zero-overhead bitwise address translation.
  * **Translation Lookaside Buffer (TLB):** Configurable fully associative cache with LRU eviction.
  * **Page Table Engine:** Supports valid, dirty, and referenced bit tracking.
  * **Physical RAM & Swap Storage:** Fixed-frame physical RAM paired with simulated secondary disk I/O.
* **Configurable Replacement Policies:**
  * `LRU` (Least Recently Used via timestamps)
  * `FIFO` (First-In, First-Out via circular queue)
  * `LFU` (Least Frequently Used via access counters)
  * `CLOCK` (Second-Chance algorithm using reference bits)
  * `RANDOM` (Pseudo-random selection baseline)
* **Synthetic Workload Generator:** Supports Sequential, Random, Temporal Locality, Spatial Locality, and Thrashing access patterns.
* **Automated Python Benchmarking:** Full suite for bulk simulation, pandas-driven metrics compilation, and matplotlib performance chart rendering.

---

## Directory Structure

```text
.
├── CMakeLists.txt          # Root CMake build configuration
├── Doxyfile                # Doxygen documentation configuration
├── include/                # Public header files (API definitions)
│   └── vmm/
│       ├── config.h
│       ├── mmu.h
│       ├── replacement.h
│       ├── tlb.h
│       └── ...
├── src/                    # C implementation files & CLI entrypoint
│   ├── CMakeLists.txt
│   ├── main_benchmark.c
│   ├── mmu.c
│   └── ...
├── tests/                  # Unit tests suite (CTest / Harness)
│   ├── CMakeLists.txt
│   └── test_*.c
├── scripts/                # Python automation & charting scripts
│   ├── plot_results.py
│   └── run_benchmarks.py
├── docs/                   # Visual charts & technical documentation
│   ├── ARCHITECTURE.md
│   ├── BENCHMARK_REPORT.md
│   ├── TECHNICAL_MANUAL.md
│   └── *.png
└── results/                # Raw benchmark CSV output (git-ignored)
```

---

## Build & Installation

### Prerequisites

* **C Compiler:** GCC (C99 standard or higher) or Clang
* **Build System:** CMake 3.15+
* **Scripting Environment:** Python 3.8+ with `pandas` and `matplotlib`

### Building from Source

1. Clone the repository:

   ```bash
   git clone https://github.com/VINI-DS001/virtual-memory-manager.git
   cd virtual-memory-manager
   ```

2. Configure and build using CMake:

   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build
   ```

Upon successful compilation, the executable binary `vmm_benchmark_cli` (or `vmm_benchmark_cli.exe` on Windows) will be generated inside the `build/` (or `build/src/`) directory.

## Command Line Interface (CLI) Usage

The `vmm_benchmark_cli` utility allows running standalone simulation instances with full control over memory parameters and console verbosity.

### CLI Options

| **Flag**      | **Description**                  | **Default Value** | **Allowed Options**                        |
| ------------- | -------------------------------- | ----------------- | ------------------------------------------ |
| `--policy`    | Page replacement algorithm       | `lru`             | `fifo`, `lru`, `lfu`, `clock`, `random`    |
| `--workload`  | Memory access pattern            | `temp`            | `seq`, `rand`, `temp`, `spatial`, `thrash` |
| `--accesses`  | Total memory access requests     | `100000`          | Any positive integer                       |
| `--frames`    | Physical RAM frame count         | `8`               | Any positive integer (e.g., 4, 8, 16, 32)  |
| `--tlb-size`  | TLB entry capacity               | `16`              | Any positive integer                       |
| `--log-level` | Console logging verbosity        | `error`           | `trace`, `debug`, `info`, `warn`, `error`  |
| `--output`    | Destination CSV file for metrics | *None*            | Path to CSV file                           |

### Example Commands

- **Run a high-performance simulation using LRU on Temporal Locality:**

  ```bash
  ./build/src/vmm_benchmark_cli --policy lru --workload temp --accesses 100000 --frames 16 --log-level error
  ```

- **Execute a single run and append metrics to CSV:**

  ```bash
  ./build/src/vmm_benchmark_cli --policy clock --workload spatial --accesses 50000 --frames 8 --output results/test_run.csv
  ```

## Automation & Benchmarking Scripts

The repository includes Python scripts to automate bulk benchmark executions and performance charting.

### 1. Install Dependencies

```bash
pip install pandas matplotlib
```

### 2. Run the Expanded Benchmark Suite

Executes the matrix of 300 test combinations across policies, workloads, frame counts, and access scales (10k, 100k, 1M):

```bash
python scripts/run_benchmarks.py
```

*Output data is automatically generated at* `results/benchmark_results.csv`.

### 3. Generate Performance Charts

Parses `results/benchmark_results.csv` and renders high-resolution plots into the `docs/` directory:

```bash
python scripts/plot_results.py
```

---

## Performance Benchmarks & Visualizations

The charts below summarize the core empirical findings from the profiling suite (generated via `scripts/plot_results.py`).

### 1. Page Fault Rate vs. Physical Memory Capacity

Increasing physical RAM frames mitigates thrashing. The `CLOCK` policy provides an optimal trade-off, matching `LRU` performance without dynamic list overhead.

![Page Fault Rate vs RAM Capacity](docs/chart_page_faults_vs_ram.png)

### 2. TLB Hit Rate Efficiency by Access Pattern

TLB hit rates depend heavily on workload spatial/temporal locality rather than replacement algorithms.

![TLB Hit Rate Efficiency](docs/chart_tlb_hit_rate.png)

### 3. Secondary Storage Swap I/O Traffic

Under extreme memory pressure (4 physical frames), dirty page eviction forces a 1:1 ratio between Swap Reads and Swap Writes.

![Swap I/O Operations](docs/chart_swap_io.png)

---

## Policy Performance Comparison

| Policy     | Time Complexity                           | Space Overhead                   | Hardware Support Required      | Best Use Case / Scenario                                     |
| :--------- | :---------------------------------------- | :------------------------------- | :----------------------------- | :----------------------------------------------------------- |
| **LRU**    | $\mathcal{O}(1)$ or $\mathcal{O}(N)$      | High (Timestamps / Lists)        | Yes (Access Clock Register)    | General purpose with high temporal locality                  |
| **FIFO**   | $\mathcal{O}(1)$                          | Low (Queue Pointer)              | None                           | Simple embedded systems without hardware reference bits      |
| **LFU**    | $\mathcal{O}(\log N)$ or $\mathcal{O}(N)$ | High (Frequency Counters)        | None                           | Workloads with stable, persistent long-term frequency        |
| **CLOCK**  | $\mathcal{O}(1)$ amortized                | Very Low (1 reference bit/frame) | Yes (1-bit hardware reference) | **Recommended:** Production OS kernels (near-LRU efficiency) |
| **RANDOM** | $\mathcal{O}(1)$                          | Zero                             | None                           | Benchmark baseline or memory-constrained microcontrollers    |

---

## Documentation Index

For in-depth architectural specifications, API details, and theoretical analysis, refer to the technical guides in the `docs/` directory:

* 🏗️ **[Architecture Overview](docs/ARCHITECTURE.md):** Hardware subsystem component diagrams and page fault decision flowcharts.
* 📊 **[Benchmark Report](docs/BENCHMARK_REPORT.md):** Detailed analysis of empirical results across all 300 test combinations.
* 📖 **[Technical Manual & API Guide](docs/TECHNICAL_MANUAL.md):** Bitwise translation mechanics, data structure definitions, and C API specifications.
* 📑 **Doxygen API Documentation:** Generate interactive HTML API documentation locally:

  ```bash
  doxygen Doxyfile
  ```

Open `docs/doxygen/html/index.html` in your browser.

---

## License

This project is open-source and available under the [MIT License](LICENSE).

---
