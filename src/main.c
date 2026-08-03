/**
 * @file main.c
 * @brief Entry point for Virtual Memory Manager Simulator.
 */

#include <stdio.h>
#include "vmm/types.h"
#include "vmm/config.h"
#include "vmm/utils/log.h"

int main(int argc, char *argv[])
{
    LOG_INFO("Starting Virtual Memory Manager Simulator...");

    vmm_config_t config;
    if (!vmm_config_init_default(&config))
    {
        LOG_ERROR("Failed to initialize VMM configuration.");
        return 1;
    }

    LOG_INFO("Phase 1 initialization complete. Base architecture successfully loaded.");
    return 0;
}