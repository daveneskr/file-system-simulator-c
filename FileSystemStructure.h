//
// Created by David Neškrabal on 20.10.2025.
//

#ifndef FILESYSTEMSTRUCTURE_H
#define FILESYSTEMSTRUCTURE_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE 4096
#define MAX_INODES 512

typedef struct
{
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t free_inodes;
    uint32_t free_blocks;
    uint32_t block_size;
    uint32_t inode_start;
    uint32_t block_bitmap_start;
    uint32_t inode_bitmap_start;
    uint32_t data_block_start;
    uint32_t root_inode;
} Superblock;

#define DIRECT_PTRS 12

// Inode modes
#define IREG  0x8000   // regular file
#define IDIR  0x4000   // directory

#define IRUSR 0x0100   // owner read
#define IWUSR 0x0080   // owner write
#define IXUSR 0x0040   // owner execute

typedef struct {
    uint16_t mode;              // permissions / type
    uint16_t links_count;
    uint32_t size;              // in bytes
    uint32_t atime;
    uint32_t mtime;
    uint32_t ctime;

    uint32_t direct[DIRECT_PTRS];   // 12 direct block pointers
    uint32_t indirect;              // single indirect
    uint32_t double_indirect;       // double indirect
} Inode;

FILE *disk;  // your "virtual disk" file
Superblock sb;

void format_disk(const char *filename, uint32_t num_blocks);

uint8_t block_bitmap[BLOCK_SIZE];
uint8_t inode_bitmap[MAX_INODES];

void initialize_bitmap();

typedef struct {
    Superblock sb;
    FILE *disk;
    char mounted;
} FileSystem;

int update_inode_bitmap(uint32_t inode_num, uint8_t used);

int update_block_bitmap(uint32_t block_num, uint8_t used);

#endif //FILESYSTEMSTRUCTURE_H
