#include <assert.h>
#include "vmm/config.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/mmu.h"
#include "vmm/replacement.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Page Replacement Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);
    config.total_frames = 2; // 2 frames only

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    tlb_t tlb;
    tlb_init(&tlb, 2);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, REPLACEMENT_FIFO);

    // Fill RAM (VPN 0 -> Frame 0, VPN 1 -> Frame 1)
    handle_page_fault(101, 0, &pt, &ram, &tlb, &repl_mgr);
    handle_page_fault(101, 1, &pt, &ram, &tlb, &repl_mgr);

    // Trigger Eviction (FIFO should evict VPN 0)
    pfn_t victim_pfn = 0;
    vpn_t victim_vpn = 0;
    assert(replacement_select_victim(&repl_mgr, &victim_pfn, &victim_vpn) == true);
    assert(victim_vpn == 0); // Oldest page must be chosen

    replacement_destroy(&repl_mgr);
    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);

    LOG_INFO("Page Replacement Sanity Test Passed Successfully!");
    return 0;
}