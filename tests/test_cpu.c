#include <assert.h>
#include "vmm/cpu.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing CPU Workload Sanity Test...");

    cpu_t cpu;
    cpu_init(&cpu, 1, 0x10000);
    cpu_set_pattern(&cpu, PATTERN_SEQUENTIAL);

    mem_access_req_t req1 = cpu_fetch_next_access(&cpu);
    mem_access_req_t req2 = cpu_fetch_next_access(&cpu);

    assert(req1.timestamp == 1);
    assert(req2.timestamp == 2);
    assert(req1.process_id == 1);
    assert(req2.virtual_address > req1.virtual_address); // Sequential condition

    LOG_INFO("CPU Sanity Test Passed Successfully!");
    return 0;
}