//
// Created by David Neškrabal on 28.10.2025.
//

#include <time.h>
#include <string.h>

#include "FileManagement.h"
#include "FileSystemStructure.h"

void read_block(uint32_t block_num, void *buf) {
    fseek(disk, block_num * BLOCK_SIZE, SEEK_SET);
    fread(buf, BLOCK_SIZE, 1, disk);
}

void write_block(uint32_t block_num, const void *buf) {
    fseek(disk, block_num * BLOCK_SIZE, SEEK_SET);
    fwrite(buf, BLOCK_SIZE, 1, disk);
}

void sync_superblock() {
    fseek(disk, 0, SEEK_SET);
    fwrite(&sb, sizeof(sb), 1, disk);
}

int alloc_block() {
    for (int i = 0; i < sb.total_blocks; i++) {
        if (block_bitmap[i] == 0) {
            block_bitmap[i] = 1;
            sb.free_blocks -= 1;
            sync_superblock();
            return i;
        }
    }
    return -1;
}

int alloc_inode() {
    for (int i = 0; i < sb.total_inodes; i++) {
        if (inode_bitmap[i] == 0) {
            inode_bitmap[i] = 1;
            sb.free_inodes -= 1;
            sync_superblock();
            return i;
        }
    }
    return -1;
}

void free_block(int b) {
    block_bitmap[b] = 0;
    sb.free_blocks += 1;
    sync_superblock();
}

void free_inode(int i) {
    inode_bitmap[i] = 0;
    sb.free_inodes += 1;
    sync_superblock();
}

int create_inode(uint16_t mode)
{
    int inode_num = alloc_inode();
    if (inode_num == -1)
    {
        return -1;
    }
    Inode new_inode;
    memset(&new_inode, 0, sizeof(Inode));

    new_inode.mode = mode;

    uint32_t now = time(NULL);
    new_inode.atime = now;
    new_inode.mtime = now;
    new_inode.ctime = now;

    inode_table[inode_num] = new_inode;

    return inode_num;
}