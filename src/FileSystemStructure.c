//
// Created by David Neškrabal on 28.10.2025.
//
#include "../include/FileSystemStructure.h"
#include "../include/FileManagement.h"

#include <stdlib.h>

void format_disk(const char *filename, uint32_t num_blocks) {
    fs.disk = fopen(filename, "wb+");
    if (!fs.disk) {
        perror("fopen");
        exit(1);
    }

    // Step 1: zero-fill the disk
    uint8_t zero_block[BLOCK_SIZE] = {0};
    for (uint32_t i = 0; i < num_blocks; i++) {
        fwrite(zero_block, BLOCK_SIZE, 1,fs.disk);
    }

    // Step 2: initialize superblock
   fs.sb.total_blocks = num_blocks;
   fs.sb.block_size = BLOCK_SIZE;
   fs.sb.total_inodes = MAX_INODES;
   fs.sb.free_blocks = num_blocks - 7;    // 7 reserved
   fs.sb.free_inodes = MAX_INODES;
   fs.sb.block_bitmap_start = 1;
   fs.sb.inode_bitmap_start = 2;
   fs.sb.inode_start = 3;
   fs.sb.data_block_start = 6;

    // Step 3: write superblock at block 0
    fseek(fs.disk, 0, SEEK_SET);
    fwrite(&fs.sb, sizeof(Superblock), 1,fs.disk);

    initialize_bitmap();

    fs.sb.root_inode = create_inode(IDIR | IRUSR | IWUSR | IXUSR); // initialize root inode

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
    inode_bitmap[inode_num] = used;   // mark inode in bitmap

    // calc correct block + inode_num
    uint32_t offset = fs.sb.inode_bitmap_start * BLOCK_SIZE + inode_num;

    // set file pointer and write update
    fseek(fs.disk, offset, SEEK_SET);
    fwrite(&used, 1, 1,fs.disk);

    return 0;
}

int update_block_bitmap(uint32_t block_num, uint8_t used) {
    block_bitmap[block_num] = used;  // mark block in bitmap

    // calc correct block + block_num
    uint32_t offset =fs.sb.block_bitmap_start * BLOCK_SIZE + block_num;

    // set file pointer to offset adn write update
    fseek(fs.disk, offset, SEEK_SET);
    fwrite(&used, 1, 1,fs.disk);

    return 0;
}


