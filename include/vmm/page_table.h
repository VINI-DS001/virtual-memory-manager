/**
 * @file page_table.h
 * @brief Page Table entry and management interfaces.
 */

#ifndef VMM_PAGE_TABLE_H
#define VMM_PAGE_TABLE_H

#include <stdbool.h>
#include <stdint.h>
#include "vmm/types.h"

#define PAGE_FLAG_PRESENT (1 << 0)  // 0x01
#define PAGE_FLAG_READABLE (1 << 1) // 0x02
#define PAGE_FLAG_WRITABLE (1 << 2) // 0x04
#define PAGE_FLAG_SWAPPED (1 << 3)  // 0x08

/** @brief Represents a single Page Table Entry (PTE). */
typedef struct
{
    pfn_t pfn;             /**< Physical Frame Number (if PRESENT). */
    uint32_t swap_page_id; /**< Disk Swap Slot ID (if SWAPPED). */
    uint8_t flags;         /**< Status flags (PRESENT, READABLE, WRITABLE, SWAPPED). */
} page_table_entry_t;

/** @brief Simple Single-Level Page Table. */
typedef struct
{
    page_table_entry_t *entries; /**< Array of entries indexed by VPN. */
    uint32_t total_pages;        /**< Capacity of the page table. */
} page_table_t;

/**
 * @brief Initializes a single-level page table.
 * @param pt Pointer to page_table_t instance.
 * @param num_pages Number of virtual pages supported.
 * @return true on success, false on allocation failure.
 */
bool page_table_init(page_table_t *pt, uint32_t max_entries);

/**
 * @brief Frees memory allocated for the page table.
 * @param pt Pointer to page_table_t instance.
 */
void page_table_destroy(page_table_t *pt);

/**
 * @brief Maps a Virtual Page Number (VPN) to a Physical Frame Number (PFN).
 * @param pt Pointer to page_table_t instance.
 * @param vpn Virtual Page Number.
 * @param pfn Physical Frame Number.
 * @param flags Initial access flags (e.g., PAGE_FLAG_PRESENT | PAGE_FLAG_READABLE).
 * @return true if successfully mapped, false if VPN out of bounds.
 */
bool page_table_map(page_table_t *pt, vpn_t vpn, pfn_t pfn, uint8_t flags);

/**
 * @brief Unmaps a Virtual Page Number.
 * @param pt Pointer to page_table_t instance.
 * @param vpn Virtual Page Number to unmap.
 * @return true if unmapped successfully, false if VPN out of bounds.
 */
bool page_table_unmap(page_table_t *pt, vpn_t vpn);

/**
 * @brief Looks up a Page Table Entry by VPN.
 * @param pt Pointer to page_table_t instance.
 * @param vpn Virtual Page Number.
 * @param out_entry Output pointer to copy entry details.
 * @return true if VPN is valid, false otherwise.
 */
bool page_table_lookup(const page_table_t *pt, vpn_t vpn, page_table_entry_t *out_entry);

/**
 * @brief Marks a page as swapped out to secondary storage (Swap Disk).
 * @param pt Pointer to page_table_t instance.
 * @param vpn Virtual Page Number.
 * @param swap_page_id Slot index allocated on the Swap Disk.
 * @return true if marked successfully, false if VPN out of bounds.
 */
bool page_table_mark_swapped(page_table_t *pt, vpn_t vpn, uint32_t swap_page_id);

#endif // VMM_PAGE_TABLE_H