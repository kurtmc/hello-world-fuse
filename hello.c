#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>


static int num_files = 0;
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

/* May return NULL */
struct simple_file *find_file(const char *path)
{
	for (int i = 0; i < num_files; i++) {
		if (strcmp(path, files[i]->path) == 0) {
			return files[i];
		}
	}
	return NULL;
}

static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));
	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
		return res;
	}

	struct simple_file *f = find_file(path);
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
	for (int i = 0; i < num_files; i++) {
		filler(buf, files[i]->path + 1, NULL, 0);
	}

	return 0;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (find_file(path))
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

	struct simple_file *f = find_file(path);
	if (f) {
		len = f->file_length;
		if (offset < len) {
			if (offset + size > len)
				size = len - offset;
			memcpy(buf, f->file_contents + offset, size);
		} else
			size = 0;

		return size;
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
	struct simple_file *f;
	f = find_file(path);

	if (f == NULL)
		return -ENOENT;

	f->file_length = size;

	f->file_contents = realloc(f->file_contents, size);


	return 0;
}

static int hello_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
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
	return fuse_main(argc, argv, &hello_oper, NULL);
}
