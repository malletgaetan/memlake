#include "memlake.h"

static inline size_t _pool_size_in_bytes(size_t element_size_in_bytes)
{
	return sizeof(MemoryPool) + element_size_in_bytes * (UINT16_MAX + 1);
}

static void _mempool_init(MemoryPool *pool, size_t element_size_in_bytes)
{
	const int vector_width = 16;
	__m256i increment = _mm256_set1_epi16(vector_width);

#if defined(__AVX2__)
	__m256i current = _mm256_set_epi16(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	for (size_t i = 0; i <= UINT16_MAX; i += vector_width) {
		_mm256_storeu_si256((__m256i *)&pool->stack[i], current);
		current = _mm256_add_epi16(current, increment);
	}
#else
	uint16_t free = 0;
	for (size_t i = 0; i <= UINT16_MAX; i++) {
		pool->stack[i] = free++;
	}
#endif
	pool->sp = 0;
	pool->element_size_in_bytes = element_size_in_bytes;
	pool->next = NULL;
}

static void *_mempool_alloc(MemoryPool *pool)
{
	if (pool->sp > UINT16_MAX)
		return NULL;

	size_t offset = pool->stack[pool->sp++] * pool->element_size_in_bytes;
	return (void *)&(pool->mem[offset]);
}

static void _mempool_free(MemoryPool *pool)
{
	return;
}

static void _memlake_init(MemoryLake *lake, MemoryPool *first_pool, size_t element_size_in_bytes)
{
	lake->element_size_in_bytes = element_size_in_bytes;
	lake->num_pools = 1;
	lake->num_allocated = 0;
	lake->free_pool_list = first_pool;
	lake->full_pool_list = NULL;
}

static void _memlake_add_pool(MemoryPool **next_ptr, size_t element_size_in_bytes)
{
	size_t pool_size_in_bytes = _pool_size_in_bytes(element_size_in_bytes);

	MemoryPool *pool = (MemoryPool *)malloc(pool_size_in_bytes);
	assert(pool != NULL); // TODO hmh
	_mempool_init(pool, element_size_in_bytes);
	*next_ptr = pool;
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
	return;
}

void *memlake_alloc(MemoryLake *lake)
{
	void *ptr = NULL;
	MemoryPool *pool = lake->free_pool_list;
	MemoryPool *next_free;
	do {
		ptr = _mempool_alloc(pool);
		if (ptr != NULL)
			return ptr;

		// put the pool in the full list
		next_free = pool->next;
		pool->next = lake->full_pool_list;
		lake->full_pool_list = pool;

		// if no free pool left, allocate a new pool
		if (next_free == NULL) {
			_memlake_add_pool(&next_free, lake->element_size_in_bytes);
			lake->free_pool_list = next_free;
		}
		pool = next_free;
	} while (1);
}

void memlake_free(MemoryLake *lake, void *ptr)
{
	return;
}

#if defined(TEST) | defined(BENCH)
#include <stdio.h>
#include <sys/sysinfo.h>
#include <time.h>

#define NUM_ALLOC 10000000

typedef struct {
	float x;
	float y;
} Coordinates;

void time_me(char identifier[], void (*fn)(void))
{
	struct timespec start, end;
	double time_taken;

	clock_gettime(CLOCK_MONOTONIC, &start);
	fn();
	clock_gettime(CLOCK_MONOTONIC, &end);

	time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

	printf("[%s] Time taken to allocate %lu objects: %f seconds\n", identifier, NUM_ALLOC,
		   time_taken);
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
