//
// Created by David Neškrabal on 26.12.2025.
//


#include <Directories.h>
#include <FileManagement.h>
#include <FileSystemStructure.h>

static void mode_to_string(uint16_t mode, char *out) {
    out[0] = (mode & IDIR) ? 'd' : '-';
    out[1] = (mode & IRUSR) ? 'r' : '-';
    out[2] = (mode & IWUSR) ? 'w' : '-';
    out[3] = (mode & IXUSR) ? 'x' : '-';
    out[4] = '\0';
}

void inode_to_string(const Inode *inode) {
    char perm[5];
    mode_to_string(inode->mode, perm);

    printf("Inode {\n");
    printf("  mode        : 0x%04x (%s)\n", inode->mode, perm);
    printf("  links       : %u\n", inode->links_count);
    printf("  size        : %u bytes\n", inode->size);

    printf("  atime       : %s", ctime(&inode->atime));
    printf("  mtime       : %s", ctime(&inode->mtime));
    printf("  ctime       : %s", ctime(&inode->ctime));

    printf("  direct      : ");
    for (int i = 0; i < DIRECT_PTRS; i++) {
        printf("%u ", inode->direct[i]);
    }
    printf("\n");

    printf("  indirect    : %u\n", inode->indirect);
    printf("  double_ind  : %u\n", inode->double_indirect);
    printf("}\n");
}

int initialize_root() {
    int root_num = create_dir(IDIR|IWUSR|IRUSR|IXUSR);
    // add itself as parent
    dir_add(root_num, "..", root_num, IDIR);

    return root_num;
}
