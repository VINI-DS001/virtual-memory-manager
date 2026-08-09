# Source files for the VMM Library / Core
set(VMM_SOURCES
    utils/log.c
    config.c
    cpu.c
    ram.c
    page_table.c
    tlb.c
    mmu.c
    page_fault.c
)

# Public Header files
set(VMM_HEADERS
    ${CMAKE_SOURCE_DIR}/include/vmm/types.h
    ${CMAKE_SOURCE_DIR}/include/vmm/config.h
    ${CMAKE_SOURCE_DIR}/include/vmm/cpu.h
    ${CMAKE_SOURCE_DIR}/include/vmm/ram.h
    ${CMAKE_SOURCE_DIR}/include/vmm/page_table.h
    ${CMAKE_SOURCE_DIR}/include/vmm/tlb.h
    ${CMAKE_SOURCE_DIR}/include/vmm/mmu.h
    ${CMAKE_SOURCE_DIR}/include/vmm/page_fault.h
    ${CMAKE_SOURCE_DIR}/include/vmm/utils/log.h
)

# Core static library
add_library(vmm_core STATIC ${VMM_SOURCES} ${VMM_HEADERS})

target_include_directories(vmm_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Main Executable
add_executable(vmm_sim main.c)
target_link_libraries(vmm_sim PRIVATE vmm_core)