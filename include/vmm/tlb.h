/**
 * @file tlb.h
 * @brief Translation Lookaside Buffer (TLB) hardware cache.
 */

#ifndef VMM_TLB_H
#define VMM_TLB_H

#include "vmm/types.h"
#include "vmm/config.h"

/** @brief Single TLB cache entry. */
typedef struct
{
    vpn_t vpn;          /**< Virtual Page Number key. */
    pfn_t pfn;          /**< Mapped Physical Frame Number. */
    bool valid;         /**< True if entry holds valid translation. */
    uint64_t last_used; /**< Timestamp for LRU replacement strategy. */
} tlb_entry_t;

/** @brief TLB Hardware Cache Controller. */
typedef struct
{
    tlb_entry_t *entries;  /**< Array of TLB cache slots. */
    uint32_t capacity;     /**< Maximum TLB entries (e.g. 4 or 8). */
    uint64_t access_clock; /**< Logical counter for LRU tracking. */

    // Performance Metrics
    uint64_t hits;
    uint64_t misses;
} tlb_t;

/**
 * @brief Initializes the TLB cache structure.
 * @param tlb Pointer to tlb_t instance.
 * @param capacity Number of cache entries supported.
 * @return true on success, false on allocation failure.
 */
bool tlb_init(tlb_t *tlb, uint32_t capacity);

/**
 * @brief Frees memory associated with TLB.
 * @param tlb Pointer to tlb_t instance.
 */
void tlb_destroy(tlb_t *tlb);

/**
 * @brief Searches TLB for a Virtual Page Number mapping.
 * @param tlb Pointer to tlb_t instance.
 * @param vpn Virtual Page Number to lookup.
 * @param out_pfn Output pointer to store associated PFN on hit.
 * @return true on TLB Hit, false on TLB Miss.
 */
bool tlb_lookup(tlb_t *tlb, vpn_t vpn, pfn_t *out_pfn);

/**
 * @brief Inserts or updates a translation mapping in TLB using LRU replacement.
 * @param tlb Pointer to tlb_t instance.
 * @param vpn Virtual Page Number.
 * @param pfn Mapped Physical Frame Number.
 */
void tlb_insert(tlb_t *tlb, vpn_t vpn, pfn_t pfn);

/**
 * @brief Involuntarily flushes/invalidates all TLB entries (e.g. on context switch).
 * @param tlb Pointer to tlb_t instance.
 */
void tlb_flush(tlb_t *tlb);

/**
 * @brief Calculates current TLB hit rate percentage.
 * @param tlb Pointer to tlb_t instance.
 * @return Hit rate as float percentage (0.0 to 100.0).
 */
float tlb_get_hit_rate(const tlb_t *tlb);

#endif // VMM_TLB_H