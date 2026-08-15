#include <assert.h>
#include <unistd.h>
#include "vmm/stats.h"
#include "vmm/utils/log.h"

int main(void)
{
    LOG_INFO("Executing VMM Statistics Sanity Test...");

    vmm_stats_t stats;
    vmm_stats_init(&stats);

    assert(stats.total_accesses == 0);

    vmm_stats_start_timer(&stats);

    // Simulate 100 accesses: 80 hits, 20 misses
    for (int i = 0; i < 80; i++)
    {
        vmm_stats_record_access(&stats, true);
    }
    for (int i = 0; i < 20; i++)
    {
        vmm_stats_record_access(&stats, false);
    }

    // Record 2 page faults and swap ops
    vmm_stats_record_page_fault(&stats);
    vmm_stats_record_page_fault(&stats);
    vmm_stats_record_swap_write(&stats);
    vmm_stats_record_swap_read(&stats);

    usleep(10000); // 10 ms delay for timer test

    vmm_stats_stop_timer(&stats);

    assert(stats.total_accesses == 100);
    assert(stats.tlb_hits == 80);
    assert(stats.tlb_misses == 20);
    assert(stats.page_faults == 2);
    assert(stats.swap_writes == 1);
    assert(stats.swap_reads == 1);

    assert(vmm_stats_get_tlb_hit_rate(&stats) == 80.0);
    assert(vmm_stats_get_page_fault_rate(&stats) == 2.0);
    assert(stats.elapsed_ms >= 10.0);

    vmm_stats_print_report(&stats, "SanityCheck", "FIFO");

    LOG_INFO("VMM Statistics Sanity Test Passed Successfully!");
    return 0;
}