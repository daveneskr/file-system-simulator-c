//
// Created by David Neškrabal on 19.12.2025.
//

#ifndef DIRECTORIES_H
#define DIRECTORIES_H
#include "FileSystemStructure.h"
#include <stdint.h>

#define NAME_MAX 32

typedef struct {
    uint32_t inode_num;
    uint8_t type; // IDIR or IREG
    uint8_t used; // USED or FREE
    uint16_t _pad; // rounds bytes
    char name[NAME_MAX]; // entry name, user visible
} DirEntry;

long dir_lookup(uint32_t dir_num, const char *entry_name);

int read_dir_entry(uint32_t offset, DirEntry *entry);

int write_dir_entry(uint32_t offset, DirEntry *entry);

int dir_block_full(uint32_t bnum);

uint32_t read_num_of_dir_entries(uint32_t bnum);

long alloc_dir_entry(uint32_t dir_inum);

long dir_add(uint32_t dir_inum, const char *name, uint32_t child_inum, uint8_t type);

int write_num_of_dir_entries(uint32_t bnum, uint32_t count);

int dir_block_update_count(uint32_t bnum, int operation);

#endif //DIRECTORIES_H
