//
// Created by David Neškrabal on 28.10.2025.
//

#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H
#include <stdint.h>
#include "FileSystemStructure.h"

void read_block(uint32_t block_num, void *buf);

void write_block(uint32_t block_num, const void *buf);

int alloc_block();

int alloc_inode();

void free_block(uint32_t b);

void free_inode(uint32_t i);

uint32_t create_inode(uint16_t mode);

int write_inode(uint32_t inode_num, Inode *new_inode);

int read_inode(uint32_t inode_num, Inode *out_inode);

#endif //FILEMANAGEMENT_H
