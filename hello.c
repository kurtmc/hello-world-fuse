#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include "simple_file.h"


static struct simple_directory *root_directory;

/* May return NULL */

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}


	struct simple_file *f = find_file(root_directory, path);

	if (f == NULL)
		return -ENOENT; /* cant find file */
	stbuf->st_mode = f->mode;
	stbuf->st_nlink = f->hard_links;
	stbuf->st_size = f->file_length;
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
	for (int i = 0; i < root_directory->num_files; i++)
		filler(buf, root_directory->files[i]->path + 1, NULL, 0);

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (find_file(root_directory, path))
		return 0;

	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return -ENOENT;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;

	struct simple_file *f = find_file(root_directory, path);

	if (f == NULL)
		return -ENOENT;

	len = f->file_length;
	if (offset < (off_t) len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(buf, f->file_contents + offset, size);
	} else
		size = 0;

	return size;
}

static int hello_write(const char *path, const char *buf, size_t size, off_t
		offset, struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;


	struct simple_file *f = find_file(root_directory, path);

	if (f == NULL)
		return -ENOENT;

	/* larger, so realloc */
	if ((size + offset) > f->file_length) {
		f->file_contents = realloc(f->file_contents, size + offset);
		f->file_length = size + offset;
	/* smaller, so realloc */
	} else if ((size + offset) < f->file_length) {
		f->file_contents = realloc(f->file_contents, size + offset);
		f->file_length = size + offset;
	}

	len = f->file_length;
	if (offset < (off_t) len) {
		if (offset + size > len)
			size = len - offset;
		memcpy(f->file_contents + offset, buf, size);
	} else
		size = 0;

	return size;

}

/* Empty implementations for utime, chown, chmod and truncate so that I can have
 * a basic implementation of the write function. Without these methods you get a
 * FUSE error SETATTR function not implemented.
 */
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
	struct simple_file *f = find_file(root_directory, path);

	if (f == NULL)
		return -ENOENT;

	f->file_length = size;

	f->file_contents = realloc(f->file_contents, size);


	return 0;
}

static int hello_unlink(const char *path)
{
	return remove_file(root_directory, path);
}

static int hello_create(const char *path, mode_t mode, struct fuse_file_info
		*info)
{
	add_file(root_directory, create_file_struct(path, mode, 1, "", 0));
	return 0;
}

static struct fuse_operations hello_oper = {
	.getattr  = hello_getattr,
	.readdir  = hello_readdir,
	.open     = hello_open,
	.read     = hello_read,
	.write    = hello_write,
	.utime    = hello_utime,
	.chmod    = hello_chmod,
	.chown    = hello_chown,
	.truncate = hello_truncate,
	.create   = hello_create,
	.unlink   = hello_unlink,
};

int main(int argc, char *argv[])
{
	root_directory = create_directory("/", S_IFDIR | 0755);
	return fuse_main(argc, argv, &hello_oper, NULL);
}
