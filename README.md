# memlake

My experiments with different allocating strategies, in this case, fast object allocation / slow deallocation for fixed size object by leveraging SIMD instructions.

## Try it

Benchmark
```sh
$ ./bench.sh # Run on my 14600k ~20x
[memlake] Time taken to allocate 10000000 objects: 0.006171 seconds
[malloc] Time taken to allocate 10000000 objects: 0.146438 seconds
```

Test
```sh
$ ./test.sh
Expected count (NUM_ALLOC): 10000000
Actual count: 10000000
SUCCESS: The counts match!
```

## API

```c
MemoryLake *memlake_create(size_t element_size_in_bytes);
void memlake_destroy(MemoryLake *lake); // nope
void *memlake_alloc(MemoryLake *lake);
void memlake_free(MemoryLake *lake, void *ptr); // nope
```
