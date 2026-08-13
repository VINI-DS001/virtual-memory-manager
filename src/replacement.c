#include "vmm/replacement.h"
#include "vmm/utils/log.h"
#include <stdlib.h>
#include <string.h>

bool replacement_init(replacement_manager_t *mgr, uint32_t total_frames, replacement_algo_t default_algo)
{
    if (!mgr || total_frames == 0)
        return false;

    mgr->total_frames = total_frames;
    mgr->current_algo = default_algo;
    mgr->global_clock = 0;
    mgr->clock_hand = 0;

    mgr->frames_meta = (frame_meta_t *)calloc(total_frames, sizeof(frame_meta_t));
    if (!mgr->frames_meta)
    {
        LOG_ERROR("Failed to allocate replacement manager frame metadata.");
        return false;
    }

    LOG_INFO("Replacement Manager initialized (%u Frames, Policy: %s)",
             total_frames, replacement_algo_to_string(default_algo));
    return true;
}

void replacement_destroy(replacement_manager_t *mgr)
{
    if (!mgr)
        return;
    if (mgr->frames_meta)
        free(mgr->frames_meta);
    mgr->frames_meta = NULL;
    LOG_INFO("Replacement Manager resources released.");
}

void replacement_set_algorithm(replacement_manager_t *mgr, replacement_algo_t algo)
{
    if (!mgr)
        return;
    mgr->current_algo = algo;
    LOG_INFO("Switched Page Replacement Algorithm to: %s", replacement_algo_to_string(algo));
}

void replacement_register_access(replacement_manager_t *mgr, pfn_t pfn)
{
    if (!mgr || pfn >= mgr->total_frames)
        return;

    mgr->global_clock++;
    frame_meta_t *meta = &mgr->frames_meta[pfn];

    if (meta->valid)
    {
        meta->last_access = mgr->global_clock;
        meta->access_count++;
        meta->reference_bit = true;
    }
}

void replacement_register_allocation(replacement_manager_t *mgr, pfn_t pfn, vpn_t vpn)
{
    if (!mgr || pfn >= mgr->total_frames)
        return;

    mgr->global_clock++;
    frame_meta_t *meta = &mgr->frames_meta[pfn];

    meta->vpn = vpn;
    meta->pfn = pfn;
    meta->valid = true;
    meta->load_time = mgr->global_clock;
    meta->last_access = mgr->global_clock;
    meta->access_count = 1;
    meta->reference_bit = true;
}

bool replacement_select_victim(replacement_manager_t *mgr,
                               pfn_t *out_pfn,
                               vpn_t *out_vpn)
{
    if (!mgr || !out_pfn || !out_vpn || mgr->total_frames == 0)
        return false;

    pfn_t victim_idx = 0;
    bool victim_found = false;

    switch (mgr->current_algo)
    {
    case REPLACEMENT_FIFO:
    {
        uint64_t oldest_time = UINT64_MAX;

        for (uint32_t i = 0; i < mgr->total_frames; i++)
        {
            if (mgr->frames_meta[i].valid &&
                mgr->frames_meta[i].load_time < oldest_time)
            {
                oldest_time = mgr->frames_meta[i].load_time;
                victim_idx = i;
                victim_found = true;
            }
        }
        break;
    }

    case REPLACEMENT_LRU:
    {
        uint64_t least_recent = UINT64_MAX;

        for (uint32_t i = 0; i < mgr->total_frames; i++)
        {
            if (mgr->frames_meta[i].valid &&
                mgr->frames_meta[i].last_access < least_recent)
            {
                least_recent = mgr->frames_meta[i].last_access;
                victim_idx = i;
                victim_found = true;
            }
        }
        break;
    }

    case REPLACEMENT_CLOCK:
    case REPLACEMENT_SECOND_CHANCE:
    {
        uint32_t scanned = 0;
        uint32_t max_scans = mgr->total_frames * 2U;

        while (scanned < max_scans)
        {
            uint32_t ptr = mgr->clock_hand;

            mgr->clock_hand =
                (mgr->clock_hand + 1U) % mgr->total_frames;

            if (mgr->frames_meta[ptr].valid)
            {
                if (!mgr->frames_meta[ptr].reference_bit)
                {
                    victim_idx = ptr;
                    victim_found = true;
                    break;
                }

                /* Give the page a second chance. */
                mgr->frames_meta[ptr].reference_bit = false;
            }

            scanned++;
        }
        break;
    }

    case REPLACEMENT_LFU:
    {
        uint64_t min_freq = UINT64_MAX;

        for (uint32_t i = 0; i < mgr->total_frames; i++)
        {
            if (mgr->frames_meta[i].valid &&
                mgr->frames_meta[i].access_count < min_freq)
            {
                min_freq = mgr->frames_meta[i].access_count;
                victim_idx = i;
                victim_found = true;
            }
        }
        break;
    }

    case REPLACEMENT_RANDOM:
    default:
    {
        /*
         * rand() returns int, while total_frames is uint32_t.
         * Explicitly convert rand() to uint32_t to avoid
         * -Wsign-conversion.
         */
        victim_idx = (uint32_t)rand() % mgr->total_frames;
        victim_found = mgr->frames_meta[victim_idx].valid;
        break;
    }
    }

    if (!victim_found)
    {
        LOG_WARN("[REPLACEMENT] No valid frame available for replacement.");
        return false;
    }

    *out_pfn = mgr->frames_meta[victim_idx].pfn;
    *out_vpn = mgr->frames_meta[victim_idx].vpn;

    LOG_WARN("[REPLACEMENT] Policy %s selected Victim Frame %u (VPN %u)",
             replacement_algo_to_string(mgr->current_algo),
             *out_pfn,
             *out_vpn);

    return true;
}

const char *replacement_algo_to_string(replacement_algo_t algo)
{
    switch (algo)
    {
    case REPLACEMENT_FIFO:
        return "FIFO";
    case REPLACEMENT_LRU:
        return "LRU";
    case REPLACEMENT_CLOCK:
        return "CLOCK";
    case REPLACEMENT_SECOND_CHANCE:
        return "SECOND_CHANCE";
    case REPLACEMENT_LFU:
        return "LFU";
    case REPLACEMENT_RANDOM:
        return "RANDOM";
    default:
        return "UNKNOWN";
    }
}