/**
 * @file cpu.h
 * @brief Virtual CPU simulation for generating memory access workloads.
 */

#ifndef VMM_CPU_H
#define VMM_CPU_H

#include "vmm/types.h"
#include "vmm/config.h"

/** @brief Access patterns that the CPU can execute. */
typedef enum
{
    PATTERN_SEQUENTIAL = 0, /**< Sequential memory accesses (e.g., array traversal). */
    PATTERN_RANDOM,         /**< Uniformly random memory accesses. */
    PATTERN_LOCALITY,       /**< High temporal and spatial locality (loops, localized data). */
    PATTERN_WORKING_SET     /**< Accesses restricted to a moving working set. */
} access_pattern_t;

/** @brief CPU instance state structure. */
typedef struct
{
    uint32_t process_id;       /**< Identifier of the process running on CPU. */
    vaddr_t current_pc;        /**< Program Counter / Current Virtual Address. */
    uint64_t clock_ticks;      /**< Monotonic logical clock tick counter. */
    access_pattern_t pattern;  /**< Active memory access pattern. */
    vaddr_t address_space_max; /**< Maximum virtual address allowed. */
} cpu_t;

/**
 * @brief Initializes a Virtual CPU instance.
 * @param cpu Pointer to allocated cpu_t structure.
 * @param process_id ID of the process assigned to this CPU.
 * @param address_space_max Maximum virtual address range.
 */
void cpu_init(cpu_t *cpu, uint32_t process_id, vaddr_t address_space_max);

/**
 * @brief Sets the memory access workload pattern for the CPU.
 * @param cpu Pointer to cpu_t instance.
 * @param pattern Access pattern to simulate.
 */
void cpu_set_pattern(cpu_t *cpu, access_pattern_t pattern);

/**
 * @brief Generates the next memory access request according to active pattern.
 * @param cpu Pointer to cpu_t instance.
 * @return mem_access_req_t Structure containing virtual address, op_type, pid, timestamp.
 */
mem_access_req_t cpu_fetch_next_access(cpu_t *cpu);

#endif // VMM_CPU_H