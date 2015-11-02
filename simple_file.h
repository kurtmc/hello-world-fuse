#ifndef SIMPLE_FILE_H_
#define SIMPLE_FILE_H_

#include <fuse.h>

struct simple_file {
	char *path;
	mode_t mode;
	int hard_links;
	char *file_contents;
	size_t file_length;
};

struct simple_file *create_file_struct(const char *path, mode_t mode, int
		hard_links, char *file_contents, int file_length);

void free_file(struct simple_file *f);

#endif /* SIMPLE_FILE_H_ */
