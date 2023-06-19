#include "PhysicalMemory.h"

#define PAGE_FAULT 0

/**
 * fill entire frame with zeros
 * @param frameNumber
 */
void clearFrame(word_t frameNumber){
    for (int i = 0; i < PAGE_SIZE; i++){
        PMwrite(frameNumber * PAGE_SIZE + i, 0);
    }
}

/**
 * check if this frame is all filled with 0
 * @param currentFrame
 * @return true/false base on the answer
 */
bool isFrameEmpty(word_t &currentFrame){
    word_t val = 0;

    for (uint64_t address = currentFrame*PAGE_SIZE; address < (currentFrame+1)*PAGE_SIZE; address++) {
        PMread(address, &val);
        if (val != 0){
            return false;
        }
    }
    return true;
}

/**
 * check if the current max cycle page in the RAM is higher then the current page cycle distance
 * @param maxCyclePageNumber what is the page number with the highest cycle distance
 * @param maxCycleValue what is the highest cycle distance so far
 * @param maxCycleFrameNumber where is that page stored in the RAM
 * @param maxCycleParent what address pointed to this frame
 * @param swappedInPageNumber page number of the current page looking to add to RAM
 * @param pageCount the number of the current page in the dfs search
 * @param currentParent what address pointed to the current frame
 * @param currentFrame
 */
void updateMaxCycle(uint64_t &maxCycleValue, word_t &maxCycleFrameNumber, uint64_t &maxCycleParent,
                    uint64_t swappedInPageNumber, uint64_t &pageCount, uint64_t &currentParent,
                    word_t &currentFrame, uint64_t &maxCyclePageNumber){
    // calculate new cycle value:
    uint64_t firstPart =  swappedInPageNumber - pageCount;
    if (swappedInPageNumber < pageCount){
        firstPart = pageCount - swappedInPageNumber;
    } else {
        firstPart = swappedInPageNumber - pageCount;
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
        maxCyclePageNumber = pageCount;
    }
}

/**
 * running a dfs over the virtual tree of tables in order to gathe information.
 * based on that information we decide where to add the current page to add
 * @param currentDepth what depth in the virtual tree are we in the dfs
 * @param maxFrameNumberInUse keeps the max frame number, the goal is to know if RAM is full
 * @param emptyFrame if we find an empty frame to fill, if not this will be 0
 * @param emptyFrameParent the address pointing to the empty frame found
 * @param forbiddenFrame a frame who we are not allowed to return - its the parent of the current page to add
 * @param maxCyclePageNumber what is the page number with the highest cycle distance
 * @param maxCycleValue what is the highest cycle distance so far
 * @param maxCycleFrameNumber where is that page stored in the RAM
 * @param maxCycleParent what address pointed to this frame
 * @param swappedInPageNumber page number of the current page looking to add to RAM
 * @param pageCount the number of the current page in the dfs search
 * @param currentParent what address pointed to the current frame
 * @param currentFrame
 */
void dfsFindFrameToEvict(int currentDepth, word_t &currentFrameNumber, uint64_t &currentParent,
                         word_t &maxFrameNumberInUse, uint64_t &pageCount, uint64_t &maxCyclePageNumber,
                         uint64_t &maxCycleValue, word_t &maxCycleFrameNumber, uint64_t &maxCycleParent, uint64_t swappedInPageNumber,
                         word_t &emptyFrame, uint64_t &emptyFrameParent, word_t forbiddenFrame){
    if(maxFrameNumberInUse < currentFrameNumber){
        maxFrameNumberInUse = currentFrameNumber;
    }
    if (currentDepth == TABLES_DEPTH){ // we reached a leaf
        updateMaxCycle(maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber, pageCount,
                       currentParent, currentFrameNumber, maxCyclePageNumber);
        pageCount ++;
    }
    else{
        if (currentFrameNumber != forbiddenFrame && isFrameEmpty(currentFrameNumber) && currentDepth != TABLES_DEPTH){
            emptyFrame = currentFrameNumber;
            emptyFrameParent = currentParent;
            pageCount = pageCount + (1 << (OFFSET_WIDTH*(TABLES_DEPTH - currentDepth - 1)));
        }
        else{
            for (uint64_t address = currentFrameNumber*PAGE_SIZE; address < static_cast<uint64_t>(currentFrameNumber+1)*PAGE_SIZE; address++){
                word_t val = 0;
                PMread(address, &val);
                if (val){
                    dfsFindFrameToEvict(currentDepth+1, val, address,
                                        maxFrameNumberInUse, pageCount, maxCyclePageNumber,
                                        maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber,
                                        emptyFrame, emptyFrameParent, forbiddenFrame);
                } else{
                    pageCount = pageCount + (1 << (OFFSET_WIDTH*(TABLES_DEPTH - currentDepth - 1)));
                }
            }
        }
    }
}

