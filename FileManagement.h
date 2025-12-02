//
// Created by David Neškrabal on 28.10.2025.
//

#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H
#include <stdint.h>

void read_block(uint32_t block_num, void *buf);

void write_block(uint32_t block_num, const void *buf);

int alloc_block();

int alloc_inode();

void free_block(int b);

void free_inode(int i);

#endif //FILEMANAGEMENT_H
