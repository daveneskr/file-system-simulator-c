//
// Created by David Neškrabal on 19.12.2025.
//

#include "../include/Directories.h"
#include "../include/FileManagement.h"

#include <string.h>

long dir_lookup(uint32_t dir_num, char *entry_name) {
    Inode dir;
    read_inode(dir_num, &dir);

    // find each block
    for (uint32_t i = 0; i < DIRECT_PTRS; i++) {
        uint32_t block_num = dir.direct[i];

        // loop through block DirEntries
        if (block_num != 0) {
            for (uint32_t j = 0; j < BLOCK_SIZE/sizeof(DirEntry); j++) {

                DirEntry entry;
                fseek(disk, block_num * BLOCK_SIZE + j * sizeof(DirEntry), SEEK_SET);
                fread(&entry, sizeof(DirEntry), 1, disk);

                if (entry.inode_num == 0 || entry.name[0] == '\0') continue;

                if (strcmp(entry.name, entry_name) == 0) {
                    return entry.inode_num;
                }
            }
        }
    }
    return -1;
}