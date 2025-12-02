//
// Created by David Neškrabal on 28.10.2025.
//

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
    write_block(0, disk);
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