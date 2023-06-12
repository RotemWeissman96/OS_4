#include "PhysicalMemory.h"

void clearPage(uint64_t page_number){
    for (int i = 0; i < PAGE_SIZE; i++){
        PMwrite(page_number*PAGE_SIZE + i, 0);
    }
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
    uint64_t phisical_add = map_virtual_to_phisical(virtualAddress);
    if (phisical_add){
        VMread(phisical_add, value);
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
    uint64_t phisical_add = map_virtual_to_phisical(virtualAddress);
    if (phisical_add){
        VMwrite(phisical_add, value);
        return 1;
    }
    return 0;
}