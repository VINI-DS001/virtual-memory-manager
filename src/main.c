#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vmm/types.h"
#include "vmm/config.h"
#include "vmm/cpu.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/mmu.h"
#include "vmm/replacement.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    LOG_INFO("Starting Virtual Memory Manager Simulator (Phase 7 - Page Replacement Algorithms)...");

    vmm_config_t config;
    vmm_config_init_default(&config);
    config.total_frames = 4; // Small RAM (4 Frames) to force page replacement easily

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    tlb_t tlb;
    tlb_init(&tlb, config.tlb_capacity);

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, REPLACEMENT_FIFO);

    // Demonstration 1: FIFO Eviction
    LOG_INFO("=== DEMO 1: Testing FIFO Replacement Policy ===");
    for (vpn_t vpn = 0; vpn < 6; vpn++)
    {
        vaddr_t vaddr = (vaddr_t)vpn * config.page_size;
        paddr_t paddr = 0;
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);

        if (status == MMU_PAGE_FAULT)
        {
            handle_page_fault(101, vpn, &pt, &ram, &tlb, &repl_mgr);
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    // Demonstration 2: Dynamic Switch to LRU
    LOG_INFO("=== DEMO 2: Dynamic Switch to LRU Policy ===");
    replacement_set_algorithm(&repl_mgr, REPLACEMENT_LRU);
    for (vpn_t vpn = 6; vpn < 9; vpn++)
    {
        vaddr_t vaddr = (vaddr_t)vpn * config.page_size;
        paddr_t paddr = 0;
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);

        if (status == MMU_PAGE_FAULT)
        {
            handle_page_fault(101, vpn, &pt, &ram, &tlb, &repl_mgr);
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    replacement_destroy(&repl_mgr);
    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);

    LOG_INFO("Phase 7 Page Replacement Algorithms completed successfully.");
    return 0;
}