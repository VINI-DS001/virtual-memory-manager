/**
 * @file stats.h
 * @brief Performance statistics collector for Virtual Memory Manager benchmarks.
 */

#ifndef VMM_STATS_H
#define VMM_STATS_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

/**
 * @brief Holds execution metrics for a benchmark run.
 */
typedef struct
{
    uint64_t total_accesses;
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t page_faults;
    uint64_t swap_reads;
    uint64_t swap_writes;

    /* Time measurement */
    struct timespec start_time;
    struct timespec end_time;
    double elapsed_ms;
} vmm_stats_t;

/**
 * @brief Initializes/resets a statistics structure to zero.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_init(vmm_stats_t *stats);

/**
 * @brief Starts the high-resolution timer.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_start_timer(vmm_stats_t *stats);

/**
 * @brief Stops the timer and calculates elapsed time in milliseconds.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_stop_timer(vmm_stats_t *stats);

/**
 * @brief Records a memory access (TLB hit or miss).
 * @param stats Pointer to vmm_stats_t instance.
 * @param is_tlb_hit True if access hit in TLB, false if miss.
 */
void vmm_stats_record_access(vmm_stats_t *stats, bool is_tlb_hit);

/**
 * @brief Increments page fault counter.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_record_page_fault(vmm_stats_t *stats);

/**
 * @brief Increments swap read counter.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_record_swap_read(vmm_stats_t *stats);

/**
 * @brief Increments swap write counter.
 * @param stats Pointer to vmm_stats_t instance.
 */
void vmm_stats_record_swap_write(vmm_stats_t *stats);

/* Derived metrics calculations */
double vmm_stats_get_tlb_hit_rate(const vmm_stats_t *stats);
double vmm_stats_get_tlb_miss_rate(const vmm_stats_t *stats);
double vmm_stats_get_page_fault_rate(const vmm_stats_t *stats);
double vmm_stats_get_throughput(const vmm_stats_t *stats);

/**
 * @brief Prints formatted metrics report to stdout.
 * @param stats Pointer to vmm_stats_t instance.
 * @param workload_name Name of the benchmark workload.
 * @param algo_name Replacement algorithm used.
 */
void vmm_stats_print_report(const vmm_stats_t *stats, const char *workload_name, const char *algo_name);

/**
 * @brief Writes metrics row to a CSV file descriptor.
 * @param stats Pointer to vmm_stats_t instance.
 * @param workload_name Name of the benchmark workload.
 * @param algo_name Replacement algorithm used.
 * @param fp Pointer to target FILE output.
 */
void vmm_stats_export_csv_row(const vmm_stats_t *stats, const char *workload_name, const char *algo_name, FILE *fp);

#endif // VMM_STATS_H