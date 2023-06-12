#include "PhysicalMemory.h"

void clearPage(uint64_t page_number){
    for (int i = 0; i < PAGE_SIZE; i++){
        PMwrite(page_number*PAGE_SIZE + i, 0);
    }
}

//call with 0 to calculate root shifts needed
int calculate_num_shifts(int depth){
    return OFFSET_WIDTH + (TABLES_DEPTH - 1 - depth) * OFFSET_WIDTH;
} // TODO: not sure that is true for all cases of number of bits representing the table

uint64_t map_virtual_to_physical(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress & ((1LL << (OFFSET_WIDTH + 1)) - 1);
    uint64_t current_p = virtualAddress >> calculate_num_shifts(0);
    for (int i = 1; i < TABLES_DEPTH; i++){

    }
    return 0;
}


//============================== exercise questions ========================================

/*
 * Initialize the virtual memory.
 */
void VMinitialize(){
    clearPage(0);
}

/* Reads a word from the given virtual address
 * and puts its content in *value.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMread(uint64_t virtualAddress, word_t* value){
    uint64_t physical_add = map_virtual_to_physical(virtualAddress);
    if (physical_add != 0){
        VMread(physical_add, value);
        return 1;
    }
    return 0;
}

/* Writes a word to the given virtual address.
 *
 * returns 1 on success.
 * returns 0 on failure (if the address cannot be mapped to a physical
 * address for any reason)
 */
int VMwrite(uint64_t virtualAddress, word_t value){
    uint64_t physical_add = map_virtual_to_phisical(virtualAddress);
    if (physical_add != 0){
        VMwrite(physical_add, value);
        return 1;
    }
    return 0;
}