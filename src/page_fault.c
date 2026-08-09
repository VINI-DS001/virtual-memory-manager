#include "vmm/page_fault.h"
#include "vmm/utils/log.h"
#include <string.h>

bool handle_page_fault(uint32_t pid, vpn_t vpn, page_table_t *pt, ram_t *ram)
{
    if (!pt || !ram)
        return false;

    LOG_WARN("[PAGE FAULT HANDLER] Interrupted CPU execution. Resolving fault for PID %u on VPN %u...", pid, vpn);

    pfn_t free_pfn = 0;
    bool allocated = ram_allocate_frame(ram, pid, vpn, &free_pfn);
    if (!allocated)
    {
        LOG_ERROR("[PAGE FAULT HANDLER] Allocation Failed: Out of Memory! (Eviction needed in Phase 7)");
        return false;
    }

    // Zero-fill newly allocated physical frame (Demand Zero Paging)
    paddr_t frame_paddr = (paddr_t)free_pfn * ram->page_size;
    uint8_t zero_buffer[ram->page_size];
    memset(zero_buffer, 0, ram->page_size);
    ram_write(ram, frame_paddr, zero_buffer, ram->page_size);

    // Update Page Table with PFN and set PRESENT flag
    uint8_t flags = PAGE_FLAG_READABLE | PAGE_FLAG_WRITABLE | PAGE_FLAG_PRESENT;
    page_table_map(pt, vpn, free_pfn, flags);

    LOG_INFO("[PAGE FAULT HANDLER] Resolved! Mapped VPN %u -> Physical Frame %u", vpn, free_pfn);
    return true;
}