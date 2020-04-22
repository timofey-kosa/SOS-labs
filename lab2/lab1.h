#define POOL_SIZE 1024
#define MIN_BLOCK_SIZE 4

typedef char el_t;

typedef enum {
    FIRST_FOUND_ALGORITHM,
    MIN_SIZE_ALGORITHM
} SearchAlgorithm;

void poolInitialize();

void poolFree();

void setSearchAlgorithm(SearchAlgorithm algorithm);

void *mem_alloc(size_t size);

void *mem_realloc(void *addr, size_t size);

void mem_free(void *addr);

void mem_dump();

void updateCheckSums();
