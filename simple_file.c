#include "simple_file.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct simple_file *create_file_struct(const char *path, mode_t mode, int
		hard_links, char *file_contents, int file_length)
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

struct simple_directory *create_directory(const char *path, mode_t mode)
{
	struct simple_directory *d;

	d = malloc(sizeof(struct simple_directory));
	d->path = strdup(path);
	d->mode = mode;
	d->num_files = 0;
	d->num_directories = 0;
	d->files = NULL;
	d->directories = NULL;

	return d;
}

void add_file(struct simple_directory *d, struct simple_file *f)
{
	d->num_files++;

	d->files = realloc(d->files, d->num_files * sizeof(struct simple_file *));
	d->files[d->num_files - 1] = f;
}

int remove_file(struct simple_directory *d, const char *path)
{
	/* Get index for file */
	int index = -1;

	for (int i = 0; i < d->num_files; i++) {
		if (strcmp(path, d->files[i]->path) == 0)
			index = i;
	}

	/* file not found */
	if (index == -1)
		return -ENOENT;

	/* free the memory */
	struct simple_file *to_delete = d->files[index];

	free_file(to_delete);

	/* shift over all the files in the array */
	if (index == d->num_files - 1) {
	} else if (index >= 0) {
		for (int i = index; i < d->num_files - 1; i++)
			d->files[i] = d->files[i + 1];
	}

	/* reallocate memory */
	d->num_files--;
	d->files = realloc(d->files, d->num_files * sizeof(struct simple_file *));

	return 0;
}

void free_file(struct simple_file *f)
{
	free(f->path);
	free(f->file_contents);
	free(f);
}

/* May return NULL */
struct simple_file *find_file(struct simple_directory *d, const char *path)
{
	for (int i = 0; i < d->num_files; i++) {
		if (strcmp(path, d->files[i]->path) == 0)
			return d->files[i];
	}
	return NULL;
}
