//
// Created by David Neškrabal on 28.10.2025.
//
#include "FileSystemStructure.h"

#include <stdlib.h>

void format_disk(const char *filename, uint32_t num_blocks) {
    disk = fopen(filename, "wb");
    if (!disk) {
        perror("fopen");
        exit(1);
    }

    // Step 1: zero-fill the disk
    uint8_t zero_block[BLOCK_SIZE] = {0};
    for (uint32_t i = 0; i < num_blocks; i++) {
        fwrite(zero_block, BLOCK_SIZE, 1, disk);
    }

    // Step 2: initialize superblock
    sb.total_blocks = num_blocks;
    sb.block_size = BLOCK_SIZE;
    sb.total_inodes = MAX_INODES;
    sb.free_blocks = num_blocks - 7;   // for example: first 10 reserved
    sb.free_inodes = MAX_INODES - 1;    // root inode used
    sb.root_inode = 0;
    sb.inode_start = 2;
    sb.data_block_start = 10;

    // Step 3: write superblock at block 0
    fseek(disk, 0, SEEK_SET);
    fwrite(&sb, sizeof(Superblock), 1, disk);

    initialize_bitmap();

    printf("Disk formatted: %s (%u blocks)\n", filename, num_blocks);
}

void initialize_bitmap() {
    block_bitmap[0] = 1; // superblock
    block_bitmap[1] = 1; // block bitmap space
    block_bitmap[2] = 1; // inode bitmap space
    block_bitmap[3] = 1; // inode table space 1/4
    block_bitmap[4] = 1;
    block_bitmap[5] = 1;
    block_bitmap[6] = 1; // inode table space 4/4
}
