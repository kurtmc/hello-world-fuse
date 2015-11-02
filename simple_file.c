#include "simple_file.h"
#include <stdlib.h>
#include <string.h>

struct simple_file *create_file_struct(const char *path, mode_t mode, int hard_links, char *file_contents, int file_length)
{
	struct simple_file *f;

	f = malloc(sizeof(struct simple_file));
	f->path = strdup(path);
	f->mode = mode;
	f->hard_links = hard_links;
	f->file_contents = malloc(file_length * sizeof(char));
	strncpy(f->file_contents, file_contents, file_length);
	f->file_length = file_length;

	return f;
}

void free_file(struct simple_file *f) {
	free(f->path);
	free(f->file_contents);
	free(f);
}
