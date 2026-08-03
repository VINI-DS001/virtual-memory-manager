# Basic test executable placeholder
add_executable(vmm_test_basic test_basic.c)
target_link_libraries(vmm_test_basic PRIVATE vmm_core)

add_test(NAME BasicSanityTest COMMAND vmm_test_basic)