#include "PhysicalMemory.h"

void clearPage(uint64_t page_number){
    for (int i = 0; i < PAGE_SIZE; i++){
        PMwrite(page_number*PAGE_SIZE + i, 0);
    }
}

//call with 0 to calculate root shifts needed
int calculateNumShifts(int depth){
    return (TABLES_DEPTH - depth) * OFFSET_WIDTH; //table depth - offset - current depth
}

uint64_t calculatePageNumber(uint64_t virtualAddress){
    return virtualAddress << OFFSET_WIDTH;
}

uint64_t mapVirtualVoPhysical(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress << calculateNumShifts(0);
    uint64_t currentTableOffset = virtualAddress >> calculateNumShifts(0);
    word_t currentTablePageNumber = 0;
    uint64_t inputPageNumber = calculatePageNumber(virtualAddress);
    for (int i = 1; i < TABLES_DEPTH - 1; i++){
        PMread(currentTablePageNumber * PAGE_SIZE + currentTableOffset, &currentTablePageNumber);
        currentTableOffset = (virtualAddress << calculateNumShifts(i)) % PAGE_SIZE;

        if(currentTablePageNumber == 0){
            //TODO: handle page fault
        }
    }
    return currentTablePageNumber * PAGE_SIZE + offset;
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
    uint64_t physicalAddress = mapVirtualVoPhysical(virtualAddress);
    if (physicalAddress != 0){
        VMread(physicalAddress, value);
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
    uint64_t physicalAddress = mapVirtualVoPhysical(virtualAddress);
    if (physicalAddress != 0){
        VMwrite(physicalAddress, value);
        return 1;
    }
    return 0;
}