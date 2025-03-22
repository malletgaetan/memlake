#include <assert.h>
#include <immintrin.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// TODO: Add possibility of padding the elements in mem[]

typedef struct _mem_pool {
	size_t element_size_in_bytes;
	struct _mem_pool *next;
	size_t sp;
	uint16_t stack[UINT16_MAX + 1];
	uint8_t mem[];
} MemoryPool;

typedef struct {
	size_t num_pools;
	size_t num_allocated;
	size_t element_size_in_bytes;
	MemoryPool *free_pool_list;
	MemoryPool *full_pool_list;
} MemoryLake;

MemoryLake *memlake_create(size_t element_size_in_bytes);
void memlake_destroy(MemoryLake *lake);
void *memlake_alloc(MemoryLake *lake);
void memlake_free(MemoryLake *lake, void *ptr);
