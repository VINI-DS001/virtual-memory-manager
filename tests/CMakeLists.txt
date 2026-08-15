# Basic architecture sanity test
add_executable(vmm_test_basic test_basic.c)
target_link_libraries(vmm_test_basic PRIVATE vmm_core)
add_test(NAME BasicSanityTest COMMAND vmm_test_basic)

# CPU module test
add_executable(vmm_test_cpu test_cpu.c)
target_link_libraries(vmm_test_cpu PRIVATE vmm_core)
add_test(NAME CpuWorkloadTest COMMAND vmm_test_cpu)

# RAM module test
add_executable(vmm_test_ram test_ram.c)
target_link_libraries(vmm_test_ram PRIVATE vmm_core)
add_test(NAME PhysicalRamTest COMMAND vmm_test_ram)

# MMU and Page Table module test
add_executable(vmm_test_mmu test_mmu.c)
target_link_libraries(vmm_test_mmu PRIVATE vmm_core)
add_test(NAME MmuTranslationTest COMMAND vmm_test_mmu)

# TLB module test
add_executable(vmm_test_tlb test_tlb.c)
target_link_libraries(vmm_test_tlb PRIVATE vmm_core)
add_test(NAME TlbCacheTest COMMAND vmm_test_tlb)

# Page fault module test
add_executable(vmm_test_page_fault test_page_fault.c)
target_link_libraries(vmm_test_page_fault PRIVATE vmm_core)
add_test(NAME PageFaultTest COMMAND vmm_test_page_fault)

# Page replacement module test
add_executable(vmm_test_replacement test_replacement.c)
target_link_libraries(vmm_test_replacement PRIVATE vmm_core)
add_test(NAME ReplacementTest COMMAND vmm_test_replacement)

# Swap disk and virtual memory system test
add_executable(vmm_test_swap test_swap.c)
target_link_libraries(vmm_test_swap PRIVATE vmm_core)
add_test(NAME SwapTest COMMAND vmm_test_swap)

# Statistics collector module test
add_executable(vmm_test_stats test_stats.c)
target_link_libraries(vmm_test_stats PRIVATE vmm_core)
add_test(NAME StatsTest COMMAND vmm_test_stats)

# Workload generator module test
add_executable(vmm_test_workload test_workload.c)
target_link_libraries(vmm_test_workload PRIVATE vmm_core)
add_test(NAME WorkloadTest COMMAND vmm_test_workload)

# Benchmark orchestrator module test
add_executable(vmm_test_benchmark test_benchmark.c)
target_link_libraries(vmm_test_benchmark PRIVATE vmm_core)
add_test(NAME BenchmarkTest COMMAND vmm_test_benchmark)