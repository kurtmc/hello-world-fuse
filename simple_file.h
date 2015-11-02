#ifndef SIMPLE_FILE_H_
#define SIMPLE_FILE_H_

#include <fuse.h>

struct simple_directory {
	char *path;
	mode_t mode;

	int num_files;
	int num_directories;
	struct simple_directory **directories;
	struct simple_file **files;
};

struct simple_file {
	char *path;
	mode_t mode;
	int hard_links;
	char *file_contents;
	size_t file_length;
};

struct simple_file *find_file(struct simple_directory *d, const char *path);
struct simple_directory *create_directory(const char *path, mode_t mode);
void add_file(struct simple_directory *d, struct simple_file *f);
int remove_file(struct simple_directory *d, const char *path);

struct simple_file *create_file_struct(const char *path, mode_t mode, int
		hard_links, char *file_contents, int file_length);

void free_file(struct simple_file *f);

#endif /* SIMPLE_FILE_H_ */
