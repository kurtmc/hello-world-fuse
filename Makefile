all:
	gcc -g -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
