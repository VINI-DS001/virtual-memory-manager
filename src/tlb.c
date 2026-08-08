#include "vmm/tlb.h"
#include "vmm/utils/log.h"
#include <stdlib.h>

bool tlb_init(tlb_t *tlb, uint32_t capacity)
{
    if (!tlb || capacity == 0)
        return false;

    tlb->capacity = capacity;
    tlb->access_clock = 0;
    tlb->hits = 0;
    tlb->misses = 0;

    tlb->entries = (tlb_entry_t *)calloc(capacity, sizeof(tlb_entry_t));
    if (!tlb->entries)
    {
        LOG_ERROR("Failed to allocate TLB cache buffer.");
        return false;
    }

    tlb_flush(tlb);
    LOG_INFO("TLB Hardware Cache initialized with %u slots.", capacity);
    return true;
}

void tlb_destroy(tlb_t *tlb)
{
    if (!tlb)
        return;
    if (tlb->entries)
        free(tlb->entries);
    tlb->entries = NULL;
    LOG_INFO("TLB Cache resources released.");
}

bool tlb_lookup(tlb_t *tlb, vpn_t vpn, pfn_t *out_pfn)
{
    if (!tlb || !out_pfn)
        return false;

    tlb->access_clock++;

    for (uint32_t i = 0; i < tlb->capacity; i++)
    {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn)
        {
            tlb->hits++;
            tlb->entries[i].last_used = tlb->access_clock; // Update LRU timestamp
            *out_pfn = tlb->entries[i].pfn;
            LOG_DEBUG("TLB HIT: VPN %u -> PFN %u (Slot %u)", vpn, *out_pfn, i);
            return true;
        }
    }

    tlb->misses++;
    LOG_DEBUG("TLB MISS: VPN %u not found in cache", vpn);
    return false;
}

void tlb_insert(tlb_t *tlb, vpn_t vpn, pfn_t pfn)
{
    if (!tlb)
        return;

    // 1. If already exists, update entry
    for (uint32_t i = 0; i < tlb->capacity; i++)
    {
        if (tlb->entries[i].valid && tlb->entries[i].vpn == vpn)
        {
            tlb->entries[i].pfn = pfn;
            tlb->entries[i].last_used = tlb->access_clock;
            LOG_DEBUG("TLB UPDATE: VPN %u -> PFN %u (Slot %u)", vpn, pfn, i);
            return;
        }
    }

    // 2. Find empty slot
    for (uint32_t i = 0; i < tlb->capacity; i++)
    {
        if (!tlb->entries[i].valid)
        {
            tlb->entries[i].vpn = vpn;
            tlb->entries[i].pfn = pfn;
            tlb->entries[i].valid = true;
            tlb->entries[i].last_used = tlb->access_clock;
            LOG_DEBUG("TLB INSERT: VPN %u -> PFN %u (Slot %u)", vpn, pfn, i);
            return;
        }
    }

    // 3. LRU Eviction Strategy (if full)
    uint32_t lru_index = 0;
    uint64_t oldest_access = tlb->entries[0].last_used;

    for (uint32_t i = 1; i < tlb->capacity; i++)
    {
        if (tlb->entries[i].last_used < oldest_access)
        {
            oldest_access = tlb->entries[i].last_used;
            lru_index = i;
        }
    }

    LOG_DEBUG("TLB EVICT (LRU): Replacing VPN %u with VPN %u in Slot %u",
              tlb->entries[lru_index].vpn, vpn, lru_index);

    tlb->entries[lru_index].vpn = vpn;
    tlb->entries[lru_index].pfn = pfn;
    tlb->entries[lru_index].valid = true;
    tlb->entries[lru_index].last_used = tlb->access_clock;
}

void tlb_flush(tlb_t *tlb)
{
    if (!tlb)
        return;
    for (uint32_t i = 0; i < tlb->capacity; i++)
    {
        tlb->entries[i].valid = false;
        tlb->entries[i].vpn = 0;
        tlb->entries[i].pfn = 0;
        tlb->entries[i].last_used = 0;
    }
    LOG_INFO("TLB Flushed.");
}

float tlb_get_hit_rate(const tlb_t *tlb)
{
    if (!tlb)
        return 0.0f;
    uint64_t total = tlb->hits + tlb->misses;
    if (total == 0)
        return 0.0f;
    return ((float)tlb->hits / (float)total) * 100.0f;
}