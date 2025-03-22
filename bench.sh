#!/bin/bash

gcc -DBENCH=1 memlake.c -O3 -mavx2 && ./a.out && rm a.out