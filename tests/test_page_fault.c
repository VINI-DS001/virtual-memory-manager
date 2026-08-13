#include <assert.h>
#include "vmm/config.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/mmu.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Page Fault Handler Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    tlb_t tlb;
    tlb_init(&tlb, 4);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, REPLACEMENT_FIFO);

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    vaddr_t vaddr = 0x2010; // VPN 2, Offset 0x10
    paddr_t paddr = 0;

    // Step 1: Initial access must trigger MMU_PAGE_FAULT
    mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
    assert(status == MMU_PAGE_FAULT);

    // Step 2: Kernel resolves Page Fault
    bool resolved = handle_page_fault(101, 2, &pt, &ram, &tlb, &repl_mgr);
    assert(resolved == true);

    // Step 3: Retried access must succeed (Mapped to Frame 0)
    status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
    assert(status == MMU_SUCCESS);
    assert(paddr == 0x0010); // Frame 0 + Offset 0x10

    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);

    LOG_INFO("Page Fault Sanity Test Passed Successfully!");
    return 0;
}