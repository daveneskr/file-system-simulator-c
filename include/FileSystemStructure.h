//
// Created by David Neškrabal on 20.10.2025.
//

#ifndef FILESYSTEMSTRUCTURE_H
#define FILESYSTEMSTRUCTURE_H
#include <stdint.h>
#include <stdio.h>

#define BLOCK_SIZE 4096 // in bytes
#define MAX_INODES 512

typedef struct
{
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t free_inodes;
    uint32_t free_blocks;
    uint32_t block_size;
    uint32_t inode_start;           // block number where inode table starts
    uint32_t block_bitmap_start;    // block number where block bitmap is located
    uint32_t inode_bitmap_start;    // block number where inode bitmap is located
    uint32_t data_block_start;      // block number where data starts
    uint32_t root_inode;            // inode number of the root inode
} Superblock;

#define DIRECT_PTRS 12  // number of direct pointers an inode has to blocks

#define USED 1
#define FREE 0

#define INCREMENT 1
#define DECREMENT (-1)

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
    uint32_t atime;             // access
    uint32_t mtime;             // modify
    uint32_t ctime;             // create

    uint32_t direct[DIRECT_PTRS];   // 12 direct block pointers
    uint32_t indirect;              // single indirect
    uint32_t double_indirect;       // double indirect
} Inode;

uint8_t block_bitmap[BLOCK_SIZE]; // global variable simulates bitmap "kept in cache"
uint8_t inode_bitmap[MAX_INODES]; // global variable simulates bitmap "kept in cache"

typedef struct {
    Superblock sb;     // global variable simulates superblock "kept in cache"
    FILE *disk;        // "virtual disk" file
    char mounted;
} FileSystem;

FileSystem fs;

void format_disk(const char *filename, uint32_t num_blocks);

void initialize_bitmap();

int update_inode_bitmap(uint32_t inode_num, uint8_t used);

int update_block_bitmap(uint32_t block_num, uint8_t used);

#endif //FILESYSTEMSTRUCTURE_H
