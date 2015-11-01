/*
 *   FUSE: Filesystem in Userspace
 *     Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>
 *
 *       This program can be distributed under the terms of the GNU GPL.
 *         See the file COPYING.
 *
 *           gcc -Wall hello.c `pkg-config fuse --cflags --libs` -o hello
 *           */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

/*
static char **file_names;
static mode_t *modes;
static int *hard_links;
static char **file_contents;
static unsigned int *file_length;
*/

static int num_files;
static struct simple_file **files;

struct simple_file {
	char *path;
	mode_t mode;
	int hard_links;
	char *file_contents;
	int file_length;
};

struct simple_file *create_file_struct(const char *path, mode_t mode, int hard_links, char *file_contents, int file_length) {
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

void setup_files()
{
	/*
	// Modify these values to create more/less/different files
	char *stack_filenames[] = { "/file_1", "/file_2", "/file_3" };
	char *stack_file_contents[] = { "contents of file_1\n", "contents of file_2\n", "contents of file_3\n" };
	mode_t stack_modes[] = { S_IFREG | 0666, S_IFREG | 0444, S_IFREG | 0444};
	int stack_hard_links[] = {1, 1, 1};

	// Setup modes
	modes = malloc(num_files * sizeof(mode_t));
	for (int i = 0; i < num_files; i++) {
		modes[i] = stack_modes[i];
	}
	// Setup hard links
	hard_links = malloc(num_files * sizeof(int));
	for (int i = 0; i < num_files; i++) {
		hard_links[i] = stack_hard_links[i];
	}
	// Setup file names
	file_names = malloc(num_files * sizeof(char *));
	for (int i = 0; i < num_files; i++) {
		file_names[i] = strdup(stack_filenames[i]);
	}
	// Setup file contents
	file_contents = malloc(num_files * sizeof(char *));
	file_length = malloc(num_files * sizeof(unsigned int));
	for (int i = 0; i < num_files; i++) {
		file_contents[i] = strdup(stack_file_contents[i]);
		file_length[i] = strlen(stack_file_contents[i]);
	}
	*/

	// Setup simple files
	num_files = 3;
	files = malloc(num_files * sizeof(struct simple_file *));
	char *contents = "contents of file_1\n";
	files[0] = create_file_struct("/file_1", S_IFREG | 0666, 1, contents, strlen(contents));
	contents = "contents of file_2\n";
	files[1] = create_file_struct("/file_2", S_IFREG | 0666, 1, contents, strlen(contents));
	contents = "contents of file_3\n";
	files[2] = create_file_struct("/file_3", S_IFREG | 0666, 1, contents, strlen(contents));
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	} else {
		/* check if in list */
		for (int i = 0; i < num_files; i++) {
			if (strcmp(path, files[i]->path) == 0) {
				stbuf->st_mode = files[i]->mode;
				stbuf->st_nlink = files[i]->hard_links;
				stbuf->st_size = files[i]->file_length;
				return res;
			}
		}
	}
	res = -ENOENT;
	return res;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for (int i = 0; i < num_files; i++) {
		filler(buf, files[i]->path + 1, NULL, 0);
	}

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	for (int i = 0; i < num_files; i++) {
		if (strcmp(path, files[i]->path) == 0)
			return 0;
	}

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return -ENOENT;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	for (int i = 0; i < num_files; i++) {
		if(strcmp(path, files[i]->path) == 0) {
			len = files[i]->file_length;
			if (offset < len) {
				if (offset + size > len)
					size = len - offset;
				memcpy(buf, files[i]->file_contents + offset, size);
			} else
				size = 0;

			return size;
		}
	}
	return -ENOENT;

}

static int hello_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	for (int i = 0; i < num_files; i++) {
		if(strcmp(path, files[i]->path) == 0) {
			if ((size + offset) > files[i]->file_length) { /* larger, so realloc */
				files[i]->file_contents = realloc(files[i]->file_contents, size + offset);
				files[i]->file_length = size + offset;
			} else if ((size + offset) < files[i]->file_length) { /* smaller, so realloc */
				files[i]->file_contents = realloc(files[i]->file_contents, size + offset);
				files[i]->file_length = size + offset;
			}

			len = files[i]->file_length;
			if (offset < len) {
				if (offset + size > len)
					size = len - offset;
				memcpy(files[i]->file_contents + offset, buf, size);
			} else
				size = 0;

			return size;
		}
	}
	return -ENOENT;
}

/* Empty implementations for utime, chown, chmod and truncate so that I can have
 * a basic implementation of the write function. Without these methods you get a
 * FUSE error SETATTR function not implemented. */
static int hello_utime(const char *path, struct utimbuf *t)
{
	return 0;
}

static int hello_chown(const char *path, uid_t uid, gid_t gid)
{
	return 0;
}
static int hello_chmod(const char *path, mode_t mode)
{
	return 0;
}

static int hello_truncate(const char *path, off_t size)
{
	return 0;
}

static int hello_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	/*
	file_names = realloc(file_names, num_files * sizeof(char *));
	file_names[num_files - 1] = strdup(path);

	modes = realloc(modes, num_files * sizeof(mode_t));
	modes[num_files - 1] = mode;

	hard_links = realloc(hard_links, num_files * sizeof(int));
	hard_links[num_files - 1] = 1;

	file_contents = realloc(file_contents, num_files * sizeof(char *));
	file_contents[num_files - 1] = strdup("");

	file_length = realloc(file_length, num_files * sizeof(unsigned int));
	file_length[num_files - 1] = 0;
	*/

	num_files++;

	files = realloc(files, num_files * sizeof(struct simple_file *));
	files[num_files - 1] = create_file_struct(path, mode, 1, "", 0);

	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
	.write		= hello_write,
	.utime		= hello_utime,
	.chmod		= hello_chmod,
	.chown		= hello_chown,
	.truncate	= hello_truncate,
	.create		= hello_create,
};

int main(int argc, char *argv[])
{
	setup_files();
	return fuse_main(argc, argv, &hello_oper, NULL);
}
