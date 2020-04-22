#include "lab1.h"
#include <malloc.h>
#include <stdio.h>

#define FREE_STATE 0
#define BUSY_STATE 1

#define DEFAULT_FILLER 0
#define DEFAULT_CHECK_SUM 0

typedef struct {
    char busyState;
    el_t checkSum;
    size_t size;
    size_t prevSize;
} Header;

typedef Header* (*SearchAlgorithmFun)(size_t size);


el_t *firstBlock;

int blocksQuantity = 3;

SearchAlgorithm sAlgorithm = FIRST_FOUND_ALGORITHM;

Header *firstFoundSAlgorithm(size_t size);
Header *minSizeSAlgorithm(size_t size);

SearchAlgorithmFun sAlgFunArray[] = {
    firstFoundSAlgorithm,
    minSizeSAlgorithm
};


size_t headerSize = sizeof(Header) / sizeof(el_t);

Header *fillHeader(Header *headerPtr, size_t size, size_t prevSize, char busyState) {
    headerPtr->busyState = busyState;
    headerPtr->checkSum = DEFAULT_CHECK_SUM;
    headerPtr->size = size;
    headerPtr->prevSize = prevSize;

    return headerPtr;
}

Header *getNextHeader(Header *lastHeaderPtr) {
    return (Header*)((el_t*)lastHeaderPtr + headerSize + lastHeaderPtr->size);
}


Header *getPrevHeader(Header *lastHeaderPtr) {
    return (Header*)((el_t*)lastHeaderPtr - lastHeaderPtr->prevSize - headerSize);
}


int isAppropriateBlock(Header *headerPtr, size_t size) {
    return headerPtr->busyState == FREE_STATE && headerPtr->size >= size;
}

void poolInitialize() {
    firstBlock = (el_t*)malloc(POOL_SIZE * sizeof(el_t));
 
    for ( el_t *ptr = firstBlock, *lastPtr = ptr + POOL_SIZE - 1;
            ptr <= lastPtr; ptr++ ) {
        *ptr = DEFAULT_FILLER;
    }

    size_t freeSpace = POOL_SIZE - 3 * headerSize;

    el_t *ptr = firstBlock;
    fillHeader((Header*) ptr, 0, 0, BUSY_STATE);

    ptr += headerSize;
    fillHeader((Header*) ptr, freeSpace, 0, FREE_STATE);

    ptr += headerSize + freeSpace;
    fillHeader((Header*) ptr, 0, freeSpace, BUSY_STATE);
}

void poolFree() {
    free(firstBlock);
}

void setSearchAlgorithm(SearchAlgorithm algorithm) {
    sAlgorithm = algorithm;
}


Header *firstFoundSAlgorithm(size_t size) {
    Header *headerPtr = (Header*)firstBlock + 1;

    for ( int i = 1, last = blocksQuantity - 2; i <= last; i++ ) {
        if ( isAppropriateBlock(headerPtr, size) ) {
            return headerPtr;
        }
        headerPtr = getNextHeader(headerPtr);
    }

    return NULL;
}

Header *minSizeSAlgorithm(size_t size) {
    Header *headerPtr = (Header*)firstBlock + 1;
    size_t minBlockSize = POOL_SIZE;
    int minBlockIndex = -1;

    for ( int i = 1, last = blocksQuantity - 2; i <= last; i++ ) {
        if ( isAppropriateBlock(headerPtr, size) && minBlockSize > headerPtr->size ) {
            minBlockSize = headerPtr->size;
            minBlockIndex = i;
        }
        headerPtr = getNextHeader(headerPtr);
    }

    if ( minBlockIndex == -1 ) {
        return NULL;
    }

    headerPtr = (Header*)firstBlock + 1;
    for ( int i = 1; i < minBlockIndex; i++ ) {
        headerPtr = getNextHeader(headerPtr);
    }

    return headerPtr;
}

void *freeBlockAlloc(Header *headerPtr, size_t size) {
    size_t restSpace = headerPtr->size - size;

        if ( restSpace > headerSize ) {
        Header *newHeaderPtr = (Header*)((el_t*)headerPtr + headerSize + size);
        fillHeader(newHeaderPtr, restSpace - headerSize, size, FREE_STATE);

        getNextHeader(newHeaderPtr)->prevSize = newHeaderPtr->size;

        headerPtr->size = size;      

        blocksQuantity++;
    }
    headerPtr->busyState = BUSY_STATE;

    return (void*)(headerPtr + 1);
}

