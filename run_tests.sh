#!/bin/bash -e

gcc priority_queue_test.c priority_queue.c -o priority_queue_test --std=c99 -g
./priority_queue_test
gcc heap_test.c heap.c -o heap_test --std=c99 -g
./heap_test
