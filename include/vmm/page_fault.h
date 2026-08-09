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

/**
 * @brief Handles Page Fault exception by allocating a free RAM frame for the target VPN.
 * @param pid Process ID requesting access.
 * @param vpn Virtual Page Number causing the fault.
 * @param pt Pointer to active Page Table.
 * @param ram Pointer to physical RAM.
 * @return true if page fault was successfully resolved, false if out of physical RAM.
 */
bool handle_page_fault(uint32_t pid, vpn_t vpn, page_table_t *pt, ram_t *ram);

#endif // VMM_PAGE_FAULT_H