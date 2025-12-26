//
// Created by David Neškrabal on 19.12.2025.
//

#include "../include/Directories.h"
#include "../include/FileManagement.h"

#include <string.h>

long dir_lookup(uint32_t dir_num, const char *entry_name) {
    // read dir's inode to cache
    Inode dir;
    read_inode(dir_num, &dir);

    // loop through each block the inode points to
    for (uint32_t i = 0; i < DIRECT_PTRS; i++) {
        uint32_t block_num = dir.direct[i];

        // loop through block DirEntries
        if (block_num != 0) {
            for (uint32_t j = 0; j < BLOCK_SIZE/sizeof(DirEntry); j++) {

                // read dir entry
                DirEntry entry;
                uint32_t offset = block_num * BLOCK_SIZE + sizeof(uint32_t) + j * sizeof(DirEntry);
                read_dir_entry(offset, &entry);

                // check if entry exists
                if (entry.used == FREE) return -1; // end of dir entry array

                // check if matches key
                if (strcmp(entry.name, entry_name) == 0) {
                    return entry.inode_num; // if yes, entry found
                }
            }
        }
    }
    return -1; // no entry found
}

// adds entry to dir
long dir_add(uint32_t dir_inum, const char *name, uint32_t child_inum, uint16_t type) {

    if (dir_lookup(dir_inum, name) != -1) return -1; // entry with this name already exists

    // initialize dir entry
    DirEntry entry = {0};
    entry.inode_num = child_inum;
    entry.type = type;
    entry.used = USED;
    strncpy(entry.name, name, sizeof(entry.name)-1);

    // allocates dir space
    long dir_entry_address = alloc_dir_entry(dir_inum);

    // validate allocation
    if (dir_entry_address == -1) return -1;

    // write dir entry
    write_dir_entry(dir_entry_address, &entry);

    // update inode
    Inode dir;
    read_inode(dir_inum, &dir);
    dir.size += sizeof(DirEntry);
    dir.links_count++;
    write_inode(dir_inum, &dir);

    // increment entry count in block
    dir_block_update_count(dir_entry_address / BLOCK_SIZE, INCREMENT);

    return dir_entry_address;
}

// returns the !! disk relative !! index of next free DirEntry slot
long alloc_dir_entry(uint32_t dir_inum) {
    Inode dir;
    read_inode(dir_inum, &dir);

    // loop through each direct block
    for (int i = 0; i < DIRECT_PTRS; i++) {
        if (dir.direct[i] == 0) continue; // skip if block not allocated
        if (dir_block_full(dir.direct[i])) continue; // skip if block full

        // if block not full return mem address

        long new_entry_address = dir.direct[i] * BLOCK_SIZE  // block number
                + sizeof(uint32_t) // holds num of entries
                + read_num_of_dir_entries(dir.direct[i]) * sizeof(DirEntry); // used block space

        return new_entry_address;
    }

    // if no free block found try to allocate new one
    int bnum = alloc_direct_inode_block(dir_inum);
    if (bnum > -1 ) {
        // return DirEntry index 0
        long new_entry_address = bnum * BLOCK_SIZE  // block number
                + sizeof(uint32_t); // holds num of entries
        Inode dir;
        read_inode(dir_inum, &dir);
        dir.size += sizeof(uint32_t); // header
        write_inode(dir_inum, &dir);

        return new_entry_address;
    }

    return -1; // allocation failed
}

int read_dir_entry(uint32_t offset, DirEntry *entry) {
    fseek(fs.disk, offset, SEEK_SET);
    fread(entry, sizeof(DirEntry), 1, fs.disk);
}

int write_dir_entry(uint32_t offset, DirEntry *entry) {
    fseek(fs.disk, offset, SEEK_SET);
    fwrite(entry, sizeof(DirEntry), 1, fs.disk);
}

// returns true if all DirEntry's at that block are used
int dir_block_full(uint32_t bnum) {
    // get num of entries
    uint32_t num_of_entries = read_num_of_dir_entries(bnum);

    // check if it equals the theoretical maximum of entries in a block
    if (num_of_entries == (BLOCK_SIZE - sizeof(uint32_t)) / sizeof(DirEntry)) return 1;
    return 0;
}

// returns 1 if directory has no DirEntry's at that block
int dir_block_empty(uint32_t bnum) {
    uint32_t num_of_entries = read_num_of_dir_entries(bnum);

    if (num_of_entries == 0) return 1;
    return 0;
}

// returns number of used DirEnry's in the directory block
uint32_t read_num_of_dir_entries(uint32_t bnum) {
    uint32_t num_of_entries;
    fseek(fs.disk, bnum * BLOCK_SIZE, SEEK_SET);
    fread(&num_of_entries, sizeof(uint32_t), 1, fs.disk);
    return num_of_entries;
}

// returns number of used DirEnry's in the directory block
int write_num_of_dir_entries(uint32_t bnum, uint32_t count) {
    fseek(fs.disk, bnum * BLOCK_SIZE, SEEK_SET);
    fwrite(&count, sizeof(uint32_t), 1, fs.disk);
    return 0;
}

// INCREMENT or DECREMENT dir entry count
int dir_block_update_count(uint32_t bnum, int operation) {
    uint32_t count = read_num_of_dir_entries(bnum);
    count += operation;

    write_num_of_dir_entries(bnum, count);
}

// make a new directory in parent, return 0 on success, -1 else
int mkdir(uint32_t parent_inum, char *child) {
    // check if parent is dir
    if (!is_dir(parent_inum)) return -1;

    int child_inum = create_dir(IDIR|IRUSR|IWUSR|IXUSR);

    // add parent as child second entry '..'
    if (dir_add(child_inum, "..", parent_inum, IDIR) == -1) return -1;

    // add child as directory entry to parent
    if (dir_add(parent_inum, child, child_inum, IDIR) == -1) return -1;

    return child_inum; // success
}

// returns 1 if inode is directory, 0 else
int is_dir(uint32_t inum) {
    Inode inode;
    read_inode(inum, &inode);

    // keep only first 4 bits and compare to IDIR
    return (inode.mode & 0xF000) == IDIR ? 1 : 0;
}

// alloc new dir inode and adds itself as first entry
int create_dir(uint16_t mode) {
    // allocate new dir full control inode
    int inum = create_inode(mode);
    if (inum == -1) return -1;

    // add itself as first entry
    dir_add(inum, ".", inum, IDIR);

    return inum;
}

int dir_list(uint32_t dir_inum) {
    Inode dir;
    read_inode(dir_inum, &dir);

    for (int i = 0; i < DIRECT_PTRS; i++) {
        // skip if block not alloc
        if (dir.direct[i] == 0) continue;
        // skip if block emtpy
        if (dir_block_empty(dir.direct[i])) continue;

        uint32_t offset = sizeof(uint32_t) + dir.direct[i] * BLOCK_SIZE;

        DirEntry entry;
        read_dir_entry(offset, &entry);

        while (entry.used == USED) {
            printf("%s\n", entry.name);
            offset += sizeof(DirEntry);
            read_dir_entry(offset, &entry);
        }
    }
}