/**
 * handles a page fault
 * @param depth the depth of the page to add
 * @param swappedInPageNumber page number of the current page looking to add to RAM
 * @param faultAddress the address in ram that was supposed to point to the current page to add but had 0
 * @param forbiddenFrame a frame who we are not allowed to return - its the parent of the current page to add
 * @return
 */
uint64_t handlePageFault(int depth, uint64_t swappedInPageNumber, uint64_t faultAddress, word_t forbiddenFrame){
    // init all arguments for dfs
    word_t lastFrameChecked = 0;
    uint64_t lastFrameCheckedParent = 0;
    word_t maxFrameNumberInUse = 0;
    uint64_t maxCycleValue = 0;
    word_t maxCycleFrameNumber = 0;
    uint64_t maxCycleParent = 0;
    word_t emptyFrame = 0;
    uint64_t emptyFrameParent = 0;
    uint64_t pageCount = 0;
    uint64_t maxCyclePageNumber = 0;
    //call dfs to gather information on the tree
    dfsFindFrameToEvict(0, lastFrameChecked, lastFrameCheckedParent,
                        maxFrameNumberInUse, pageCount, maxCyclePageNumber,
                        maxCycleValue, maxCycleFrameNumber, maxCycleParent, swappedInPageNumber,
                        emptyFrame, emptyFrameParent, forbiddenFrame);
    word_t newFrame = 0;
    if (emptyFrame != 0){ // case 1
        newFrame = emptyFrame;
        PMwrite(emptyFrameParent,PAGE_FAULT);
    }
    else if(maxFrameNumberInUse+1<NUM_FRAMES){ // case 2
        newFrame = maxFrameNumberInUse + 1;
    }
    else { // case 3
        newFrame = maxCycleFrameNumber;
        PMevict(newFrame, maxCyclePageNumber);
        PMwrite(maxCycleParent, PAGE_FAULT);
    }
    if (depth == TABLES_DEPTH - 1){ // if we insert a leaf
        PMrestore(newFrame, swappedInPageNumber);
    } else { // if we insert a table
        clearFrame(newFrame);
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

/**
 * maps virtual addresses to Physical addresses, handles page faults
 * @param virtualAddress
 * @return a physical address
 */
uint64_t mapVirtualToPhysical(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress % PAGE_SIZE;
    uint64_t swappedInPageNumber = virtualAddress >> OFFSET_WIDTH;
    uint64_t f[TABLES_DEPTH];
    parseVirtualAddress(virtualAddress, f);
    word_t currentFrameNumber = 0;
    uint64_t currAddress = 0;
    word_t forbiddenFrame = 0;
    for (int i = 0; i < TABLES_DEPTH; i++){
        currAddress =  (currentFrameNumber * PAGE_SIZE) + f[i];
        PMread(currAddress, &currentFrameNumber);
        if(currentFrameNumber == PAGE_FAULT){ // there is a page fault
            currentFrameNumber = handlePageFault(i, swappedInPageNumber, currAddress, forbiddenFrame);
        }
        forbiddenFrame = currentFrameNumber;
    }

    return currentFrameNumber * PAGE_SIZE + offset;
}


//============================== exercise questions ========================================

/*
 * Initialize the virtual memory.
 */
void VMinitialize(){
    clearFrame(0);
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
    if (physicalAddress != 0 && virtualAddress < VIRTUAL_MEMORY_SIZE){
        PMread(physicalAddress, value);
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
    if (physicalAddress != 0 && virtualAddress < VIRTUAL_MEMORY_SIZE){
        PMwrite(physicalAddress, value);
        return 1;
    }
    return 0;
}