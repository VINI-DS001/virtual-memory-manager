/**
 * @file benchmark.h
 * @brief Benchmark orchestrator engine for Virtual Memory Manager evaluation.
 */

#ifndef VMM_BENCHMARK_H
#define VMM_BENCHMARK_H

#include "vmm/config.h"
#include "vmm/stats.h"
#include "vmm/workload.h"
#include "vmm/replacement.h"
#include "vmm/types.h"

/**
 * @brief Configuration parameters for a benchmark run.
 */
typedef struct
{
    replacement_algo_t policy;
    uint32_t pid;
    uint32_t custom_frame_count;  /**< Set to 0 to use vmm_config_t default. */
    uint32_t custom_tlb_capacity; /**< Set to 0 to use vmm_config_t default. */
} benchmark_config_t;

/**
 * @brief Orchestrates a complete benchmark execution pass.
 * @param bench_cfg Specific parameters for this benchmark execution.
 * @param workload Pre-generated memory access pattern trace.
 * @param base_cfg System configuration parameters.
 * @param out_stats Pointer to vmm_stats_t where results will be stored.
 */
void vmm_benchmark_run(const benchmark_config_t *bench_cfg,
                       const workload_t *workload,
                       const vmm_config_t *base_cfg,
                       vmm_stats_t *out_stats);

#endif // VMM_BENCHMARK_H