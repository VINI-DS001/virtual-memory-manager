/**
 * @file workload.h
 * @brief Memory access trace generator for VMM benchmarks.
 */

#ifndef VMM_WORKLOAD_H
#define VMM_WORKLOAD_H

#include <stdint.h>
#include <stddef.h>
#include "vmm/types.h"

typedef enum
{
    WORKLOAD_SEQUENTIAL,
    WORKLOAD_RANDOM,
    WORKLOAD_TEMPORAL_LOCALITY,
    WORKLOAD_SPATIAL_LOCALITY,
    WORKLOAD_THRASHING
} workload_type_t;

typedef struct
{
    workload_type_t type;
    size_t total_accesses;
    uint32_t max_vpn;
    vaddr_t *access_pattern; // Array storing pre-generated virtual addresses
} workload_t;

/**
 * @brief Generates a workload trace array based on pattern type.
 * @param wl Pointer to workload structure to populate.
 * @param type Type of access pattern.
 * @param total_accesses Number of memory access operations to simulate.
 * @param max_vpn Highest Virtual Page Number available in the virtual address space.
 * @param page_size Size of a page in bytes (e.g., 4096).
 */
void workload_generate(workload_t *wl, workload_type_t type, size_t total_accesses, uint32_t max_vpn, uint32_t page_size);

/**
 * @brief Releases memory allocated for the workload pattern.
 * @param wl Pointer to workload structure.
 */
void workload_destroy(workload_t *wl);

/**
 * @brief Returns human-readable string name of the workload type.
 */
const char *workload_type_to_string(workload_type_t type);

#endif // VMM_WORKLOAD_H