/**
 * @file config.h
 * @brief Configuration structures and default settings for the simulator.
 */

#ifndef VMM_CONFIG_H
#define VMM_CONFIG_H

#include "vmm/types.h"

/** Default System Configuration Constants */
#define DEFAULT_PAGE_SIZE_BYTES 4096    /**< 4 KB Page Size */
#define DEFAULT_RAM_SIZE_BYTES 65536    /**< 64 KB RAM (16 Frames) */
#define DEFAULT_SWAP_SIZE_BYTES 1048576 /**< 1 MB Swap Space */
#define DEFAULT_TLB_ENTRIES 16          /**< 16 TLB entries */

/** @brief Configuration settings structure for VMM instantiation. */
typedef struct
{
    uint32_t page_size;    /**< Page/Frame size in bytes. */
    uint32_t ram_size;     /**< Total physical RAM size in bytes. */
    uint32_t swap_size;    /**< Total swap storage size in bytes. */
    uint32_t tlb_capacity; /**< Number of entries in TLB. */

    // Calculated fields
    uint32_t total_frames;     /**< Calculated: ram_size / page_size */
    uint32_t total_swap_pages; /**< Calculated: swap_size / page_size */
    uint32_t page_offset_bits; /**< Calculated: log2(page_size) */
} vmm_config_t;

/**
 * @brief Initializes a configuration struct with default values and calculates offsets.
 * @param config Pointer to user-allocated vmm_config_t structure.
 * @return true if configuration is valid, false otherwise.
 */
bool vmm_config_init_default(vmm_config_t *config);

#endif // VMM_CONFIG_H