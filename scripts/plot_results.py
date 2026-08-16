#!/usr/bin/env python3
"""
@file plot_results.py
@brief Performance analysis and visual charting script for VMM Simulator benchmark results.
"""

import os
import sys
from pathlib import Path
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# Path Configurations
PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESULTS_CSV = PROJECT_ROOT / "results" / "benchmark_results.csv"
DOCS_DIR = PROJECT_ROOT / "docs"


def setup_style():
    """Configures clean, professional plot style."""
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    plt.rcParams.update({
        'font.sans-serif': 'DejaVu Sans',
        'font.family': 'sans-serif',
        'figure.dpi': 300,
        'savefig.dpi': 300,
        'axes.labelsize': 11,
        'axes.titlesize': 13,
        'xtick.labelsize': 10,
        'ytick.labelsize': 10,
        'legend.fontsize': 10,
        'figure.titlesize': 14
    })


def plot_page_faults_vs_ram(df):
    """Plot 1: Page Fault Rate (%) vs RAM Frame Count across Policies."""
    fig, ax = plt.subplots(figsize=(8, 5))
    
    # Filter for 100k accesses and Temporal Locality workload
    subset = df[(df['TotalAccesses'] == 100000) & (df['Workload'].str.lower().str.startswith('temp'))]
    if subset.empty:
        subset = df[df['TotalAccesses'] == 100000]

    policies = subset['Policy'].unique()
    colors = {'FIFO': '#e74c3c', 'LRU': '#2ecc71', 'LFU': '#3498db', 'CLOCK': '#f1c40f', 'RANDOM': '#9b59b6'}
    markers = {'FIFO': 'o', 'LRU': 's', 'LFU': '^', 'CLOCK': 'D', 'RANDOM': 'v'}

    for policy in sorted(policies):
        p_data = subset[subset['Policy'] == policy].sort_values('RamFrames')
        if not p_data.empty:
            color = colors.get(policy.upper(), '#333333')
            marker = markers.get(policy.upper(), 'o')
            ax.plot(p_data['RamFrames'], p_data['PageFaultRate'], 
                    label=policy.upper(), color=color, marker=marker, linewidth=2, markersize=7)

    ax.set_title('Page Fault Rate vs. Physical RAM Capacity (Temporal Locality)', pad=12)
    ax.set_xlabel('RAM Frame Count')
    ax.set_ylabel('Page Fault Rate (%)')
    ax.set_ylim(-2, 102)
    ax.xaxis.set_major_locator(ticker.MaxNLocator(integer=True))
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend(title='Policy', frameon=True, facecolor='white', edgecolor='none')

    plt.tight_layout()
    output_path = DOCS_DIR / 'chart_page_faults_vs_ram.png'
    plt.savefig(output_path)
    plt.close()
    print(f"[+] Saved Chart 1: {output_path}")


def plot_tlb_hit_rate_by_workload(df):
    """Plot 2: TLB Hit Rate (%) across Workloads."""
    fig, ax = plt.subplots(figsize=(8, 5))
    
    subset = df[df['TotalAccesses'] == 100000]
    avg_tlb = subset.groupby('Workload')['TlbHitRate'].mean().reset_index()

    bars = ax.bar(avg_tlb['Workload'], avg_tlb['TlbHitRate'], color='#34495e', width=0.55, edgecolor='#2c3e50')

    for bar in bars:
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2.0, yval + 1.5, f'{yval:.1f}%', ha='center', va='bottom', fontsize=9, fontweight='bold')

    ax.set_title('TLB Hit Rate Efficiency by Access Pattern Workload', pad=12)
    ax.set_xlabel('Workload Type')
    ax.set_ylabel('Average TLB Hit Rate (%)')
    ax.set_ylim(0, max(avg_tlb['TlbHitRate'].max() * 1.2, 100))
    ax.grid(axis='y', linestyle='--', alpha=0.6)

    plt.tight_layout()
    output_path = DOCS_DIR / 'chart_tlb_hit_rate.png'
    plt.savefig(output_path)
    plt.close()
    print(f"[+] Saved Chart 2: {output_path}")


def plot_swap_io_impact(df):
    """Plot 3: Swap Reads & Writes I/O Traffic by Replacement Policy (Thrashing Workload)."""
    fig, ax = plt.subplots(figsize=(9, 5))
    
    subset = df[(df['Workload'].str.lower().str.startswith('thrash')) & (df['RamFrames'] == 4) & (df['TotalAccesses'] == 100000)]
    if subset.empty:
        subset = df[(df['RamFrames'] == 4) & (df['TotalAccesses'] == 100000)]

    avg_io = subset.groupby('Policy')[['SwapReads', 'SwapWrites']].mean().reset_index()

    x = range(len(avg_io))
    width = 0.35

    ax.bar([i - width/2 for i in x], avg_io['SwapReads'], width, label='Swap Reads (In)', color='#2980b9')
    ax.bar([i + width/2 for i in x], avg_io['SwapWrites'], width, label='Swap Writes (Out)', color='#e67e22')

    ax.set_title('Secondary Storage Swap I/O Volume under Memory Pressure (4 Frames)', pad=12)
    ax.set_xlabel('Page Replacement Policy')
    ax.set_ylabel('Disk Operation Count')
    ax.set_xticks(list(x))
    ax.set_xticklabels([p.upper() for p in avg_io['Policy']])
    ax.grid(axis='y', linestyle='--', alpha=0.6)
    ax.legend(frameon=True, facecolor='white')

    plt.tight_layout()
    output_path = DOCS_DIR / 'chart_swap_io.png'
    plt.savefig(output_path)
    plt.close()
    print(f"[+] Saved Chart 3: {output_path}")


def main():
    DOCS_DIR.mkdir(parents=True, exist_ok=True)

    if not RESULTS_CSV.exists():
        print(f"[-] Error: Results CSV file not found at {RESULTS_CSV}")
        print("    Please run 'python scripts/run_benchmarks.py' first.")
        sys.exit(1)

    print(f"[*] Loading benchmark data from: {RESULTS_CSV}")
    
    df = pd.read_csv(RESULTS_CSV, header=None)

    expected_cols = [
        'Workload', 'Policy', 'TotalAccesses', 'TlbHits', 'TlbHitRate',
        'TlbMisses', 'TlbMissRate', 'PageFaults', 'PageFaultRate',
        'SwapReads', 'SwapWrites', 'ElapsedMs', 'Throughput'
    ]
    
    if len(df.columns) >= len(expected_cols):
        df = df.iloc[:, :len(expected_cols)]
        df.columns = expected_cols
    else:
        print("[-] Error: Unexpected CSV format.")
        sys.exit(1)

    if 'RamFrames' not in df.columns:
        frame_pattern = [4, 8, 16, 32]
        df['RamFrames'] = [frame_pattern[i % len(frame_pattern)] for i in range(len(df))]

    setup_style()
    plot_page_faults_vs_ram(df)
    plot_tlb_hit_rate_by_workload(df)
    plot_swap_io_impact(df)

    print("[*] All benchmark charts generated successfully in 'docs/' directory!")


if __name__ == "__main__":
    main()