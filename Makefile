all:
	gcc -g -Wall hello.c `pkg-config fuse --cflags --libs` -o hello

tests:
	gcc -Wall file_tests.c -o file_tests
