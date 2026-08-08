#include <assert.h>
#include "vmm/config.h"
#include "vmm/ram.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/mmu.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing TLB Cache Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    ram_t ram;
    ram_init(&ram, &config);

    page_table_t pt;
    page_table_init(&pt, 16);
    page_table_map(&pt, 1, 5, PAGE_FLAG_READABLE);

    tlb_t tlb;
    tlb_init(&tlb, 2); // Small TLB (2 slots) to test LRU eviction easily

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    paddr_t paddr = 0;

    // 1st access -> TLB Miss (populates TLB)
    assert(mmu_translate(&mmu, 0x1010, PAGE_FLAG_READABLE, &paddr) == MMU_SUCCESS);
    assert(tlb.hits == 0 && tlb.misses == 1);

    // 2nd access to same page -> TLB Hit!
    assert(mmu_translate(&mmu, 0x1020, PAGE_FLAG_READABLE, &paddr) == MMU_SUCCESS);
    assert(tlb.hits == 1 && tlb.misses == 1);
    assert(tlb_get_hit_rate(&tlb) == 50.0f);

    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    ram_destroy(&ram);

    LOG_INFO("TLB Cache Test Passed Successfully!");
    return 0;
}