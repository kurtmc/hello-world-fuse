#!/bin/bash

fusermount -u /home/kurt/workspace/hello-world-fuse/testdir > /dev/null

make
make tests

./hello testdir

./file_tests

fusermount -u /home/kurt/workspace/hello-world-fuse/testdir > /dev/null
