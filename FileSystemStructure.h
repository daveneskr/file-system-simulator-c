//
// Created by David Neškrabal on 20.10.2025.
//

#ifndef FILESYSTEMSTRUCTURE_H
#define FILESYSTEMSTRUCTURE_H
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE 4096
#define MAX_INODES 64

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

typedef struct {
    uint16_t mode;              // permissions / type
    uint16_t links_count;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;              // in bytes
    uint32_t atime;
    uint32_t mtime;
    uint32_t ctime;

    uint32_t direct[DIRECT_PTRS];   // 12 direct block pointers
    uint32_t indirect;              // single indirect
    uint32_t double_indirect;       // double indirect
} Inode;

#define NAME_MAX 64

typedef struct {
    uint32_t inode_num;
    char name[NAME_MAX];
} DirEntry;

FILE *disk;  // your "virtual disk" file
Superblock sb;

void format_disk(const char *filename, uint32_t num_blocks);

void initialize_bitmap();

uint8_t block_bitmap[BLOCK_SIZE];
uint8_t inode_bitmap[BLOCK_SIZE];

typedef struct {
    Superblock sb;
    FILE *disk;
    char mounted;
} FileSystem;

#endif //FILESYSTEMSTRUCTURE_H
