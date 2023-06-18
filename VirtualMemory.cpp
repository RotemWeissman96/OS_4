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

void dfsFindFrameToEvict(uint64_t currentDepth, uint64_t &currentFrame, uint64_t &currentParent
                         ,uint64_t &maxFrameNumberInUse, uint64_t *parentsList,
                         uint64_t &maxCycleValue, uint64_t &maxCyclePageNumber, uint64_t &maxCycleParent,
                         uint64_t &emptyFrame, uint64_t &emptyFrameParent){
    if (currentDepth == TABLES_DEPTH - 1){ // we reached a leaf
        //TODO: check if it is max cycle
    }
    else{
        if (isFrameEmpty(currentFrame)){
            emptyFrame = currentFrame;
            emptyFrameParent = currentParent;
        }
        else{
            for (uint64_t address = currentFrame*PAGE_SIZE; address < (currentFrame+1)*PAGE_SIZE; address++){
                word_t val = 0;
                PMread(address, &val);
                if (val){
                    if(maxFrameNumberInUse < val){
                        maxFrameNumberInUse = val;
                    }
                    //TODO: add to parent list


                    dfsFindFrameToEvict(currentDepth+1, reinterpret_cast<uint64_t &>(val), currentFrame
                            , maxFrameNumberInUse, parentsList, maxCycleValue, maxCyclePageNumber, maxCycleParent,
                                        emptyFrame, emptyFrameParent);
                }
            }
            // TODO: remove from parent list
        }
    }


}

uint64_t mapVirtualToPhysical(uint64_t virtualAddress) {
    uint64_t offset = virtualAddress << calculateNumShifts(0);
    uint64_t currentTableOffset = virtualAddress >> calculateNumShifts(0);
    word_t currentTableFrameNumber = 0;
    uint64_t inputPageNumber = calculatePageNumber(virtualAddress);
    uint64_t parentsList[TABLES_DEPTH*2];
    parentsList[0] = 0;
    for (int i = 1; i < TABLES_DEPTH - 1; i++){
        // calculating the next table address (if its last one, then the next table is the resulting page)
        PMread(currentTableFrameNumber * PAGE_SIZE + currentTableOffset, &currentTableFrameNumber);
        currentTableOffset = (virtualAddress << calculateNumShifts(i)) % PAGE_SIZE;

        parentsList[i] = currentTableFrameNumber;

        if(currentTableFrameNumber == 0){ // there is a page falt
            // init all arguments for dfs
            uint64_t lastFrameChecked = 0;
            uint64_t lastFrameCheckedParent = -1;
            uint64_t maxFrameNumberInUse = 0;
            uint64_t maxCycleValue = -1;
            uint64_t maxCyclePageNumber = -1;
            uint64_t maxCycleParent = -1;
            uint64_t emptyFrame = -1;
            uint64_t emptyFrameParent = -1;
            //call dfs to gather information on the tree
            dfsFindFrameToEvict(0, lastFrameChecked, lastFrameCheckedParent
                    ,maxFrameNumberInUse, parentsList,
                    maxCycleValue, maxCyclePageNumber, maxCycleParent,
                    emptyFrame, emptyFrameParent);
            // TODO: check options to choose and update parent
        }
    }
    return currentTableFrameNumber * PAGE_SIZE + offset;
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