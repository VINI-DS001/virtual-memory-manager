#include "vmm/page_fault.h"
#include "vmm/utils/log.h"
#include "vmm/swap.h"
#include <string.h>

bool handle_page_fault(uint32_t pid, vpn_t vpn, page_table_t *pt, ram_t *ram,
                       tlb_t *tlb, replacement_manager_t *repl_mgr, swap_disk_t *swap)
{
    if (!pt || !ram || !repl_mgr || !swap)
        return false;

    LOG_WARN("[PAGE FAULT HANDLER] Interrupted CPU execution for PID %u on VPN %u", pid, vpn);

    pfn_t target_pfn = 0;
    bool allocated = ram_allocate_frame(ram, pid, vpn, &target_pfn);

    // STEP 1: If RAM is FULL -> SWAP OUT VICTIM PAGE
    if (!allocated)
    {
        pfn_t victim_pfn = 0;
        vpn_t victim_vpn = 0;

        if (!replacement_select_victim(repl_mgr, &victim_pfn, &victim_vpn))
        {
            LOG_ERROR("[PAGE FAULT HANDLER] Eviction failed!");
            return false;
        }

        // Allocate slot on Swap Disk
        int32_t swap_slot = swap_allocate_slot(swap);
        if (swap_slot < 0)
        {
            LOG_ERROR("[PAGE FAULT HANDLER] Swap Disk Full! Critical Thrashing!");
            return false;
        }

        LOG_INFO("[PAGE FAULT HANDLER] Evicting VPN %u from RAM Frame %u -> SWAP OUT to Slot %d",
                 victim_vpn, victim_pfn, swap_slot);

        // Read physical RAM frame data & write to Swap Disk
        uint8_t page_data_buffer[ram->page_size];
        paddr_t victim_paddr = (paddr_t)victim_pfn * ram->page_size;
        ram_read(ram, victim_paddr, page_data_buffer, ram->page_size);
        swap_write_page(swap, (uint32_t)swap_slot, page_data_buffer);

        // Mark victim page in Page Table as SWAPPED and invalidate TLB
        page_table_mark_swapped(pt, victim_vpn, (uint32_t)swap_slot);
        if (tlb)
            tlb_flush(tlb);

        // Reclaim RAM frame for new request
        ram_free_frame(ram, victim_pfn);
        ram_allocate_frame(ram, pid, vpn, &target_pfn);
    }

    // STEP 2: SWAP IN OR DEMAND ZERO FILL
    page_table_entry_t entry;
    page_table_lookup(pt, vpn, &entry);
    paddr_t frame_paddr = (paddr_t)target_pfn * ram->page_size;

    if (entry.flags & PAGE_FLAG_SWAPPED)
    {
        // Page was previously swapped out -> Read back from Disk (SWAP IN)
        LOG_INFO("[PAGE FAULT HANDLER] Page VPN %u found in Swap Slot %u -> SWAP IN to RAM Frame %u",
                 vpn, entry.swap_page_id, target_pfn);

        uint8_t page_data_buffer[ram->page_size];
        swap_read_page(swap, entry.swap_page_id, page_data_buffer);
        ram_write(ram, frame_paddr, page_data_buffer, ram->page_size);

        // Free disk slot after reading back into RAM
        swap_free_slot(swap, entry.swap_page_id);
    }
    else
    {
        // Brand new page -> Zero-fill frame (Demand Zero Paging)
        uint8_t zero_buffer[ram->page_size];
        memset(zero_buffer, 0, ram->page_size);
        ram_write(ram, frame_paddr, zero_buffer, ram->page_size);
    }

    // STEP 3: Update Page Table & Replacement Manager
    uint8_t flags = PAGE_FLAG_READABLE | PAGE_FLAG_WRITABLE | PAGE_FLAG_PRESENT;
    page_table_map(pt, vpn, target_pfn, flags);
    replacement_register_allocation(repl_mgr, target_pfn, vpn);

    LOG_INFO("[PAGE FAULT HANDLER] Resolved! Mapped VPN %u -> Physical Frame %u", vpn, target_pfn);
    return true;
}