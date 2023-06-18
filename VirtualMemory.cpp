#include "PhysicalMemory.h"

#define PAGE_FAULT 0
void clearPage(uint64_t page_number){
    for (int i = 0; i < PAGE_SIZE; i++){
        PMwrite(page_number*PAGE_SIZE + i, 0);
    }
}

bool isFrameEmpty(uint64_t &currentFrame){
    word_t val = 0;

    for (uint64_t address = currentFrame*PAGE_SIZE; address < (currentFrame+1)*PAGE_SIZE; address++) {
        PMread(address, &val);
        if (val != 0){
            return false;
        }
    }
    return true;

}

void updateMaxCycle(uint64_t &maxCycleValue, uint64_t &maxCycleFrameNumber, uint64_t &maxCycleParent,
                    uint64_t swappedInPageNumber, uint64_t &pageCount, uint64_t &currentParent, uint64_t &currentFrame){
    // calculate new cycle value:
    uint64_t firstPart =  swappedInPageNumber - pageCount;
    if (firstPart < 0){
        firstPart = -firstPart;
    }
    uint64_t currentCyclicValue = firstPart;
    if (currentCyclicValue > NUM_PAGES - firstPart){
        currentCyclicValue = NUM_PAGES - firstPart;
    }
    // decide if to update the max cycle
    if (currentCyclicValue > maxCycleValue){
        maxCycleValue = currentCyclicValue;
        maxCycleParent = currentParent;
        maxCycleFrameNumber = currentFrame;
    }
}

void dfsFindFrameToEvict(uint64_t currentDepth, uint64_t &currentFrameNumber, uint64_t &currentParent,
                         uint64_t &maxFrameNumberInUse, uint64_t &pageCount,
                         uint64_t &maxCycleValue, uint64_t &maxCycleFrameNumber, uint64_t &maxCycleParent, uint64_t swappedInPageNumber,
                         uint64_t &emptyFrame, uint64_t &emptyFrameParent, uint64_t forbiddenFrame){
    if(maxFrameNumberInUse < currentFrameNumber){
        maxFrameNumberInUse = currentFrameNumber;
    }
    if (currentDepth == TABLES_DEPTH - 1){ // we reached a leaf
        pageCount ++;
        updateMaxCycle(maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber, pageCount,
                       currentParent, currentFrameNumber);
    }
    else{
        if (currentFrameNumber != forbiddenFrame && isFrameEmpty(currentFrameNumber)){
            emptyFrame = currentFrameNumber;
            emptyFrameParent = currentParent;
            pageCount = pageCount + (1 << (TABLES_DEPTH - currentDepth));
        }
        else{
            for (uint64_t address = currentFrameNumber*PAGE_SIZE; address < (currentFrameNumber+1)*PAGE_SIZE; address++){
                word_t val = 0;
                PMread(address, &val);
                if (val){
                    dfsFindFrameToEvict(currentDepth+1, reinterpret_cast<uint64_t &>(val), address,
                                        maxFrameNumberInUse, pageCount,
                                        maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber,
                                        emptyFrame, emptyFrameParent, forbiddenFrame);
                }
            }
        }
    }
}

uint64_t handlePageFault(uint64_t swappedInPageNumber, uint64_t faultAddress, uint64_t forbiddenFrame){
    // init all arguments for dfs
    uint64_t lastFrameChecked = 0;
    uint64_t lastFrameCheckedParent = 0;
    uint64_t maxFrameNumberInUse = 0;
    uint64_t maxCycleValue = 0;
    uint64_t maxCycleFrameNumber = 0;
    uint64_t maxCycleParent = 0;
    uint64_t emptyFrame = 0;
    uint64_t emptyFrameParent = 0;
    uint64_t pageCount = 0;
    //call dfs to gather information on the tree
    dfsFindFrameToEvict(0, lastFrameChecked, lastFrameCheckedParent,
                        maxFrameNumberInUse, pageCount,
                        maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber,
                        emptyFrame, emptyFrameParent, forbiddenFrame);
    uint64_t newFrame = 0;
    if (emptyFrame != 0){ // case 1
        newFrame = emptyFrame;
        PMwrite(emptyFrameParent,PAGE_FAULT);
    }
    else if(maxFrameNumberInUse+1<NUM_FRAMES){ // case 2
        newFrame = maxFrameNumberInUse + 1;
    }
    else { // case 3
        newFrame = maxCycleFrameNumber;
        PMwrite(maxCycleParent, PAGE_FAULT);
    }
    PMwrite(faultAddress,newFrame);
    return newFrame;
}

/**
 * f[DEPTH] = offset
 * the rest are the inner tabels off sets
 * @param virtualAddress
 * @param f
 */
void parseVirtualAddress(uint64_t virtualAddress, uint64_t *f){
    virtualAddress = virtualAddress >> OFFSET_WIDTH;
    for (int i = 0; i < TABLES_DEPTH; i++){
        f[TABLES_DEPTH - 1 - i] = virtualAddress % PAGE_SIZE;
        virtualAddress = virtualAddress >> OFFSET_WIDTH;
    }
}

uint64_t mapVirtualToPhysical(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress % PAGE_SIZE;
    uint64_t swappedInPageNumber = virtualAddress >> OFFSET_WIDTH;
    uint64_t f[TABLES_DEPTH];
    parseVirtualAddress(virtualAddress, f);
    word_t currentFrameNumber = 0;
    uint64_t currAddress = 0;
    uint64_t forbiddenFrame = 0;
    for (int i = 0; i < TABLES_DEPTH; i++){
        currAddress =  (currentFrameNumber * PAGE_SIZE) + f[i];
        PMread(currAddress, &currentFrameNumber);
        if(currentFrameNumber == PAGE_FAULT){ // there is a page fault
            currentFrameNumber = handlePageFault(swappedInPageNumber, currAddress, forbiddenFrame);
            forbiddenFrame = currentFrameNumber;
        }
    }
    return currentFrameNumber * PAGE_SIZE + offset;
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
    uint64_t physicalAddress = mapVirtualToPhysical(virtualAddress);
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
    uint64_t physicalAddress = mapVirtualToPhysical(virtualAddress);
    if (physicalAddress != 0){
        VMwrite(physicalAddress, value);
        return 1;
    }
    return 0;
}