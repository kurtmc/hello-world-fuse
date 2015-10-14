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

static const char *file_names[3] = { "/file_1", "/file_2", "/file_3" };
static const mode_t modes[3] = { S_IFREG | 0444, S_IFREG | 0444, S_IFREG | 0444};
static const int hard_links[3] = { 1, 1, 1 };
static const char *file_contents[3] = { "contents of file_1", "contents of file_2", "contents of file_3" };
static const int num_files = 3;

static const char *hello_str = "Hello World!\n";
static const char *hello_path = "/hello";
static const char *my_path = "/my-new-file";
static const char *my_str = "Fuck yeah, a new file!\n";

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		/* check if in list */
		for (int i = 0; i < num_files; i++) {
			if (strcmp(path, file_names[i]) == 0) {
				stbuf->st_mode = modes[i];
				stbuf->st_nlink = hard_links[i];
				stbuf->st_size = strlen(file_contents[i]);
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
		filler(buf, file_names[i] + 1, NULL, 0);
	}

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	for (int i = 0; i < num_files; i++) {
		if (strcmp(path, file_names[i]) == 0)
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
		if(strcmp(path, file_names[i]) == 0) {
			len = strlen(hello_str);
			if (offset < len) {
				if (offset + size > len)
					size = len - offset;
				memcpy(buf, hello_str + offset, size);
			} else
				size = 0;

			return size;
		}
	}
	return -ENOENT;

}

static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
};

int main(int argc, char *argv[])
{
	return fuse_main(argc, argv, &hello_oper, NULL);
}
