#define FUSE_USE_VERSION 26
#include "tif.h"

static tif_superblock superblock = {
    .vfs = {
        .f_bsize = BLOCKSIZE,
        .f_blocks = MAX_NUM,
        .f_files = MAX_NUM,
        .f_namemax = MAX_NAME_LEN
    }
};
static tif_inode_bitmap inodeBitmap = {.used = 0};
static tif_datablock_bitmap datablockBitmap = {.used = 0};
static tif_inode_space inodeSpace = {.used = 0};
static tif_data_space dataSpace = {.used = 0};
static tif_dcache dcache = { .used = 0 };

/** Helper functions */

int getFirstUsableInode() {
    int rst = -ENOENT;
    if (inodeBitmap.used == MAX_NUM) goto out;    
    for (int i = 0; i < MAX_NUM; ++i) {
        if (inodeBitmap.map[i] == false) {
            inodeBitmap.map[i] = true;
            inodeBitmap.used++;
            rst = i;
            LOG("Return inode num %d, index %d", rst, i);
            break;
        }
    }
out:
    if (rst == -ENOENT) {
        LOG("All %d inodes used", inodeBitmap.used);
    }
    return rst;
}

int indexInDcache(const char* path) {
    int rst = -ENOENT;
    for (int i = 0; i < dcache.used; ++i) {
        if (strcmp(path, dcache.cache[i].path) == 0) {
            rst  = i;
            break;
        }
    }
    LOG("Lookup %s in dcache, dcache index %d", path, rst);
    return rst;
}

/** FUSE Operations */

// stat -f
static int tif_statfs(const char *path, struct statvfs *outbuf)
{
    LOG("path: %s", path);
    struct statvfs *vfs = &superblock.vfs;
    vfs->f_ffree = MAX_NUM - inodeBitmap.used;
    vfs->f_favail = vfs->f_ffree;
    vfs->f_bfree = MAX_NUM - datablockBitmap.used;
    vfs->f_bavail = vfs->f_bfree;
    vfs->f_fsid = FS_ID;
    LOG("path: %s, bsize: %ld, blocks: %ld, favail: %ld, ffree: %ld, f_bavail: %ld", path, vfs->f_bsize, vfs->f_blocks, vfs->f_favail, vfs->f_ffree, vfs->f_bavail);
    memcpy(outbuf, vfs, sizeof(*vfs));
    return 0;
}

// list or read subfiles of a directory
static int tif_readdir(const char *path, void *outbuf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    LOG("path: %s", path);
    if (strcmp(path, "/") != 0) return -EPERM;
    for (int i = 0; i < dcache.used; ++i) {
        if (dcache.cache[i].active) {
            LOG("To filler dcache index %d, path %s", i, dcache.cache[i].path);
            filler(outbuf, dcache.cache[i].path, &dcache.cache[i].inode->attr, 0);
        }            
    }
    return 0;
}

// get a file's attribute
static int tif_getattr(const char *path, struct stat *outbuf)
{
    LOG("path: %s", path);
    if (strcmp(path, "/") == 0)
        outbuf->st_mode = 0777 | S_IFDIR;
    else {
        if(path[0] == '/') path++;
        int cahIdx = indexInDcache(path);
        if (cahIdx == -ENOENT) return -ENOENT;
        //LOG("inpout inode %d", outbuf->st_ino); // no inode info
        memcpy(outbuf, &(dcache.cache[cahIdx].inode->attr), sizeof(*outbuf));
    }
    return 0;
}

// mkdir
int tif_mkdir(const char* path, mode_t mode) {
    LOG("path %s, mode %04o", path, mode);
    if(path[0] == '/') path++;

    int cahIdx = indexInDcache(path);
    if (cahIdx != -ENOENT) return -EEXIST;
    int inodeIdx = getFirstUsableInode();
    if (inodeIdx == -ENOENT) return -EPERM;

    tif_inode* inode = &inodeSpace.data[inodeIdx];
    memset(inode, 0, sizeof(tif_inode));
    inode->attr.st_mode = mode | S_IFDIR;
    inode->attr.st_ino = USER_FIRST_INODE_NUM + inodeIdx; // not used by fuse kernel (hight level apis), although we set it here
    inode->attr.st_nlink = 1;
    inode->attr.st_blksize = BLOCKSIZE;
    inode->attr.st_blocks = 1;
    inodeSpace.used++;

    memset(dcache.cache[inodeIdx].path, 0, MAX_NAME_LEN);
    strcpy(dcache.cache[inodeIdx].path, path);
    dcache.cache[inodeIdx].inode = inode;
    dcache.cache[inodeIdx].active = true;
    dcache.used++;

    return 0;
}

// create general file
int tif_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    LOG("path %s, mode %04o", path, mode);
    if(path[0] == '/') path++;

    int cahIdx = indexInDcache(path);
    if (cahIdx != -ENOENT) return -EEXIST;
    int inodeIdx = getFirstUsableInode();
    if (inodeIdx == -ENOENT) return -EPERM;

    tif_inode* inode = &inodeSpace.data[inodeIdx];
    memset(inode, 0, sizeof(tif_inode));
    inode->attr.st_mode = mode;
    inode->attr.st_ino = USER_FIRST_INODE_NUM + inodeIdx;
    inode->attr.st_nlink = 1;
    inode->attr.st_blksize = BLOCKSIZE;
    inode->attr.st_blocks = 1;
    inode->attr.st_size = 0;
    inodeSpace.used++;

    memset(dcache.cache[inodeIdx].path, 0, MAX_NAME_LEN);
    strcpy(dcache.cache[inodeIdx].path, path);
    dcache.cache[inodeIdx].inode = inode;
    dcache.cache[inodeIdx].active = true;
    dcache.used++;

    return 0;
}

static struct fuse_operations tif_ops = {
    .statfs = tif_statfs,
    .readdir = tif_readdir,
    .getattr = tif_getattr,
    .mkdir = tif_mkdir,
    .create = tif_create,
};

FILE *logfd;

int main(int argc, char *argv[])
{
    logfd = fopen("./fs.log", "w");
    int ret = 0;
    ret = fuse_main(argc, argv, &tif_ops, NULL);
    return ret;
}
