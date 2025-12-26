//
// Created by David Neškrabal on 26.12.2025.
//

#ifndef INODE_H
#define INODE_H

static void mode_to_string(uint16_t mode, char *out);

void inode_to_string(const Inode *inode);

int initialize_root();

#endif //INODE_H
