#include "vmm/cpu.h"
#include "vmm/utils/log.h"
#include <stdlib.h>

void cpu_init(cpu_t *cpu, uint32_t process_id, vaddr_t address_space_max)
{
    if (!cpu)
        return;

    cpu->process_id = process_id;
    cpu->current_pc = 0x0000;
    cpu->clock_ticks = 0;
    cpu->pattern = PATTERN_SEQUENTIAL;
    cpu->address_space_max = (address_space_max > 0) ? address_space_max : 0x100000; // Default 1MB

    LOG_INFO("CPU Initialized for PID %u with Address Space [0x0 - 0x%llX]",
             cpu->process_id, (unsigned long long)cpu->address_space_max);
}

void cpu_set_pattern(cpu_t *cpu, access_pattern_t pattern)
{
    if (!cpu)
        return;
    cpu->pattern = pattern;
    LOG_INFO("CPU PID %u workload pattern set to %d", cpu->process_id, pattern);
}

mem_access_req_t cpu_fetch_next_access(cpu_t *cpu)
{
    mem_access_req_t req;
    req.process_id = cpu->process_id;
    req.timestamp = ++cpu->clock_ticks;

    // Default operation distribution: 60% Read, 30% Write, 10% Execute
    int op_rnd = rand() % 100;
    if (op_rnd < 60)
    {
        req.op_type = MEM_OP_READ;
    }
    else if (op_rnd < 90)
    {
        req.op_type = MEM_OP_WRITE;
    }
    else
    {
        req.op_type = MEM_OP_EXECUTE;
    }

    switch (cpu->pattern)
    {
    case PATTERN_SEQUENTIAL:
        req.virtual_address = cpu->current_pc;
        // Advance PC by 4 to 64 bytes
        cpu->current_pc += (vaddr_t)(4 + (rand() % 60));
        if (cpu->current_pc >= cpu->address_space_max)
        {
            cpu->current_pc = 0; // Wrap around
        }
        break;

    case PATTERN_RANDOM:
    {
        uint64_t rnd_val = ((uint64_t)(unsigned int)rand() << 16) | (uint64_t)(unsigned int)rand();
        req.virtual_address = rnd_val % cpu->address_space_max;
        break;
    }

    case PATTERN_LOCALITY:
        // 80% chance to access within the same 4KB window, 20% jump
        if ((rand() % 100) < 80)
        {
            vaddr_t base_page = cpu->current_pc & ~0xFFFULL; // Align to 4KB page
            vaddr_t offset = (vaddr_t)(rand() % 4096);
            req.virtual_address = base_page + offset;
        }
        else
        {
            uint64_t rnd_val = ((uint64_t)(unsigned int)rand() << 16) | (uint64_t)(unsigned int)rand();
            cpu->current_pc = rnd_val % cpu->address_space_max;
            req.virtual_address = cpu->current_pc;
        }
        break;

    case PATTERN_WORKING_SET:
        // Restrict accesses to a working set of 4 specific pages (16KB total)
        {
            uint32_t ws_page = (uint32_t)(rand() % 4);
            vaddr_t base_page = ws_page * 4096;
            vaddr_t offset = (vaddr_t)(rand() % 4096);
            req.virtual_address = base_page + offset;
        }
        break;
    }

    return req;
}