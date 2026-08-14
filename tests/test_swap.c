#include <assert.h>
#include "vmm/config.h"
#include "vmm/ram.h"
#include "vmm/swap.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/replacement.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Swap System Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);
    config.total_frames = 1; // 1 Frame RAM forces Swap on 2nd page access

    ram_t ram;
    ram_init(&ram, &config);

    swap_disk_t swap;
    swap_init(&swap, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    tlb_t tlb;
    tlb_init(&tlb, 2);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, REPLACEMENT_FIFO);

    // Page Fault on VPN 0 (Loads into RAM Frame 0)
    bool resolved = handle_page_fault(101, 0, &pt, &ram, &tlb, &repl_mgr, &swap);
    assert(resolved == true);

    // Page Fault on VPN 1 (Evicts VPN 0 -> Swap Out)
    resolved = handle_page_fault(101, 1, &pt, &ram, &tlb, &repl_mgr, &swap);
    assert(resolved == true);
    assert(swap.swap_writes == 1);

    // Page Fault on VPN 0 again (Loads from Disk -> Swap In)
    resolved = handle_page_fault(101, 0, &pt, &ram, &tlb, &repl_mgr, &swap);
    assert(resolved == true);
    assert(swap.swap_reads == 1);

    replacement_destroy(&repl_mgr);
    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    swap_destroy(&swap);
    ram_destroy(&ram);

    LOG_INFO("Swap System Sanity Test Passed Successfully!");
    return 0;
}