void *mem_alloc(size_t size) {
    Header *headerPtr = (*sAlgFunArray[sAlgorithm])(size);

    if ( headerPtr == NULL ) {
        return NULL;
    }

    return freeBlockAlloc(headerPtr, size);
}

void *mem_realloc(void *addr, size_t size) {
    if ( addr == NULL ) {
        return mem_alloc(size);
    }

    Header *headerPtr = (Header*)addr - 1;
    size_t freeSpace = headerPtr->size;

    
    if ( freeSpace > size ) {
        return freeBlockAlloc(headerPtr, size);
    }

        Header *nextHeaderPtr = getNextHeader(headerPtr);
    if ( nextHeaderPtr->busyState == FREE_STATE ) {
        freeSpace += nextHeaderPtr->size + headerSize;

        if ( freeSpace > size ) {
            headerPtr->size = freeSpace;
            blocksQuantity--;

            return freeBlockAlloc(headerPtr, size);
        }
    }

    el_t *oldPtr = (el_t*)addr;
    el_t *newPtr;

   
    Header *prevHeaderPtr = getPrevHeader(headerPtr);
    if ( prevHeaderPtr->busyState == FREE_STATE ) {
        freeSpace += prevHeaderPtr->size + headerSize;

        if ( freeSpace > size) {
            prevHeaderPtr->size = freeSpace;
            blocksQuantity--;

            newPtr = (el_t*)(prevHeaderPtr + 1);
            for ( size_t i = 0; i < headerPtr->size; i++ ) {
                newPtr[i] = oldPtr[i];
            }

            return freeBlockAlloc(prevHeaderPtr, size);
        }
    }

    newPtr = (el_t*)mem_alloc(size);
    if ( newPtr == NULL ) {
        return NULL;
    }

    for ( size_t i = 0; i < headerPtr->size; i++ ) {
        newPtr[i] = oldPtr[i];
    }

    mem_free((void*)oldPtr);
    return newPtr;
}

void mem_free(void *addr) {
    Header *headerPtr = (Header*)addr - 1;
    Header *nextHeaderPtr = getNextHeader(headerPtr);
    Header *prevHeaderPtr = getPrevHeader(headerPtr);

    headerPtr->busyState = FREE_STATE;

    if ( nextHeaderPtr->busyState == FREE_STATE ) {
        headerPtr->size += nextHeaderPtr->size + headerSize;
        getNextHeader(headerPtr)->prevSize = headerPtr->size;
        blocksQuantity--;
    }

    if ( prevHeaderPtr->busyState == FREE_STATE ) {
        prevHeaderPtr->size += headerPtr->size + headerSize;
        getNextHeader(prevHeaderPtr)->prevSize = prevHeaderPtr->size;
        blocksQuantity--;
    }
}

void mem_dump() {
    Header *headerPtr = (Header*) firstBlock;

    printf("Blocks quantity: %d\n", blocksQuantity);

    for ( int i = 0; i < blocksQuantity; i++ ) {
        printf("Block %d:\n", i);
        printf("\tBusy state: %s\n", headerPtr->busyState == BUSY_STATE ? "busy" : "free");
        printf("\tCheck sum: %d\n", (unsigned char)headerPtr->checkSum);
        printf("\tBlock size: %u\n", headerPtr->size);
        printf("\tPrevious block size: %d\n", headerPtr->prevSize);

        headerPtr = getNextHeader(headerPtr);
    }
    printf("----------------------------------------\n");
}

void updateCheckSums() {
    Header *headerPtr = (Header*)firstBlock + 1;

    for ( int i = 1, last = blocksQuantity - 2; i <= last; i++ ) {
        el_t checkSum = 0;

        for ( el_t *ptr = (el_t*)(headerPtr + 1), *lastPtr = ptr + headerPtr->size - 1;
                ptr <= lastPtr; ptr++ ) {
            checkSum ^= *ptr;
        }
        headerPtr->checkSum = checkSum;

        headerPtr = getNextHeader(headerPtr);
    }
}
