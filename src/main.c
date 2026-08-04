/**
 * @file main.c
 * @brief Entry point for Virtual Memory Manager Simulator.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vmm/types.h"
#include "vmm/config.h"
#include "vmm/cpu.h"
#include "vmm/utils/log.h"

static const char *op_type_str(mem_op_type_t op)
{
    switch (op)
    {
    case MEM_OP_READ:
        return "READ";
    case MEM_OP_WRITE:
        return "WRITE";
    case MEM_OP_EXECUTE:
        return "EXEC";
    default:
        return "UNK";
    }
}

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    LOG_INFO("Starting Virtual Memory Manager Simulator...");

    vmm_config_t config;
    if (!vmm_config_init_default(&config))
    {
        LOG_ERROR("Failed to initialize VMM configuration.");
        return 1;
    }

    cpu_t cpu;
    cpu_init(&cpu, 101, 0x10000); // Process 101, 64KB Virtual Space
    cpu_set_pattern(&cpu, PATTERN_LOCALITY);

    LOG_INFO("Simulating 10 Virtual Memory Accesses from CPU...");
    for (int i = 0; i < 10; i++)
    {
        mem_access_req_t req = cpu_fetch_next_access(&cpu);
        LOG_INFO("Tick #%llu | PID %u | Op: %-5s | Virtual Address: 0x%08llX",
                 (unsigned long long)req.timestamp,
                 req.process_id,
                 op_type_str(req.op_type),
                 (unsigned long long)req.virtual_address);
    }

    LOG_INFO("Phase 2 CPU simulation completed successfully.");
    return 0;
}