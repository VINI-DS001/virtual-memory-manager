/**
 * @file swap.h
 * @brief Simulated Secondary Storage (Disk Swap Space) interface.
 */

#ifndef VMM_SWAP_H
#define VMM_SWAP_H

#include <stdbool.h>
#include <stdint.h>
#include "vmm/types.h"
#include "vmm/config.h"

typedef struct
{
    uint8_t *storage;     /**< Simulated disk block buffer. */
    bool *occupied;       /**< Bitmap tracking active swap slots. */
    uint32_t total_pages; /**< Total available disk swap slots. */
    uint32_t page_size;   /**< Page size in bytes. */
    uint64_t swap_writes; /**< Counter for Swap Out operations. */
    uint64_t swap_reads;  /**< Counter for Swap In operations. */
} swap_disk_t;

/**
 * @brief Initializes simulated disk swap space.
 */
bool swap_init(swap_disk_t *swap, const vmm_config_t *config);

/**
 * @brief Releases swap disk resources.
 */
void swap_destroy(swap_disk_t *swap);

/**
 * @brief Allocates an available slot on swap disk.
 * @return Slot index or -1 if swap space is full (Thrashing / Out of Disk).
 */
int32_t swap_allocate_slot(swap_disk_t *swap);

/**
 * @brief Frees a swap slot.
 */
void swap_free_slot(swap_disk_t *swap, uint32_t slot_id);

/**
 * @brief Writes RAM frame data to Swap Disk (Swap Out).
 */
bool swap_write_page(swap_disk_t *swap, uint32_t slot_id, const uint8_t *buffer);

/**
 * @brief Reads data from Swap Disk to RAM frame (Swap In).
 */
bool swap_read_page(swap_disk_t *swap, uint32_t slot_id, uint8_t *buffer);

#endif // VMM_SWAP_H