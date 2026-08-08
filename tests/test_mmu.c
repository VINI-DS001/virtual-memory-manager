#include <assert.h>
#include "vmm/config.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/mmu.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing MMU Translation Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    page_table_init(&pt, 16);

    // Map VPN 2 (Virtual address 0x2000-0x2FFF) to PFN 5 (Physical address 0x5000-0x5FFF)
    page_table_map(&pt, 2, 5, (uint8_t)(PAGE_FLAG_READABLE | PAGE_FLAG_WRITABLE));

    tlb_t tlb;
    assert(tlb_init(&tlb, 4));

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    // Bit extraction checks
    vaddr_t test_vaddr = 0x2123; // VPN = 2, Offset = 0x123 (291)
    assert(mmu_extract_vpn(test_vaddr, 12) == 2);
    assert(mmu_extract_offset(test_vaddr, 12) == 0x123);

    // Address translation check
    paddr_t paddr = 0;
    mmu_status_t status = mmu_translate(&mmu, test_vaddr, PAGE_FLAG_READABLE, &paddr);
    assert(status == MMU_SUCCESS);
    assert(paddr == 0x5123); // PFN 5 + Offset 0x123

    // Page Fault check (Unmapped VPN 3)
    status = mmu_translate(&mmu, 0x3000, PAGE_FLAG_READABLE, &paddr);
    assert(status == MMU_PAGE_FAULT);

    // Protection Fault check (Write operation on Read-Only mapped page)
    page_table_map(&pt, 4, 8, (uint8_t)PAGE_FLAG_READABLE); // Read-only
    status = mmu_translate(&mmu, 0x4000, PAGE_FLAG_WRITABLE, &paddr);
    assert(status == MMU_PROTECTION_FAULT);

    LOG_INFO(
        "TLB Statistics | Hits: %llu | Misses: %llu | Hit Rate: %.2f%%",
        (unsigned long long)tlb.hits,
        (unsigned long long)tlb.misses,
        tlb_get_hit_rate(&tlb));

    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);

    LOG_INFO("MMU Translation Test Passed Successfully!");
    return 0;
}