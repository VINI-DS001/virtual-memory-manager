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

void mmu_init(mmu_t *mmu, ram_t *ram, page_table_t *pt, uint32_t page_offset_bits)
{
    if (!mmu)
        return;

    mmu->ram = ram;
    mmu->page_table = pt;
    mmu->page_offset_bits = page_offset_bits;
    mmu->total_translations = 0;
    mmu->page_faults = 0;

    LOG_INFO("MMU Hardware Initialized (%u bits offset shift).", page_offset_bits);
}

mmu_status_t mmu_translate(mmu_t *mmu, vaddr_t vaddr, page_flags_t req_flag, paddr_t *out_paddr)
{
    if (!mmu || !out_paddr || !mmu->page_table)
        return MMU_INVALID_ADDRESS;

    mmu->total_translations++;

    vpn_t vpn = mmu_extract_vpn(vaddr, mmu->page_offset_bits);
    offset_t offset = mmu_extract_offset(vaddr, mmu->page_offset_bits);

    page_table_entry_t entry;
    if (!page_table_lookup(mmu->page_table, vpn, &entry))
    {
        LOG_WARN("MMU Address Translation Error: Out-Of-Bounds VPN %u", vpn);
        return MMU_INVALID_ADDRESS;
    }

    // Check if Page is Resident in Memory
    if (!(entry.flags & PAGE_FLAG_PRESENT))
    {
        mmu->page_faults++;
        LOG_WARN("MMU Page Fault: VPN %u not present in Physical RAM!", vpn);
        return MMU_PAGE_FAULT;
    }

    // Check Permission Flags
    if (req_flag != PAGE_FLAG_NONE && !(entry.flags & req_flag))
    {
        LOG_WARN("MMU Protection Fault: Access 0x%02X denied for VPN %u (Flags: 0x%02X)",
                 req_flag, vpn, entry.flags);
        return MMU_PROTECTION_FAULT;
    }

    // Update Access and Dirty Flags in Page Table Entry
    uint8_t *flags_ptr = &mmu->page_table->entries[vpn].flags;
    *flags_ptr |= PAGE_FLAG_ACCESSED;
    if (req_flag & PAGE_FLAG_WRITABLE)
    {
        *flags_ptr |= PAGE_FLAG_DIRTY;
    }

    // Combine Physical Frame Number with Offset
    *out_paddr = ((paddr_t)entry.pfn << mmu->page_offset_bits) | (paddr_t)offset;

    LOG_DEBUG("MMU Translation: Virtual 0x%08llX (VPN %u, Offset %u) -> Physical 0x%08llX (PFN %u)",
              (unsigned long long)vaddr, vpn, offset, (unsigned long long)*out_paddr, entry.pfn);

    return MMU_SUCCESS;
}