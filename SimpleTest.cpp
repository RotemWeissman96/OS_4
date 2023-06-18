#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <iostream>
#include <cstdio>
#include <cassert>

//void parseVirtualAddress1(uint64_t virtualAddress, uint64_t *f){
//    virtualAddress = virtualAddress >> OFFSET_WIDTH;
//    for (int i = 0; i < TABLES_DEPTH; i++){
//        f[TABLES_DEPTH - 1 - i] = virtualAddress % PAGE_SIZE;
//        virtualAddress = virtualAddress >> OFFSET_WIDTH;
//    }
//}

int main(int argc, char **argv) {
//    word_t value;
//    VMwrite(0, 30);
//    VMread(0, &value);
//    printf("%d\n", value);
//    PMread(0, &value);
//    printf("%d", value);

    VMinitialize();
    for (uint64_t i = 0; i < (2); ++i) {
        printf("writing to %llu %d\n", (long long int) i, i);
        VMwrite(i * PAGE_SIZE, i);
    }

    for (uint64_t i = 0; i < (2); ++i) {
        word_t value;
        VMread( i*PAGE_SIZE, &value);
        printf("reading from %llu %d\n", (long long int) i, value);
//        assert(uint64_t(value) == i);
    }
    printf("success\n");



//    uint64_t f[TABLES_DEPTH];
//    uint64_t virtualAddress = 0;
//    std::cout << "table depth = " << TABLES_DEPTH << std::endl;
//    std::cout << "virtual memory width = " << VIRTUAL_ADDRESS_WIDTH;
//
//    parseVirtualAddress1(virtualAddress, f);

    return 0;
}
