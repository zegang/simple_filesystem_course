#ifndef __TIF_H__
#define __TIF_H__

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <fuse.h>
#include <fuse/fuse_lowlevel.h>
#include <errno.h>

#define FS_ID 1024
#define MAX_NUM 10
#define BLOCKSIZE 1024
#define MAX_NAME_LEN 255
#define USER_FIRST_INODE_NUM 10

typedef struct {
    struct statvfs vfs;
} tif_superblock;

struct tif_block {
    char data[BLOCKSIZE];
};

typedef struct  {
    unsigned int used;
    bool map[MAX_NUM];
} tif_datablock_bitmap;

typedef struct  {
    unsigned int used;
    bool map[MAX_NUM];
} tif_inode_bitmap;

// just a wrap of struct stat
typedef struct {
    struct stat attr;
} tif_inode;

typedef struct {
    unsigned int used;
    tif_inode data[MAX_NUM];
} tif_inode_space;

typedef struct {
    unsigned int used;
    struct tif_block data_blocks[MAX_NUM];
} tif_data_space;

typedef struct {
    char path[MAX_NAME_LEN];
    tif_inode* inode;
    bool active;
} tif_dentry;

typedef struct {
    unsigned used;
    tif_dentry cache[MAX_NUM];
} tif_dcache;

extern FILE *logfd;
#define LOG(fmt, args...)                                 \
    fprintf(logfd, "%s " fmt "\n", __FUNCTION__, ##args); \
    fflush(logfd);

#endif // __TIF_H__