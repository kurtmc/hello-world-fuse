#!/bin/bash

mkdir -p testdir
fusermount -u testdir > /dev/null

make
make tests

./hello testdir

./file_tests

fusermount -u testdir > /dev/null
