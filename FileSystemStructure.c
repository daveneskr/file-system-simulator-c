//
// Created by David Neškrabal on 28.10.2025.
//
#include "FileSystemStructure.h"

#include <stdlib.h>

#include "FileManagement.h"

void format_disk(const char *filename, uint32_t num_blocks) {
    disk = fopen(filename, "wb+");
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
    sb.free_blocks = num_blocks - 7;    // 7 reserved
    sb.free_inodes = MAX_INODES - 1;    // root inode used
    sb.root_inode = 0;
    sb.block_bitmap_start = 1;
    sb.inode_bitmap_start = 2;
    sb.inode_start = 3;
    sb.data_block_start = 6;

    // Step 3: write superblock at block 0
    fseek(disk, 0, SEEK_SET);
    fwrite(&sb, sizeof(Superblock), 1, disk);

    initialize_bitmap();

    // IDIR | IRUSR | IWUSR | IXUSR
    uint32_t root_ino = create_inode(1);

    printf("Disk formatted: %s (%u blocks)\n", filename, num_blocks);
}

void initialize_bitmap() {
    update_block_bitmap(0, 1); // superblock
    update_block_bitmap(1, 1); // block bitmap space
    update_block_bitmap(2, 1); // inode bitmap space
    update_block_bitmap(3, 1); // inode table space 1/4
    update_block_bitmap(4, 1);
    update_block_bitmap(5, 1);
    update_block_bitmap(6, 1); // inode table space 4/4
}

int update_inode_bitmap(uint32_t inode_num, uint8_t used) {
    inode_bitmap[inode_num] = used;
    uint32_t offset = sb.inode_bitmap_start * BLOCK_SIZE + inode_num;

    fseek(disk, offset, SEEK_SET);
    fwrite(&used, 1, 1, disk);

    return 0;
}

int update_block_bitmap(uint32_t block_num, uint8_t used) {
    block_bitmap[block_num] = used;
    uint32_t offset = sb.block_bitmap_start * BLOCK_SIZE + block_num;

    fseek(disk, offset, SEEK_SET);
    fwrite(&used, 1, 1, disk);

    return 0;
}


