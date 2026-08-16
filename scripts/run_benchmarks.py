#!/usr/bin/env python3
"""
@file run_benchmarks.py
@brief Automated benchmark execution script for Virtual Memory Manager.
"""

import os
import sys
import subprocess
import time
from pathlib import Path

# Paths configuration
PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESULTS_DIR = PROJECT_ROOT / "results"
CSV_OUTPUT = RESULTS_DIR / "benchmark_results.csv"

# Matrix of test configurations
POLICIES = ["fifo", "lru", "lfu", "clock", "random"]
WORKLOADS = ["seq", "rand", "temp", "spatial", "thrash"]
RAM_FRAMES = [4, 8, 16, 32]
ACCESS_COUNTS = [10000, 100000, 1000000]
TLB_SIZE_DEFAULT = 16


def find_executable():
    """Finds the vmm_benchmark_cli executable across common build directories."""
    candidates = [
        PROJECT_ROOT / "build" / "vmm_benchmark_cli",
        PROJECT_ROOT / "build" / "vmm_benchmark_cli.exe",
        PROJECT_ROOT / "build" / "src" / "vmm_benchmark_cli",
        PROJECT_ROOT / "build" / "src" / "vmm_benchmark_cli.exe",
        PROJECT_ROOT / "build" / "Debug" / "vmm_benchmark_cli.exe",
        PROJECT_ROOT / "build" / "Release" / "vmm_benchmark_cli.exe",
        PROJECT_ROOT / "build" / "src" / "Debug" / "vmm_benchmark_cli.exe",
        PROJECT_ROOT / "build" / "src" / "Release" / "vmm_benchmark_cli.exe",
    ]

    for candidate in candidates:
        if candidate.is_file():
            return candidate

    return None


def prepare_environment():
    """Ensures results directory exists and resets CSV header if starting fresh."""
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    if CSV_OUTPUT.exists():
        CSV_OUTPUT.unlink()  # Clear previous results file


def run_benchmark_matrix(exe_path):
    """Iterates over the test matrix and executes vmm_benchmark_cli for each combination."""
    total_runs = len(POLICIES) * len(WORKLOADS) * len(RAM_FRAMES) * len(ACCESS_COUNTS)
    current_run = 0

    print(f"==================================================")
    print(f" Starting Expanded VMM Benchmark Suite ({total_runs} combinations)")
    print(f" Access variations : {ACCESS_COUNTS}")
    print(f" Output File       : {CSV_OUTPUT}")
    print(f"==================================================\n")

    start_time = time.time()

    for accesses in ACCESS_COUNTS:
        for policy in POLICIES:
            for workload in WORKLOADS:
                for frames in RAM_FRAMES:
                    current_run += 1

                    cmd = [
                        str(exe_path),
                        "--policy", policy,
                        "--workload", workload,
                        "--accesses", str(accesses),
                        "--frames", str(frames),
                        "--tlb-size", str(TLB_SIZE_DEFAULT),
                        "--log-level", "error",
                        "--output", str(CSV_OUTPUT)
                    ]

                    print(f"[{current_run:03d}/{total_runs:03d}] Acc={accesses:<7} | Policy={policy.upper():<6} | Workload={workload:<7} | Frames={frames:<2} ... ", end="", flush=True)

                    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

                    if res.returncode == 0:
                        print("OK")
                    else:
                        print(f"FAILED (Exit Code {res.returncode})")
                        print(f"Error output:\n{res.stderr}")

    elapsed = time.time() - start_time
    print(f"\n==================================================")
    print(f" Expanded Suite Completed Successfully in {elapsed:.2f}s!")
    print(f" Results written to: {CSV_OUTPUT}")
    print(f"==================================================")


def main():
    exe_path = find_executable()
    if not exe_path:
        print("ERROR: Could not locate 'vmm_benchmark_cli' executable.")
        print("Please compile the project first using 'cmake --build build'.")
        sys.exit(1)

    prepare_environment()
    run_benchmark_matrix(exe_path)


if __name__ == "__main__":
    main()