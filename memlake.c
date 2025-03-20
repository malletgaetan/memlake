#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

// TODO: make stack grow upward for better cache locality in early allocations
// TODO: Add possibility of padding the elements in mem[]

typedef struct {
	size_t   element_size_in_bytes;
	uint16_t sp;
	uint16_t stack[UINT16_MAX + 1];
	uint8_t  mem[];
} MemoryPool;

typedef struct {
	size_t     num_pools;
	size_t     num_allocated;
	size_t     element_size_in_bytes;
	uint16_t   sp;
	uint16_t   stack[4096];
	MemoryPool *pools[4096];
} MemoryLake;

static inline size_t _pool_size_in_bytes(size_t element_size_in_bytes)
{
	return sizeof(MemoryPool) + element_size_in_bytes * (UINT16_MAX + 1);
}

static void _mempool_init(MemoryPool *pool, size_t element_size_in_bytes)
{
	for (size_t i = 0; i <= UINT16_MAX; i++) {
		pool->stack[i] = (uint16_t)i;
	}
	pool->sp = UINT16_MAX;
	pool->element_size_in_bytes = element_size_in_bytes;
}

// 0x7ffff7c4803a
static void *_mempool_alloc(MemoryPool *pool)
{
	if (pool->sp == 0)
		return NULL;

	size_t offset = pool->stack[pool->sp--] * pool->element_size_in_bytes;
	void *ptr = (void *)&(pool->mem[pool->stack[pool->sp--] * pool->element_size_in_bytes]);
	return ptr;
}

static void _mempool_free(MemoryPool *pool)
{
	return ;
}

static void _memlake_init(MemoryLake *lake, MemoryPool *first_pool, size_t element_size_in_bytes)
{
	lake->sp = 1;
	lake->stack[1] = 0;
	lake->pools[0] = first_pool;
	lake->element_size_in_bytes = element_size_in_bytes;
	lake->num_pools = 1;
	lake->num_allocated = 0;
}

static void _memlake_add_pool(MemoryLake *lake)
{
	size_t pool_size_in_bytes = _pool_size_in_bytes(lake->element_size_in_bytes);

	MemoryPool *pool = (MemoryPool *)malloc(pool_size_in_bytes);
	assert(pool != NULL); // TODO hmh
	_mempool_init(pool, lake->element_size_in_bytes);
}

MemoryLake *memlake_create(size_t element_size_in_bytes)
{
	size_t pool_size_in_bytes = _pool_size_in_bytes(element_size_in_bytes);

	void *mem = malloc(sizeof(MemoryLake) + pool_size_in_bytes);
	if (mem == NULL)
		return NULL;

	MemoryLake *lake = (MemoryLake *)mem;
	MemoryPool *pool = (MemoryPool *)((size_t)mem + sizeof(MemoryLake));
	_mempool_init(pool, element_size_in_bytes);
	_memlake_init(lake, pool, element_size_in_bytes);
	return lake;
}

void memlake_destroy(MemoryLake *lake)
{
	return ;
}

void *memlake_alloc(MemoryLake *lake)
{
	void *ptr = NULL;
	do {
		MemoryPool *pool = lake->pools[lake->stack[lake->sp]];
		ptr = _mempool_alloc(pool);
		if (ptr == NULL) {
			lake->sp--;
			if (lake->sp == 0) _memlake_add_pool(lake);
			continue ;
		}
		return ptr;
	} while (1);
}

void memlake_free(MemoryLake *lake, void *ptr)
{
	return ;
}


#if defined(TEST) | defined(BENCH)
# include <sys/sysinfo.h>
# include <time.h>
# include <stdio.h>

# define NUM_ALLOC 1000000

typedef struct {
	float x;
	float y;
} Coordinates;

void time_me(char identifier[], void(*fn)(void))
{
    struct timespec start, end;
    double time_taken;

    clock_gettime(CLOCK_MONOTONIC, &start);
	fn();
    clock_gettime(CLOCK_MONOTONIC, &end);

    time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("[%s] Time taken to allocate %lu objects: %f seconds\n", identifier, NUM_ALLOC, time_taken);
}

void lot_of_alloc_memlake(void)
{
	MemoryLake *lake = memlake_create(sizeof(Coordinates));
	for (size_t i = 0; i < NUM_ALLOC; i++) {
		Coordinates *coord = memlake_alloc(lake);
		assert(coord != NULL);
#ifndef BENCH
		coord->x = 0.0;
		coord->y = 0.0;
#endif
#ifdef TEST
		printf("%p\n", coord);
#endif
	}
}

void lot_of_alloc_malloc(void)
{
	MemoryLake *lake = memlake_create(sizeof(Coordinates));
	for (size_t i = 0; i < NUM_ALLOC; i++) {
		Coordinates *coord = malloc(sizeof(Coordinates));
		assert(coord != NULL);
#ifndef BENCH
		coord->x = 0.0;
		coord->y = 0.0;
#endif
	}
}

void bench(void)
{
	time_me("memlake", lot_of_alloc_memlake);
	time_me("malloc", lot_of_alloc_malloc);
}

int main(void)
{
#ifdef BENCH
	bench();
#else
	lot_of_alloc_memlake();
#endif
}

#endif
