#!/bin/bash

# Extract NUM_ALLOC value from memlake.c
NUM_ALLOC=$(grep -oP '#define\s+NUM_ALLOC\s+\K\d+' memlake.c)

# Compile and run, capturing the count
ACTUAL_COUNT=$(gcc -DTEST=1 memlake.c -O3 -mavx2 && (./a.out | uniq | wc -l) && rm a.out)

# Output results
echo "Expected count (NUM_ALLOC): $NUM_ALLOC"
echo "Actual count: $ACTUAL_COUNT"

# Compare the values
if [ "$NUM_ALLOC" -eq "$ACTUAL_COUNT" ]; then
    echo "SUCCESS: The counts match!"
else
    echo "ERROR: The counts do not match!"
    echo "Difference: $((ACTUAL_COUNT - NUM_ALLOC))"
fi
