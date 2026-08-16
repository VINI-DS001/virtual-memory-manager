#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "vmm/config.h"
#include "vmm/types.h"
#include "vmm/stats.h"
#include "vmm/workload.h"
#include "vmm/benchmark.h"
#include "vmm/replacement.h"
#include "vmm/utils/log.h"

static void print_usage(const char *prog_name)
{
    printf("Usage: %s [options]\n", prog_name);
    printf("Options:\n");
    printf("  --policy <fifo|lru|lfu|clock|random>   Set replacement policy (default: lru)\n");
    printf("  --workload <seq|rand|temp|spatial|thrash> Set workload type (default: temp)\n");
    printf("  --accesses <num>                        Number of memory accesses (default: 100000)\n");
    printf("  --frames <num>                          Number of physical frames (default: 8)\n");
    printf("  --tlb-size <num>                        Number of TLB entries (default: 16)\n");
    printf("  --output <filename.csv>                 Append stats to CSV file\n");
    printf("  --help                                  Display this help message\n");
}

static replacement_algo_t parse_policy(const char *str)
{
    if (strcmp(str, "fifo") == 0)
        return REPLACEMENT_FIFO;
    if (strcmp(str, "lru") == 0)
        return REPLACEMENT_LRU;
    if (strcmp(str, "lfu") == 0)
        return REPLACEMENT_LFU;
    if (strcmp(str, "clock") == 0)
        return REPLACEMENT_CLOCK;
    if (strcmp(str, "random") == 0)
        return REPLACEMENT_RANDOM;
    return REPLACEMENT_LRU; // Default fallback
}

static workload_type_t parse_workload(const char *str)
{
    if (strcmp(str, "seq") == 0)
        return WORKLOAD_SEQUENTIAL;
    if (strcmp(str, "rand") == 0)
        return WORKLOAD_RANDOM;
    if (strcmp(str, "temp") == 0)
        return WORKLOAD_TEMPORAL_LOCALITY;
    if (strcmp(str, "spatial") == 0)
        return WORKLOAD_SPATIAL_LOCALITY;
    if (strcmp(str, "thrash") == 0)
        return WORKLOAD_THRASHING;
    return WORKLOAD_TEMPORAL_LOCALITY; // Default fallback
}

static log_level_t parse_log_level(const char *str)
{
    if (strcmp(str, "debug") == 0)
        return LOG_LEVEL_DEBUG;
    if (strcmp(str, "info") == 0)
        return LOG_LEVEL_INFO;
    if (strcmp(str, "warn") == 0)
        return LOG_LEVEL_WARN;
    if (strcmp(str, "error") == 0)
        return LOG_LEVEL_ERROR;
    return LOG_LEVEL_ERROR; // Default for benchmarks
}

int main(int argc, char *argv[])
{

    replacement_algo_t policy = REPLACEMENT_LRU;
    workload_type_t workload_type = WORKLOAD_TEMPORAL_LOCALITY;
    size_t total_accesses = 100000;
    uint32_t custom_frames = 8;
    uint32_t custom_tlb_size = 16;
    log_level_t log_level = LOG_LEVEL_ERROR; // Default high performance
    const char *output_csv = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--policy") == 0 && i + 1 < argc)
        {
            policy = parse_policy(argv[++i]);
        }
        else if (strcmp(argv[i], "--workload") == 0 && i + 1 < argc)
        {
            workload_type = parse_workload(argv[++i]);
        }
        else if (strcmp(argv[i], "--accesses") == 0 && i + 1 < argc)
        {
            total_accesses = (size_t)atol(argv[++i]);
        }
        else if (strcmp(argv[i], "--frames") == 0 && i + 1 < argc)
        {
            custom_frames = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--tlb-size") == 0 && i + 1 < argc)
        {
            custom_tlb_size = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--log-level") == 0 && i + 1 < argc)
        {
            log_level = parse_log_level(argv[++i]);
        }
        else if (strcmp(argv[i], "--output") == 0 && i + 1 < argc)
        {
            output_csv = argv[++i];
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            print_usage(argv[0]);
            return 0;
        }
    }

    /* Configure log level dynamically based on CLI flag */
    log_set_level(log_level);

    /* Initialize base configuration */
    vmm_config_t config;
    vmm_config_init_default(&config);

    /* Generate Workload */
    workload_t wl;
    uint32_t max_vpn = 64; // Virtual page scope for benchmark
    workload_generate(&wl, workload_type, total_accesses, max_vpn, config.page_size);

    /* Configure Benchmark Pass */
    benchmark_config_t b_cfg = {
        .policy = policy,
        .pid = 1,
        .custom_frame_count = custom_frames,
        .custom_tlb_capacity = custom_tlb_size};

    /* Execute Benchmark */
    vmm_stats_t stats;
    vmm_benchmark_run(&b_cfg, &wl, &config, &stats);

    const char *workload_str = workload_type_to_string(workload_type);
    const char *policy_str = replacement_algo_to_string(policy);

    /* Print Console Report */
    vmm_stats_print_report(&stats, workload_str, policy_str);

    /* Export to CSV if requested */
    if (output_csv)
    {
        FILE *fp = fopen(output_csv, "a");

        if (!fp)
        {
            perror("Failed to open CSV output file");
            workload_destroy(&wl);
            return 1;
        }

        vmm_stats_export_csv_row(&stats, workload_str, policy_str, fp);
        fclose(fp);

        LOG_INFO("Exported benchmark stats to CSV: %s", output_csv);
    }

    workload_destroy(&wl);
    return 0;
}