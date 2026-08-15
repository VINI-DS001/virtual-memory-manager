#include <assert.h>
#include "vmm/benchmark.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Benchmark Orchestrator Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    workload_t wl;
    size_t total_accesses = 500;
    uint32_t max_vpn = 16;
    workload_generate(&wl, WORKLOAD_TEMPORAL_LOCALITY, total_accesses, max_vpn, config.page_size);

    benchmark_config_t b_cfg = {
        .policy = REPLACEMENT_FIFO,
        .pid = 101,
        .custom_frame_count = 4,
        .custom_tlb_capacity = 4};

    vmm_stats_t stats;
    vmm_benchmark_run(&b_cfg, &wl, &config, &stats);

    assert(stats.total_accesses == total_accesses);
    assert(stats.tlb_hits + stats.tlb_misses == total_accesses);
    assert(stats.elapsed_ms > 0.0);

    vmm_stats_print_report(&stats, "TemporalLocality", "FIFO");

    workload_destroy(&wl);

    LOG_INFO("Benchmark Orchestrator Sanity Test Passed Successfully!");
    return 0;
}