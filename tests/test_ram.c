#include <assert.h>
#include <string.h>
#include "vmm/ram.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Physical RAM Subsystem Sanity Test...");

    vmm_config_t config;
    vmm_config_init_default(&config);

    ram_t ram;
    bool success = ram_init(&ram, &config);
    assert(success == true);
    assert(ram.free_frames == 16);

    // Allocate frame test
    pfn_t allocated_pfn = 0;
    bool alloc_ok = ram_allocate_frame(&ram, 100, 5, &allocated_pfn);
    assert(alloc_ok == true);
    assert(allocated_pfn == 0);
    assert(ram.free_frames == 15);

    // Read/Write RAM test
    char test_data[] = "VMM Physical RAM Test!";
    paddr_t physical_addr = 0x0010; // Offset 16 inside Frame 0

    assert(ram_write(&ram, physical_addr, test_data, sizeof(test_data)) == true);

    char read_buffer[64] = {0};
    assert(ram_read(&ram, physical_addr, read_buffer, sizeof(test_data)) == true);
    assert(strcmp(test_data, read_buffer) == 0);

    // Free frame test
    ram_free_frame(&ram, allocated_pfn);
    assert(ram.free_frames == 16);

    ram_destroy(&ram);
    LOG_INFO("RAM Sanity Test Passed Successfully!");
    return 0;
}