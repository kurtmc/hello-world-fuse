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

To use the debugger
```bash
$ mkdir testdir         # if necessary
$ gdb hello
[gdb noise]
(gdb) [set breakpoints, etc.]
(gdb) run -s -d testdir
```
