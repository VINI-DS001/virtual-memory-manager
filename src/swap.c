#include "vmm/swap.h"
#include "vmm/utils/log.h"
#include <stdlib.h>
#include <string.h>

bool swap_init(swap_disk_t *swap, const vmm_config_t *config)
{
    if (!swap || !config)
        return false;

    // Atenção especial: Usando total_swap_pages conforme seu feedback!
    swap->total_pages = config->total_swap_pages;
    swap->page_size = config->page_size;
    swap->swap_writes = 0;
    swap->swap_reads = 0;

    size_t total_bytes = (size_t)swap->total_pages * swap->page_size;
    swap->storage = (uint8_t *)calloc(1, total_bytes);
    swap->occupied = (bool *)calloc(swap->total_pages, sizeof(bool));

    if (!swap->storage || !swap->occupied)
    {
        LOG_ERROR("Failed to allocate Swap Disk memory.");
        swap_destroy(swap);
        return false;
    }

    LOG_INFO("Swap Disk initialized: %u Pages (%zu KB Total)",
             swap->total_pages, total_bytes / 1024);
    return true;
}

void swap_destroy(swap_disk_t *swap)
{
    if (!swap)
        return;
    if (swap->storage)
        free(swap->storage);
    if (swap->occupied)
        free(swap->occupied);
    swap->storage = NULL;
    swap->occupied = NULL;
    LOG_INFO("Swap Disk resources released.");
}

int32_t swap_allocate_slot(swap_disk_t *swap)
{
    if (!swap)
        return -1;
    for (uint32_t i = 0; i < swap->total_pages; i++)
    {
        if (!swap->occupied[i])
        {
            swap->occupied[i] = true;
            return (int32_t)i;
        }
    }
    LOG_ERROR("[THRASHING / CRITICAL] Swap Space Out of Disk!");
    return -1;
}

void swap_free_slot(swap_disk_t *swap, uint32_t slot_id)
{
    if (!swap || slot_id >= swap->total_pages)
        return;
    swap->occupied[slot_id] = false;
}

bool swap_write_page(swap_disk_t *swap, uint32_t slot_id, const uint8_t *buffer)
{
    if (!swap || !buffer || slot_id >= swap->total_pages)
        return false;

    size_t offset = (size_t)slot_id * swap->page_size;
    memcpy(swap->storage + offset, buffer, swap->page_size);
    swap->swap_writes++;
    LOG_DEBUG("Swap Disk: Swapped OUT -> Written Page to Disk Slot %u", slot_id);
    return true;
}

bool swap_read_page(swap_disk_t *swap, uint32_t slot_id, uint8_t *buffer)
{
    if (!swap || !buffer || slot_id >= swap->total_pages)
        return false;

    size_t offset = (size_t)slot_id * swap->page_size;
    memcpy(buffer, swap->storage + offset, swap->page_size);
    swap->swap_reads++;
    LOG_DEBUG("Swap Disk: Swapped IN -> Read Page from Disk Slot %u", slot_id);
    return true;
}