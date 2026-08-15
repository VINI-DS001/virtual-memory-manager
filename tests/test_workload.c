#include <assert.h>
#include "vmm/workload.h"
#include "vmm/utils/log.h"
#include "vmm/types.h"

int main(void)
{
    LOG_INFO("Executing Workload Generator Sanity Test...");

    workload_t wl;
    size_t accesses = 1000;
    uint32_t max_vpn = 16;
    uint32_t page_size = 4096;

    workload_generate(&wl, WORKLOAD_SEQUENTIAL, accesses, max_vpn, page_size);
    assert(wl.total_accesses == accesses);
    assert(wl.access_pattern != NULL);

    // Verify sequential progression in page numbers
    vpn_t vpn0 = (vpn_t)(wl.access_pattern[0] / page_size);
    vpn_t vpn1 = (vpn_t)(wl.access_pattern[1] / page_size);
    assert(vpn0 == 0);
    assert(vpn1 == 1);

    workload_destroy(&wl);
    assert(wl.access_pattern == NULL);

    LOG_INFO("Workload Generator Sanity Test Passed Successfully!");
    return 0;
}