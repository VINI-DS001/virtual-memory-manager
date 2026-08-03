#include <assert.h>
#include "vmm/types.h"
#include "vmm/config.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing Basic Architecture Sanity Test...");

    vmm_config_t config;
    bool success = vmm_config_init_default(&config);

    assert(success == true);
    assert(config.page_size == 4096);
    assert(config.ram_size == 65536);
    assert(config.total_frames == 16);
    assert(config.page_offset_bits == 12);

    LOG_INFO("Sanity Test Passed Successfully!");
    return 0;
}