#include "vmm/config.h"
#include "vmm/utils/log.h"
#include <math.h>

static bool is_power_of_two(uint32_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

bool vmm_config_init_default(vmm_config_t *config)
{
    if (!config)
        return false;

    config->page_size = DEFAULT_PAGE_SIZE_BYTES;
    config->ram_size = DEFAULT_RAM_SIZE_BYTES;
    config->swap_size = DEFAULT_SWAP_SIZE_BYTES;
    config->tlb_capacity = DEFAULT_TLB_ENTRIES;

    if (!is_power_of_two(config->page_size))
    {
        LOG_ERROR("Page size must be a power of two.");
        return false;
    }

    config->total_frames = config->ram_size / config->page_size;
    config->total_swap_pages = config->swap_size / config->page_size;

    // Calculate offset bits: log2(page_size)
    uint32_t temp = config->page_size;
    uint32_t bits = 0;
    while (temp > 1)
    {
        temp >>= 1;
        bits++;
    }
    config->page_offset_bits = bits;

    LOG_INFO("VMM Config initialized: Page Size = %u B (%u bits offset), Frames = %u, Swap Pages = %u",
             config->page_size, config->page_offset_bits, config->total_frames, config->total_swap_pages);

    return true;
}