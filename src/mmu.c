#include "vmm/mmu.h"
#include "vmm/utils/log.h"

vpn_t mmu_extract_vpn(vaddr_t vaddr, uint32_t page_offset_bits)
{
    return (vpn_t)(vaddr >> page_offset_bits);
}

offset_t mmu_extract_offset(vaddr_t vaddr, uint32_t page_offset_bits)
{
    vaddr_t mask = (1ULL << page_offset_bits) - 1ULL;
    return (offset_t)(vaddr & mask);
}

void mmu_init(mmu_t *mmu, ram_t *ram, page_table_t *pt, tlb_t *tlb, uint32_t page_offset_bits)
{
    if (!mmu)
        return;
    mmu->ram = ram;
    mmu->page_table = pt;
    mmu->tlb = tlb;
    mmu->page_offset_bits = page_offset_bits;
    mmu->total_translations = 0;
    mmu->page_faults = 0;
    LOG_INFO("MMU Hardware Initialized with TLB support.");
}

mmu_status_t mmu_translate(mmu_t *mmu, vaddr_t vaddr, page_flags_t req_flag, paddr_t *out_paddr)
{
    if (!mmu || !out_paddr || !mmu->page_table)
        return MMU_INVALID_ADDRESS;

    mmu->total_translations++;
    vpn_t vpn = mmu_extract_vpn(vaddr, mmu->page_offset_bits);
    offset_t offset = mmu_extract_offset(vaddr, mmu->page_offset_bits);
    pfn_t target_pfn = 0;

    // STEP 1: Fast path — Search TLB Cache first
    if (mmu->tlb && tlb_lookup(mmu->tlb, vpn, &target_pfn))
    {
        *out_paddr = ((paddr_t)target_pfn << mmu->page_offset_bits) | (paddr_t)offset;
        return MMU_SUCCESS;
    }

    // STEP 2: Slow path — TLB Miss, query Page Table
    page_table_entry_t entry;
    if (!page_table_lookup(mmu->page_table, vpn, &entry))
    {
        return MMU_INVALID_ADDRESS;
    }

    if (!(entry.flags & PAGE_FLAG_PRESENT))
    {
        mmu->page_faults++;
        return MMU_PAGE_FAULT;
    }

    if (req_flag != PAGE_FLAG_NONE && !(entry.flags & req_flag))
    {
        return MMU_PROTECTION_FAULT;
    }

    target_pfn = entry.pfn;

    // STEP 3: Insert new mapping into TLB Cache for future accesses
    if (mmu->tlb)
    {
        tlb_insert(mmu->tlb, vpn, target_pfn);
    }

    *out_paddr = ((paddr_t)target_pfn << mmu->page_offset_bits) | (paddr_t)offset;
    return MMU_SUCCESS;
}