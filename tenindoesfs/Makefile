
all: fuse_teninodesfs

fuse_teninodesfs: main.c
	gcc -g -o $@ $+ -D_FILE_OFFSET_BITS=64 -lfuse 

clean:
	rm fuse_teninodesfs
