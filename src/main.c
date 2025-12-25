#include <FileManagement.h>
#include <stdio.h>

#include "../include/FileSystemStructure.h"
#include "../include/Directories.h"

/* TO DO:
   - add argument validation
   - optimise dir lookup
     - condence dir contents?
*/

int main(void) {
    format_disk("FS.bin", 10);

    uint32_t new_inode = create_inode(IDIR);

    printf("%i\n", dir_lookup(fs.sb.root_inode, "NEW"));

    dir_add(fs.sb.root_inode, "NEW", new_inode, IDIR);

    printf("%i\n", dir_lookup(fs.sb.root_inode, "NEW"));

    fclose(fs.disk);
}
