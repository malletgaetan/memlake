# memlake

My experiments with building fast object allocator.

## Run it

Benchmark
```console
>$ gcc -BENCH=1 memlake.c -O3 && ./a.out
[memlake] Time taken to allocate 1000000 objects: 0.000560 seconds
[malloc] Time taken to allocate 1000000 objects: 0.018569 seconds
```

Test
```sh
gcc -DTEST=1 memlake.c -O3
./a.out | uniq | wc -l # should be equal to NUM_ALLOC
```
