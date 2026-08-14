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
#include "vmm/swap.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    LOG_INFO("Starting Virtual Memory Manager Simulator (Phase 8 - Complete Virtual Memory System with Swap)...");

    vmm_config_t config;
    vmm_config_init_default(&config);
    config.total_frames = 2; // Very small RAM (2 Frames) to demonstrate Swap immediately

    ram_t ram;
    ram_init(&ram, &config);

    swap_disk_t swap;
    swap_init(&swap, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    tlb_t tlb;
    tlb_init(&tlb, config.tlb_capacity);

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, REPLACEMENT_FIFO);

    LOG_INFO("=== DEMO 1: Filling RAM (VPN 0 and VPN 1) ===");
    for (vpn_t vpn = 0; vpn < 2; vpn++)
    {
        vaddr_t vaddr = (vaddr_t)vpn * config.page_size;
        paddr_t paddr = 0;
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        if (status == MMU_PAGE_FAULT)
        {
            handle_page_fault(101, vpn, &pt, &ram, &tlb, &repl_mgr, &swap);
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    LOG_INFO("=== DEMO 2: Accessing VPN 2 (Triggers SWAP OUT of VPN 0) ===");
    {
        vaddr_t vaddr = (vaddr_t)2 * config.page_size;
        paddr_t paddr = 0;
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        if (status == MMU_PAGE_FAULT)
        {
            handle_page_fault(101, 2, &pt, &ram, &tlb, &repl_mgr, &swap);
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    LOG_INFO("=== DEMO 3: Accessing VPN 0 again (Triggers SWAP IN from Disk!) ===");
    {
        vaddr_t vaddr = (vaddr_t)0 * config.page_size;
        paddr_t paddr = 0;
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        if (status == MMU_PAGE_FAULT)
        {
            handle_page_fault(101, 0, &pt, &ram, &tlb, &repl_mgr, &swap);
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    LOG_INFO("Swap Statistics | Writes (Swap Out): %llu | Reads (Swap In): %llu",
             (unsigned long long)swap.swap_writes,
             (unsigned long long)swap.swap_reads);

    replacement_destroy(&repl_mgr);
    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    swap_destroy(&swap);
    ram_destroy(&ram);

    LOG_INFO("Phase 8 Complete Virtual Memory System completed successfully.");
    return 0;
}