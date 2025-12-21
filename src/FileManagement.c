//
// Created by David Neškrabal on 28.10.2025.
//

#include "../include/FileManagement.h"

#include <time.h>
#include <string.h>

void read_block(uint32_t block_num, void *buf) {
    fseek(fs.disk, block_num * BLOCK_SIZE, SEEK_SET);
    fread(buf, BLOCK_SIZE, 1, fs.disk);
}

void write_block(uint32_t block_num, const void *buf) {
    fseek(fs.disk, block_num * BLOCK_SIZE, SEEK_SET);
    fwrite(buf, BLOCK_SIZE, 1, fs.disk);
}

// writes current superblock state stored in cache to disk
void sync_superblock() {
    fseek(fs.disk, 0, SEEK_SET); // set file pointer to first block
    fwrite(&fs.sb, sizeof(fs.sb), 1, fs.disk); // write superblock
}

// finds free block, allocates it and returns the block number
int alloc_block() {
    // check if there are any free blocks
    if (fs.sb.free_blocks > 0) {
        // start at data blocks, as before are taken at disk formating
        // iterate through bitmap
        for (int i = fs.sb.data_block_start; i < fs.sb.total_blocks; i++) {
            // check if block free
            if (block_bitmap[i] == 0) {
                update_block_bitmap(i , 1); // mark used in bitmap
                fs.sb.free_blocks -= 1;          // decrement num of free blocks
                sync_superblock();
                return i;   // block number
            }
        }
    }
    return -1; // if no free blocks found
}

// finds free inode, allocates it and returns the inode number
int alloc_inode() {
    for (int i = 0; i < fs.sb.total_inodes; i++) {
        if (inode_bitmap[i] == 0) {
            update_inode_bitmap(i, 1);
            fs.sb.free_inodes -= 1;
            sync_superblock();
            return i;
        }
    }
    return -1;
}

void free_block(uint32_t b) {
    block_bitmap[b] = 0;
    fs.sb.free_blocks += 1;
    sync_superblock();
}

void free_inode(uint32_t i) {
    inode_bitmap[i] = 0;
    fs.sb.free_inodes += 1;
    sync_superblock();
}

uint32_t create_inode(uint16_t mode)
{
    int inode_num = alloc_inode();
    if (inode_num == -1)
    {
        return 0;
    }
    Inode new_inode;
    memset(&new_inode, 0, sizeof(Inode));

    new_inode.mode = mode;

    uint32_t now = time(NULL);
    new_inode.atime = now;
    new_inode.mtime = now;
    new_inode.ctime = now;

    write_inode(inode_num, &new_inode);

    return inode_num;
}

int write_inode(uint32_t inode_num, Inode *new_inode) {
    int inode_per_block = BLOCK_SIZE / sizeof(Inode);

    uint32_t inode_idx = inode_num % inode_per_block;
    uint32_t block_idx = inode_num / inode_per_block + fs.sb.inode_start;

    uint32_t offset = block_idx * BLOCK_SIZE + inode_idx * sizeof(Inode);

    fseek(fs.disk, offset, SEEK_SET);
    fwrite(new_inode, sizeof(Inode), 1, fs.disk);

    return 0;
}

int read_inode(uint32_t inode_num, Inode *out_inode) {
    int inode_per_block = BLOCK_SIZE / sizeof(Inode);

    uint32_t inode_idx = inode_num % inode_per_block;
    uint32_t block_idx = inode_num / inode_per_block + fs.sb.inode_start;

    uint32_t offset = block_idx * BLOCK_SIZE + inode_idx * sizeof(Inode);

    fseek(fs.disk, offset, SEEK_SET);
    fread(out_inode, sizeof(Inode), 1, fs.disk);

    return 0;
}