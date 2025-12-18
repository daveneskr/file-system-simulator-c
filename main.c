#include "FileManagement.h"
#include "FileSystemStructure.h"

/* TO DO:
 * - write block and inode bitmaps on disk
 * - implement inode table
 * - function for adding inodes to the table
 *   -> calculate how much an inode takes of space
 *   -> in which block is the part of the array we need to write into
*/

int main(void) {
    format_disk("FS.bin", 10);

    fclose(disk);
}