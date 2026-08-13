/**
 * @file page_fault.h
 * @brief OS Kernel Page Fault Handler interface for handling unmapped memory access.
 */

#ifndef VMM_PAGE_FAULT_H
#define VMM_PAGE_FAULT_H

#include <stdbool.h>
#include <stdint.h>
#include "vmm/types.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/replacement.h"

/**
 * @brief Handles Page Fault exception by allocating a free RAM frame or evicting a page when RAM is full.
 * @param pid Process ID requesting access.
 * @param vpn Virtual Page Number causing the fault.
 * @param pt Pointer to active Page Table.
 * @param ram Pointer to physical RAM.
 * @param tlb Pointer to TLB cache (invalidated/flushed on page eviction).
 * @param repl_mgr Pointer to Page Replacement Manager instance.
 * @return true if page fault was successfully resolved, false if eviction/allocation failed.
 */
bool handle_page_fault(uint32_t pid, vpn_t vpn, page_table_t *pt, ram_t *ram, tlb_t *tlb, replacement_manager_t *repl_mgr);

#endif // VMM_PAGE_FAULT_H