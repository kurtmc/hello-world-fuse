You can test out a simple FUSE filesystem like this:

```bash
$ mkdir testdir
$ ./hello testdir
$ ls testdir
hello
$ cat testdir/hello
hello, world
$ fusermount -u testdir
$ rmdir testdir
```
