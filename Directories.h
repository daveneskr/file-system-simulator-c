//
// Created by David Neškrabal on 19.12.2025.
//

#ifndef DIRECTORIES_H
#define DIRECTORIES_H
#include "FileSystemStructure.h"
#include <stdint.h>

#define IFMT 0xF000

#define NAME_MAX 32

typedef struct {
    uint32_t inode_num;
    char name[NAME_MAX];
} DirEntry;

long dir_lookup(uint32_t dir_num, char *entry_name);

void test_dir_lookup();

#endif //DIRECTORIES_H
