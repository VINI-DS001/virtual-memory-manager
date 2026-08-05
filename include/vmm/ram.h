/**
 * @file ram.h
 * @brief Physical RAM and frame management simulation.
 */

#ifndef VMM_RAM_H
#define VMM_RAM_H

#include "vmm/types.h"
#include "vmm/config.h"

/** @brief Physical Frame representation. */
typedef struct
{
    pfn_t pfn;              /**< Physical Frame Number. */
    bool is_free;           /**< True if frame is unallocated. */
    uint32_t allocated_pid; /**< PID of process currently holding the frame. */
    vpn_t mapped_vpn;       /**< Virtual Page Number mapped to this frame. */
} frame_t;

/** @brief Physical RAM controller instance structure. */
typedef struct
{
    uint8_t *memory_buffer; /**< Raw contiguous physical memory block. */
    frame_t *frames;        /**< Array of frame metadata structures. */
    uint32_t total_frames;  /**< Total number of physical frames. */
    uint32_t free_frames;   /**< Count of currently unallocated frames. */
    uint32_t page_size;     /**< Size of each page/frame in bytes. */
} ram_t;

/**
 * @brief Initializes the physical RAM system based on simulator configuration.
 * @param ram Pointer to allocated ram_t structure.
 * @param config Pointer to active vmm_config_t structure.
 * @return true on success, false on memory allocation failure.
 */
bool ram_init(ram_t *ram, const vmm_config_t *config);

/**
 * @brief Frees all allocated memory buffers associated with RAM.
 * @param ram Pointer to ram_t structure.
 */
void ram_destroy(ram_t *ram);

/**
 * @brief Allocates an available physical frame.
 * @param ram Pointer to ram_t instance.
 * @param pid Process ID requesting allocation.
 * @param vpn Virtual Page Number to map.
 * @param out_pfn Output pointer to store assigned Physical Frame Number.
 * @return true if allocation succeeded, false if RAM is full.
 */
bool ram_allocate_frame(ram_t *ram, uint32_t pid, vpn_t vpn, pfn_t *out_pfn);

/**
 * @brief Releases a physical frame back to the free pool.
 * @param ram Pointer to ram_t instance.
 * @param pfn Physical Frame Number to free.
 */
void ram_free_frame(ram_t *ram, pfn_t pfn);

/**
 * @brief Writes bytes to physical memory.
 * @param ram Pointer to ram_t instance.
 * @param paddr Physical address destination.
 * @param src Source buffer.
 * @param size Number of bytes to write.
 * @return true on valid address range, false otherwise.
 */
bool ram_write(ram_t *ram, paddr_t paddr, const void *src, size_t size);

/**
 * @brief Reads bytes from physical memory.
 * @param ram Pointer to ram_t instance.
 * @param paddr Physical address source.
 * @param dest Destination buffer.
 * @param size Number of bytes to read.
 * @return true on valid address range, false otherwise.
 */
bool ram_read(const ram_t *ram, paddr_t paddr, void *dest, size_t size);

#endif // VMM_RAM_H