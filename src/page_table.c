#include "vmm/page_table.h"
#include "vmm/utils/log.h"
#include <stdlib.h>

bool page_table_init(page_table_t *pt, uint32_t num_pages)
{
    if (!pt || num_pages == 0)
        return false;

    pt->total_pages = num_pages;
    pt->entries = (page_table_entry_t *)calloc(num_pages, sizeof(page_table_entry_t));
    if (!pt->entries)
    {
        LOG_ERROR("Failed to allocate Page Table with %u entries.", num_pages);
        return false;
    }

    LOG_INFO("Page Table initialized with %u entries.", num_pages);
    return true;
}

void page_table_destroy(page_table_t *pt)
{
    if (!pt)
        return;
    if (pt->entries)
        free(pt->entries);
    pt->entries = NULL;
    LOG_INFO("Page Table resources released.");
}

bool page_table_map(page_table_t *pt, vpn_t vpn, pfn_t pfn, uint8_t flags)
{
    if (!pt || vpn >= pt->total_pages)
    {
        LOG_ERROR("Page Table Map failed: VPN %u out of bounds.", vpn);
        return false;
    }

    pt->entries[vpn].pfn = pfn;
    pt->entries[vpn].flags = flags | PAGE_FLAG_PRESENT;

    LOG_DEBUG("Page Table: Mapped VPN %u -> PFN %u (Flags: 0x%02X)", vpn, pfn, flags);
    return true;
}

bool page_table_unmap(page_table_t *pt, vpn_t vpn)
{
    if (!pt || vpn >= pt->total_pages)
    {
        LOG_ERROR("Page Table Unmap failed: VPN %u out of bounds.", vpn);
        return false;
    }

    pt->entries[vpn].pfn = 0;
    pt->entries[vpn].swap_page_id = 0;
    pt->entries[vpn].flags = 0;

    LOG_DEBUG("Page Table: Unmapped VPN %u", vpn);
    return true;
}

bool page_table_lookup(const page_table_t *pt, vpn_t vpn, page_table_entry_t *out_entry)
{
    if (!pt || !out_entry || vpn >= pt->total_pages)
        return false;
    *out_entry = pt->entries[vpn];
    return true;
}

bool page_table_mark_swapped(page_table_t *pt, vpn_t vpn, uint32_t swap_page_id)
{
    if (!pt || vpn >= pt->total_pages)
        return false;
    pt->entries[vpn].flags &= (uint8_t)~PAGE_FLAG_PRESENT;
    pt->entries[vpn].flags |= PAGE_FLAG_SWAPPED;
    pt->entries[vpn].swap_page_id = swap_page_id;
    LOG_DEBUG("Page Table: Marked VPN %u as SWAPPED (Disk Slot %u)", vpn, swap_page_id);
    return true;
}