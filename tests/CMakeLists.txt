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