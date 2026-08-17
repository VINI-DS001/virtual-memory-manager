# VMM Simulator: Technical Manual & API Reference

## 1. Core Data Structures

The simulator relies on packed, memory-efficient data structures designed to model hardware-level MMU registers, Page Table Entries (PTE), and TLB rows.

### Config Specification (`vmm_config_t`)
Defines the simulation parameters before initializing the VMM instance:
* `num_virtual_pages`: Total virtual pages in address space ($2^N$).
* `num_frames`: Number of physical RAM frames available.
* `page_size`: Bytes per page (default: 4096 bytes / 12-bit offset).
* `tlb_size`: Number of slots in the fully-associative TLB.
* `replacement_policy`: Enum selecting the active policy (`POLICY_FIFO`, `POLICY_LRU`, `POLICY_LFU`, `POLICY_CLOCK`, `POLICY_RANDOM`).

### Page Table Entry (`page_table_entry_t`)
* `frame_number`: Physical frame index if mapped in RAM.
* `valid_bit`: `1` if page resides in RAM, `0` if on Swap disk.
* `dirty_bit`: `1` if page was modified by a store operation.
* `referenced_bit`: Used by CLOCK/LRU to track recent memory accesses.

### TLB Entry (`tlb_entry_t`)
* `virtual_page_number` (VPN): Target page mapping.
* `frame_number`: Physical frame cache.
* `valid_bit`: Cache slot validity status.
* `last_access_timestamp`: Cycle timestamp for LRU cache eviction.

---

## 2. Bitwise Translation Mechanics

Address translation is executed in `vmm_mmu_translate()` using bitwise mask and shift operations for zero-overhead performance:

$$\text{VPN} = \text{Virtual Address} \gg \text{PAGE\_SHIFT}$$

$$\text{Offset} = \text{Virtual Address} \ \& \ (\text{PAGE\_SIZE} - 1)$$

$$\text{Physical Address} = (\text{Frame Number} \ll \text{PAGE\_SHIFT}) \ | \ \text{Offset}$$

---

## 3. Page Replacement Algorithms Mechanics

1. **FIFO (First-In, First-Out):**
   * Uses a circular queue pointer (`queue_head`).
   * Evicts the oldest loaded frame regardless of access frequency or recency.
2. **LRU (Least Recently Used):**
   * Tracks a global access logical clock (`uint64_t access_counter`).
   * On access, updates `last_access_timestamp`. On eviction, scans frame array for $\min(\text{timestamp})$.
3. **LFU (Least Frequently Used):**
   * Maintains an access counter (`frequency_count`) per active page.
   * On fault, evicts the page with the lowest counter. Resets counter on page load.
4. **CLOCK (Second-Chance Algorithm):**
   * Employs a circular hand pointer over physical frames.
   * If `referenced_bit == 1`, resets it to `0` and advances hand. If `referenced_bit == 0`, selects frame as victim.
5. **RANDOM:**
   * Selects a frame uniformly at random using pseudo-random generation (`rand() % num_frames`).

---

## 4. Primary API Reference

```c
/**
 * @brief Initializes the Virtual Memory Manager state and allocates RAM/Swap structures.
 * @param config Pointer to user-defined VMM configuration struct.
 * @return VMM_SUCCESS on success, VMM_ERROR_OOM on allocation failure.
 */
vmm_status_t vmm_init(const vmm_config_t *config);

/**
 * @brief Translates a virtual address to a physical address.
 * Handles TLB lookup, page table walks, and page fault triggers automatically.
 * 
 * @param v_addr Virtual address to access.
 * @param req_type Operation type (VMM_READ or VMM_WRITE).
 * @param p_addr Pointer to store translated physical address.
 * @return vmm_status_t Success status or fault indication.
 */
vmm_status_t vmm_translate_address(uint32_t v_addr, vmm_access_t req_type, uint32_t *p_addr);

/**
 * @brief Handles page faults by loading target page from Swap into RAM.
 * Runs eviction policies if physical memory is full.
 * 
 * @param vpn Virtual Page Number that triggered the fault.
 * @return Allocated frame index assigned to the page.
 */
int32_t vmm_handle_page_fault(uint32_t vpn);

/**
 * @brief Destroys VMM instance and frees allocated memory and disk files.
 */
void vmm_shutdown(void);