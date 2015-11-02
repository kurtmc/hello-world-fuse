CFLAGS := -g -Wall `pkg-config fuse --cflags --libs`

hello: hello.o simple_file.o
	gcc $(CFLAGS) -o hello hello.o simple_file.o
	
hello.o: hello.c simple_file.h
	gcc $(CFLAGS) -c hello.c

simple_file.o: simple_file.c simple_file.h
	gcc $(CFLAGS) -c simple_file.c

tests:
	gcc $(CFLAGS) file_tests.c -o file_tests
