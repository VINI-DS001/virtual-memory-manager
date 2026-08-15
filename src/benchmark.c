#include "vmm/benchmark.h"
#include "vmm/ram.h"
#include "vmm/swap.h"
#include "vmm/page_table.h"
#include "vmm/tlb.h"
#include "vmm/mmu.h"
#include "vmm/page_fault.h"
#include "vmm/utils/log.h"

void vmm_benchmark_run(const benchmark_config_t *bench_cfg,
                       const workload_t *workload,
                       const vmm_config_t *base_cfg,
                       vmm_stats_t *out_stats)
{
    if (!bench_cfg || !workload || !base_cfg || !out_stats)
        return;

    vmm_stats_init(out_stats);

    /* 1. Setup execution configuration override if specified */
    vmm_config_t config = *base_cfg;
    if (bench_cfg->custom_frame_count > 0)
    {
        config.total_frames = bench_cfg->custom_frame_count;
        config.ram_size = config.total_frames * config.page_size;
    }
    if (bench_cfg->custom_tlb_capacity > 0)
    {
        config.tlb_capacity = bench_cfg->custom_tlb_capacity;
    }

    /* 2. Initialize VMM Subsystems */
    ram_t ram;
    ram_init(&ram, &config);

    swap_disk_t swap;
    swap_init(&swap, &config);

    page_table_t pt;
    uint32_t max_pt_entries = (workload->max_vpn > 0) ? workload->max_vpn : config.total_swap_pages;
    page_table_init(&pt, max_pt_entries);

    tlb_t tlb;
    tlb_init(&tlb, config.tlb_capacity);

    replacement_manager_t repl_mgr;
    replacement_init(&repl_mgr, config.total_frames, bench_cfg->policy);

    mmu_t mmu;
    mmu_init(&mmu, &ram, &pt, &tlb, config.page_offset_bits);

    /* 3. Execute Workload Loop with High-Precision Timing */
    vmm_stats_start_timer(out_stats);

    for (size_t i = 0; i < workload->total_accesses; i++)
    {
        vaddr_t vaddr = workload->access_pattern[i];
        paddr_t paddr = 0;
        vpn_t vpn = mmu_extract_vpn(vaddr, config.page_offset_bits);

        // Check TLB hit prior to translation
        pfn_t dummy_pfn = 0;
        bool is_tlb_hit = tlb_lookup(&tlb, vpn, &dummy_pfn);
        vmm_stats_record_access(out_stats, is_tlb_hit);

        // Perform hardware translation
        mmu_status_t status = mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);

        if (status == MMU_PAGE_FAULT)
        {
            vmm_stats_record_page_fault(out_stats);
            handle_page_fault(bench_cfg->pid, vpn, &pt, &ram, &tlb, &repl_mgr, &swap);

            // Retry translation after page fault resolution
            mmu_translate(&mmu, vaddr, PAGE_FLAG_READABLE, &paddr);
        }
    }

    vmm_stats_stop_timer(out_stats);

    /* 4. Aggregate Swap Disk Operations */
    out_stats->swap_reads = swap.swap_reads;
    out_stats->swap_writes = swap.swap_writes;

    /* 5. Cleanup Resources */
    replacement_destroy(&repl_mgr);
    tlb_destroy(&tlb);
    page_table_destroy(&pt);
    swap_destroy(&swap);
    ram_destroy(&ram);
}