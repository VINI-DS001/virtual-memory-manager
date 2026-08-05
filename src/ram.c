#include "vmm/ram.h"
#include "vmm/utils/log.h"
#include <stdlib.h>
#include <string.h>

bool ram_init(ram_t *ram, const vmm_config_t *config)
{
    if (!ram || !config)
        return false;

    ram->page_size = config->page_size;
    ram->total_frames = config->total_frames;
    ram->free_frames = config->total_frames;

    // Allocate contiguous raw memory simulating physical RAM
    size_t total_ram_bytes = (size_t)config->ram_size;
    ram->memory_buffer = (uint8_t *)calloc(total_ram_bytes, sizeof(uint8_t));
    if (!ram->memory_buffer)
    {
        LOG_ERROR("Failed to allocate RAM buffer of %zu bytes.", total_ram_bytes);
        return false;
    }

    // Allocate metadata structures for each frame
    ram->frames = (frame_t *)calloc(ram->total_frames, sizeof(frame_t));
    if (!ram->frames)
    {
        LOG_ERROR("Failed to allocate frame metadata table.");
        free(ram->memory_buffer);
        return false;
    }

    // Initialize frames metadata
    for (uint32_t i = 0; i < ram->total_frames; i++)
    {
        ram->frames[i].pfn = i;
        ram->frames[i].is_free = true;
        ram->frames[i].allocated_pid = 0;
        ram->frames[i].mapped_vpn = 0;
    }

    LOG_INFO("Physical RAM initialized: %u KB (%u Frames of %u B)",
             config->ram_size / 1024, ram->total_frames, ram->page_size);
    return true;
}

void ram_destroy(ram_t *ram)
{
    if (!ram)
        return;
    if (ram->memory_buffer)
        free(ram->memory_buffer);
    if (ram->frames)
        free(ram->frames);
    ram->memory_buffer = NULL;
    ram->frames = NULL;
    LOG_INFO("Physical RAM resources released.");
}

bool ram_allocate_frame(ram_t *ram, uint32_t pid, vpn_t vpn, pfn_t *out_pfn)
{
    if (!ram || !out_pfn || ram->free_frames == 0)
        return false;

    // First-fit free frame allocation strategy
    for (uint32_t i = 0; i < ram->total_frames; i++)
    {
        if (ram->frames[i].is_free)
        {
            ram->frames[i].is_free = false;
            ram->frames[i].allocated_pid = pid;
            ram->frames[i].mapped_vpn = vpn;
            ram->free_frames--;

            *out_pfn = i;
            LOG_DEBUG("RAM Frame %u allocated to PID %u (VPN %u). Free frames remaining: %u",
                      i, pid, vpn, ram->free_frames);
            return true;
        }
    }

    return false;
}

void ram_free_frame(ram_t *ram, pfn_t pfn)
{
    if (!ram || pfn >= ram->total_frames)
        return;

    if (!ram->frames[pfn].is_free)
    {
        ram->frames[pfn].is_free = true;
        ram->frames[pfn].allocated_pid = 0;
        ram->frames[pfn].mapped_vpn = 0;
        ram->free_frames++;
        LOG_DEBUG("RAM Frame %u released. Free frames available: %u", pfn, ram->free_frames);
    }
}

bool ram_write(ram_t *ram, paddr_t paddr, const void *src, size_t size)
{
    if (!ram || !src)
        return false;
    if (paddr + size > (paddr_t)(ram->total_frames * ram->page_size))
    {
        LOG_ERROR("Physical RAM write Out-Of-Bounds at address 0x%llX", (unsigned long long)paddr);
        return false;
    }

    memcpy(&ram->memory_buffer[paddr], src, size);
    return true;
}

bool ram_read(const ram_t *ram, paddr_t paddr, void *dest, size_t size)
{
    if (!ram || !dest)
        return false;
    if (paddr + size > (paddr_t)(ram->total_frames * ram->page_size))
    {
        LOG_ERROR("Physical RAM read Out-Of-Bounds at address 0x%llX", (unsigned long long)paddr);
        return false;
    }

    memcpy(dest, &ram->memory_buffer[paddr], size);
    return true;
}