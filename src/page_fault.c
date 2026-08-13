#include "vmm/page_fault.h"
#include "vmm/utils/log.h"
#include <string.h>

bool handle_page_fault(uint32_t pid, vpn_t vpn, page_table_t *pt, ram_t *ram, tlb_t *tlb, replacement_manager_t *repl_mgr)
{
    if (!pt || !ram || !repl_mgr)
        return false;

    LOG_WARN("[PAGE FAULT HANDLER] Interrupted CPU execution for PID %u on VPN %u", pid, vpn);

    pfn_t target_pfn = 0;
    bool allocated = ram_allocate_frame(ram, pid, vpn, &target_pfn);

    // RAM IS FULL -> EVICTION REQUIRED!
    if (!allocated)
    {
        pfn_t victim_pfn = 0;
        vpn_t victim_vpn = 0;

        if (!replacement_select_victim(repl_mgr, &victim_pfn, &victim_vpn))
        {
            LOG_ERROR("[PAGE FAULT HANDLER] Eviction failed: Out of memory!");
            return false;
        }

        LOG_INFO("[PAGE FAULT HANDLER] Evicting Page VPN %u from Frame %u", victim_vpn, victim_pfn);

        // Invalidate old mapping in Page Table and TLB
        page_table_unmap(pt, victim_vpn);
        if (tlb)
            tlb_flush(tlb); // Flush TLB to purge stale mapping

        // Reclaim freed frame
        ram_free_frame(ram, victim_pfn);
        ram_allocate_frame(ram, pid, vpn, &target_pfn);
    }

    // Zero-fill physical frame (Demand Zero)
    paddr_t frame_paddr = (paddr_t)target_pfn * ram->page_size;
    uint8_t zero_buffer[ram->page_size];
    memset(zero_buffer, 0, ram->page_size);
    ram_write(ram, frame_paddr, zero_buffer, ram->page_size);

    // Map new page in Page Table
    uint8_t flags = PAGE_FLAG_READABLE | PAGE_FLAG_WRITABLE | PAGE_FLAG_PRESENT;
    page_table_map(pt, vpn, target_pfn, flags);

    // Register frame allocation in Replacement Manager
    replacement_register_allocation(repl_mgr, target_pfn, vpn);

    LOG_INFO("[PAGE FAULT HANDLER] Resolved! Mapped VPN %u -> Frame %u", vpn, target_pfn);
    return true;
}