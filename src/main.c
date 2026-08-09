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
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    LOG_INFO("Starting Virtual Memory Manager Simulator (Phase 6 - Page Fault Handler)...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    uint32_t max_pages = config.ram_size / config.page_size;
    page_table_init(&pt, max_pages);

    // Page table starts EMPTY to test Demand Paging!

    tlb_t tlb;
    if (!tlb_init(&tlb, config.tlb_capacity))
    {
        LOG_ERROR("Failed to initialize TLB.");
        page_table_destroy(&pt);
        ram_destroy(&ram);
        return 1;
    }

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    cpu_t cpu;
    cpu_init(&cpu, 101, config.ram_size);
    cpu_set_pattern(&cpu, PATTERN_SEQUENTIAL);

    LOG_INFO("Simulating Demand Paging with CPU accesses...");
    for (int i = 0; i < 6; i++)
    {
        mem_access_req_t req = cpu_fetch_next_access(&cpu);
        paddr_t paddr = 0;

        page_flags_t req_flag = (req.op_type == MEM_OP_WRITE) ? PAGE_FLAG_WRITABLE : PAGE_FLAG_READABLE;
        mmu_status_t status = mmu_translate(&mmu, req.virtual_address, req_flag, &paddr);

        if (status == MMU_PAGE_FAULT)
        {
            vpn_t fault_vpn = mmu_extract_vpn(req.virtual_address, config.page_offset_bits);
            LOG_WARN("MMU Page Fault on Virtual Address 0x%04llX (VPN %u)",
                     (unsigned long long)req.virtual_address, fault_vpn);

            // Kernel handles Page Fault dynamically by allocating RAM frame
            if (handle_page_fault(req.process_id, fault_vpn, &pt, &ram))
            {
                // Retry MMU translation after page fault resolution
                status = mmu_translate(&mmu, req.virtual_address, req_flag, &paddr);
            }
        }

        if (status == MMU_SUCCESS)
        {
            LOG_INFO("Access OK | Virtual: 0x%04llX -> Physical: 0x%04llX",
                     (unsigned long long)req.virtual_address, (unsigned long long)paddr);
        }
        else
        {
            LOG_ERROR("Fatal memory access failure for address 0x%04llX",
                      (unsigned long long)req.virtual_address);
        }
    }

    LOG_INFO("TLB Statistics | Hits: %llu | Misses: %llu | Hit Rate: %.2f%%",
             (unsigned long long)tlb.hits,
             (unsigned long long)tlb.misses,
             (double)tlb_get_hit_rate(&tlb));

    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);
    LOG_INFO("Phase 6 Page Fault Handler completed successfully.");
    return 0;
}