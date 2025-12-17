#include "FileManagement.h"
#include "FileSystemStructure.h"

/* TO DO:
 * - write block and inode bitmaps on disk
 * - implement inode table
*/

int main(void) {
    format_disk("FS.bin", 10);

    fclose(disk);
}