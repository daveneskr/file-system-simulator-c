//
// Created by David Neškrabal on 26.12.2025.
//

#include "../include/Files.h"

#include <Directories.h>
#include <FileManagement.h>
#include <stdint.h>


int creat(uint32_t parent, char *name, uint16_t mode) {
    // check if entry with this name already exists
    if (dir_lookup(parent, name) != -1) return -1;
    // check if type regular file
    if (mode & 0xF000 != IREG) return -1;

    int file = create_inode(mode);
    if (file == -1) return -1;


    dir_add(parent, name, file, mode);

    return file;
}
