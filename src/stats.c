#include "vmm/stats.h"
#include "vmm/utils/log.h"
#include <string.h>

void vmm_stats_init(vmm_stats_t *stats)
{
    if (!stats)
        return;
    memset(stats, 0, sizeof(vmm_stats_t));
}

void vmm_stats_start_timer(vmm_stats_t *stats)
{
    if (!stats)
        return;
    clock_gettime(CLOCK_MONOTONIC, &stats->start_time);
}

void vmm_stats_stop_timer(vmm_stats_t *stats)
{
    if (!stats)
        return;
    clock_gettime(CLOCK_MONOTONIC, &stats->end_time);

    double start_sec = (double)stats->start_time.tv_sec + ((double)stats->start_time.tv_nsec / 1e9);
    double end_sec = (double)stats->end_time.tv_sec + ((double)stats->end_time.tv_nsec / 1e9);

    stats->elapsed_ms = (end_sec - start_sec) * 1000.0;
}

void vmm_stats_record_access(vmm_stats_t *stats, bool is_tlb_hit)
{
    if (!stats)
        return;
    stats->total_accesses++;
    if (is_tlb_hit)
    {
        stats->tlb_hits++;
    }
    else
    {
        stats->tlb_misses++;
    }
}

void vmm_stats_record_page_fault(vmm_stats_t *stats)
{
    if (!stats)
        return;
    stats->page_faults++;
}

void vmm_stats_record_swap_read(vmm_stats_t *stats)
{
    if (!stats)
        return;
    stats->swap_reads++;
}

void vmm_stats_record_swap_write(vmm_stats_t *stats)
{
    if (!stats)
        return;
    stats->swap_writes++;
}

double vmm_stats_get_tlb_hit_rate(const vmm_stats_t *stats)
{
    if (!stats || stats->total_accesses == 0)
        return 0.0;
    return ((double)stats->tlb_hits / (double)stats->total_accesses) * 100.0;
}

double vmm_stats_get_tlb_miss_rate(const vmm_stats_t *stats)
{
    if (!stats || stats->total_accesses == 0)
        return 0.0;
    return ((double)stats->tlb_misses / (double)stats->total_accesses) * 100.0;
}

double vmm_stats_get_page_fault_rate(const vmm_stats_t *stats)
{
    if (!stats || stats->total_accesses == 0)
        return 0.0;
    return ((double)stats->page_faults / (double)stats->total_accesses) * 100.0;
}

double vmm_stats_get_throughput(const vmm_stats_t *stats)
{
    if (!stats || stats->elapsed_ms <= 0.0)
        return 0.0;
    return ((double)stats->total_accesses / (stats->elapsed_ms / 1000.0));
}

void vmm_stats_print_report(const vmm_stats_t *stats, const char *workload_name, const char *algo_name)
{
    if (!stats)
        return;

    printf("\n===================================================\n");
    printf("        VMM BENCHMARK REPORT (%s - %s)\n", workload_name ? workload_name : "N/A", algo_name ? algo_name : "N/A");
    printf("===================================================\n");
    printf(" Total Accesses   : %llu\n", (unsigned long long)stats->total_accesses);
    printf(" TLB Hits         : %llu (%.2f%%)\n", (unsigned long long)stats->tlb_hits, vmm_stats_get_tlb_hit_rate(stats));
    printf(" TLB Misses       : %llu (%.2f%%)\n", (unsigned long long)stats->tlb_misses, vmm_stats_get_tlb_miss_rate(stats));
    printf(" Page Faults      : %llu (%.4f%%)\n", (unsigned long long)stats->page_faults, vmm_stats_get_page_fault_rate(stats));
    printf(" Swap Reads (In)  : %llu\n", (unsigned long long)stats->swap_reads);
    printf(" Swap Writes (Out): %llu\n", (unsigned long long)stats->swap_writes);
    printf(" Execution Time   : %.3f ms\n", stats->elapsed_ms);
    printf(" Throughput       : %.2f ops/sec\n", vmm_stats_get_throughput(stats));
    printf("===================================================\n\n");
}

void vmm_stats_export_csv_row(const vmm_stats_t *stats, const char *workload_name, const char *algo_name, FILE *fp)
{
    if (!stats || !fp)
        return;

    fprintf(fp, "%s,%s,%llu,%llu,%.2f,%llu,%.2f,%llu,%.4f,%llu,%llu,%.3f,%.2f\n",
            workload_name ? workload_name : "N/A",
            algo_name ? algo_name : "N/A",
            (unsigned long long)stats->total_accesses,
            (unsigned long long)stats->tlb_hits,
            vmm_stats_get_tlb_hit_rate(stats),
            (unsigned long long)stats->tlb_misses,
            vmm_stats_get_tlb_miss_rate(stats),
            (unsigned long long)stats->page_faults,
            vmm_stats_get_page_fault_rate(stats),
            (unsigned long long)stats->swap_reads,
            (unsigned long long)stats->swap_writes,
            stats->elapsed_ms,
            vmm_stats_get_throughput(stats));
}