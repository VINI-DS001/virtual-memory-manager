#include "vmm/workload.h"
#include "vmm/utils/log.h"
#include <stdlib.h>
#include <string.h>

void workload_generate(workload_t *wl, workload_type_t type, size_t total_accesses, uint32_t max_vpn, uint32_t page_size)
{
    if (!wl || total_accesses == 0 || max_vpn == 0 || page_size == 0)
        return;

    wl->type = type;
    wl->total_accesses = total_accesses;
    wl->max_vpn = max_vpn;
    wl->access_pattern = malloc(total_accesses * sizeof(*wl->access_pattern));

    if (!wl->access_pattern)
    {
        LOG_ERROR("Failed to allocate memory for workload trace!");
        return;
    }

    srand(42); // Fixed seed for reproducible benchmarks

    uint32_t working_set_size = max_vpn > 10 ? max_vpn / 5 : 2; // 20% hot set

    for (size_t i = 0; i < total_accesses; i++)
    {
        vpn_t vpn = 0;
        offset_t offset = (offset_t)(rand() % (int)page_size);

        switch (type)
        {
        case WORKLOAD_SEQUENTIAL:
            vpn = (vpn_t)(i % max_vpn);
            break;

        case WORKLOAD_RANDOM:
            vpn = (vpn_t)(rand() % (int)max_vpn);
            break;

        case WORKLOAD_TEMPORAL_LOCALITY:
            // 80% of accesses target 20% hot working set pages
            if ((rand() % 100) < 80)
            {
                vpn = (vpn_t)(rand() % (int)working_set_size);
            }
            else
            {
                vpn = (vpn_t)(working_set_size + (uint32_t)(rand() % (int)(max_vpn - working_set_size)));
            }
            break;

        case WORKLOAD_SPATIAL_LOCALITY:
            // Cluster accesses around recent pages with small stride offsets
            if (i > 0 && (rand() % 100) < 85)
            {
                vpn_t prev_vpn = (vpn_t)(wl->access_pattern[i - 1] / page_size);
                int stride = (rand() % 3) - 1; // -1, 0, or +1 page offset
                int target = (int)prev_vpn + stride;
                if (target < 0)
                    target = 0;
                if ((uint32_t)target >= max_vpn)
                    target = (int)(max_vpn - 1);
                vpn = (vpn_t)target;
            }
            else
            {
                vpn = (vpn_t)(rand() % (int)max_vpn);
            }
            break;

        case WORKLOAD_THRASHING:
            // Rapidly cycle across a active pages range wider than physical RAM frames
            vpn = (vpn_t)(i % max_vpn);
            break;
        }

        wl->access_pattern[i] = ((vaddr_t)vpn * page_size) + offset;
    }
}

void workload_destroy(workload_t *wl)
{
    if (!wl)
        return;
    if (wl->access_pattern)
    {
        free(wl->access_pattern);
        wl->access_pattern = NULL;
    }
}

const char *workload_type_to_string(workload_type_t type)
{
    switch (type)
    {
    case WORKLOAD_SEQUENTIAL:
        return "Sequential";
    case WORKLOAD_RANDOM:
        return "Random";
    case WORKLOAD_TEMPORAL_LOCALITY:
        return "TemporalLocality";
    case WORKLOAD_SPATIAL_LOCALITY:
        return "SpatialLocality";
    case WORKLOAD_THRASHING:
        return "Thrashing";
    default:
        return "Unknown";
    }
}