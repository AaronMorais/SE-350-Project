#!/bin/bash -e

gcc priority_queue_test.c priority_queue.c -o priority_queue_test --std=c99
./priority_queue_test
