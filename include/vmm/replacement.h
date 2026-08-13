/**
 * @file replacement.h
 * @brief Page Replacement Policy Manager supporting dynamic algorithm selection.
 */

#ifndef VMM_REPLACEMENT_H
#define VMM_REPLACEMENT_H

#include <stdbool.h>
#include <stdint.h>
#include "vmm/types.h"
#include "vmm/page_table.h"
#include "vmm/ram.h"

/** @brief Supported Page Replacement Algorithms. */
typedef enum
{
    REPLACEMENT_FIFO,
    REPLACEMENT_LRU,
    REPLACEMENT_CLOCK,
    REPLACEMENT_SECOND_CHANCE,
    REPLACEMENT_LFU,
    REPLACEMENT_RANDOM
} replacement_algo_t;

/** @brief Tracking structure for replacement algorithms metadata. */
typedef struct
{
    vpn_t vpn;             /**< Virtual Page Number assigned to frame. */
    pfn_t pfn;             /**< Physical Frame Number. */
    bool valid;            /**< Is slot currently tracking a frame. */
    uint64_t load_time;    /**< Timestamp when loaded into RAM (FIFO). */
    uint64_t last_access;  /**< Timestamp of last access (LRU). */
    uint64_t access_count; /**< Total frequency counter (LFU). */
    bool reference_bit;    /**< Reference bit for Clock / Second Chance. */
} frame_meta_t;

/** @brief Global Replacement Manager Controller. */
typedef struct
{
    replacement_algo_t current_algo;
    frame_meta_t *frames_meta;
    uint32_t total_frames;
    uint64_t global_clock;
    uint32_t clock_hand; /**< Pointer for Clock/Second Chance algorithm. */
} replacement_manager_t;

/**
 * @brief Initializes the Replacement Manager.
 * @param mgr Pointer to replacement_manager_t structure.
 * @param total_frames Number of physical RAM frames to manage.
 * @param default_algo Initial replacement algorithm to use.
 * @return true on success, false on allocation failure.
 */
bool replacement_init(replacement_manager_t *mgr, uint32_t total_frames, replacement_algo_t default_algo);

/**
 * @brief Releases allocated replacement metadata memory.
 * @param mgr Pointer to replacement_manager_t instance.
 */
void replacement_destroy(replacement_manager_t *mgr);

/**
 * @brief Dynamically updates active replacement algorithm.
 * @param mgr Pointer to replacement_manager_t instance.
 * @param algo New replacement algorithm strategy.
 */
void replacement_set_algorithm(replacement_manager_t *mgr, replacement_algo_t algo);

/**
 * @brief Registers page access to update metadata (LRU, Clock, LFU counters).
 * @param mgr Pointer to replacement_manager_t instance.
 * @param pfn Physical Frame Number accessed.
 */
void replacement_register_access(replacement_manager_t *mgr, pfn_t pfn);

/**
 * @brief Registers allocation of a new page frame.
 * @param mgr Pointer to replacement_manager_t instance.
 * @param pfn Physical Frame Number allocated.
 * @param vpn Virtual Page Number assigned.
 */
void replacement_register_allocation(replacement_manager_t *mgr, pfn_t pfn, vpn_t vpn);

/**
 * @brief Selects victim Physical Frame Number according to active policy.
 * @param mgr Pointer to replacement_manager_t instance.
 * @param out_pfn Pointer to store victim PFN.
 * @param out_vpn Pointer to store victim VPN.
 * @return true if victim was chosen, false if no valid frame to evict.
 */
bool replacement_select_victim(replacement_manager_t *mgr, pfn_t *out_pfn, vpn_t *out_vpn);

/**
 * @brief Returns string name of current active algorithm.
 * @param algo Algorithm enum identifier.
 * @return Const string representation.
 */
const char *replacement_algo_to_string(replacement_algo_t algo);

#endif // VMM_REPLACEMENT_H